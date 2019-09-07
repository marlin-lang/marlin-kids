#import "ExecuteViewController.h"

#import <chrono>

#import "logo_sketcher.hpp"

#import "DrawContext.h"
#import "NSString+StringView.h"
#import "Theme.h"

void dispatch_on_main(void (^block)(void)) {
  if (NSThread.isMainThread) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

constexpr double refreshTimeInMS = 40;

@interface ExecuteViewController ()

@property(weak) IBOutlet ImageView *imageView;
@property(weak) IBOutlet TextView *outputTextView;

@end

@implementation ExecuteViewController {
  std::optional<marlin::control::exec_environment> _environment;
  DrawContext _drawContext;
  marlin::control::logo_sketcher<DrawContext> _logoSketcher;

  bool _needRefreshImage;
  std::chrono::high_resolution_clock::time_point _refresh_time;
}

- (void)viewDidLoad {
  [super viewDidLoad];

#ifndef IOS
  self.imageView.image = [Image imageWithSize:self.imageView.bounds.size
                                      flipped:NO
                               drawingHandler:^BOOL(Rect dstRect) {
                                 return YES;
                               }];
#endif
  _drawContext.initWithImage(self.imageView.image, self);
  _logoSketcher.set_context(_drawContext);
  _needRefreshImage = NO;
}

#ifdef IOS
- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
#else
- (void)viewDidAppear {
  [super viewDidAppear];
#endif

  [self startExecute];
}

- (void)viewWillDisappear {
  [self stopExecute];
}

- (void)setEnvironment:(marlin::control::exec_environment)environment {
  _environment = std::move(environment);
}

#pragma mark - DrawContextDelegate implementation

- (void)setNeedRefreshImage {
  auto time = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _refresh_time);
  if (diff.count() < refreshTimeInMS) {
    _needRefreshImage = YES;
  } else {
    [self refreshImage];
  }
}

#pragma mark - Private methods

- (void)startExecute {
  assert(_environment.has_value());

  __weak auto weakSelf = self;
  _environment->register_print_callback([weakSelf](std::string value) {
    dispatch_on_main(^{
#ifndef IOS
      [weakSelf.outputTextView.textStorage
          replaceCharactersInRange:NSMakeRange(weakSelf.outputTextView.string.length, 0)
              withAttributedString:[[NSAttributedString alloc]
                                       initWithString:[NSString stringWithStringView:value]
                                           attributes:currentTheme().consoleAttrs]];
#endif
    });
  });

  _environment->add_custom_callback(
      "draw_line", [weakSelf](auto ctx, auto, auto args, auto exception) {
        if (args.size() != 4) {
          *exception = ctx.error("Incorrect number of arguments!");
        } else {
          double start_x = args[0].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double start_y = args[1].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double end_x = args[2].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double end_y = args[3].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          dispatch_on_main(^{
            if (auto strongSelf = weakSelf) {
              strongSelf->_drawContext.draw_line(start_x, start_y, end_x, end_y);
            }
          });
        }
      });

  _environment->add_custom_callback("logo_forward",
                                    [weakSelf](auto ctx, auto, auto args, auto exception) {
                                      if (args.size() != 1) {
                                        *exception = ctx.error("Incorrect number of arguments!");
                                      } else {
                                        double length = args[0].to_number();
                                        if (!ctx.ok()) {
                                          *exception = ctx.get_exception();
                                          return;
                                        }

                                        dispatch_on_main(^{
                                          if (auto strongSelf = weakSelf) {
                                            strongSelf->_logoSketcher.forward(length);
                                          }
                                        });
                                      }
                                    });
  _environment->add_custom_callback("logo_backward",
                                    [weakSelf](auto ctx, auto, auto args, auto exception) {
                                      if (args.size() != 1) {
                                        *exception = ctx.error("Incorrect number of arguments!");
                                      } else {
                                        double length = args[0].to_number();
                                        if (!ctx.ok()) {
                                          *exception = ctx.get_exception();
                                          return;
                                        }

                                        dispatch_on_main(^{
                                          if (auto strongSelf = weakSelf) {
                                            strongSelf->_logoSketcher.backward(length);
                                          }
                                        });
                                      }
                                    });
  _environment->add_custom_callback("logo_turn_left",
                                    [weakSelf](auto ctx, auto, auto args, auto exception) {
                                      if (args.size() != 1) {
                                        *exception = ctx.error("Incorrect number of arguments!");
                                      } else {
                                        double degree = args[0].to_number();
                                        if (!ctx.ok()) {
                                          *exception = ctx.get_exception();
                                          return;
                                        }

                                        dispatch_on_main(^{
                                          if (auto strongSelf = weakSelf) {
                                            strongSelf->_logoSketcher.turn_left(degree);
                                          }
                                        });
                                      }
                                    });
  _environment->add_custom_callback("logo_turn_right",
                                    [weakSelf](auto ctx, auto, auto args, auto exception) {
                                      if (args.size() != 1) {
                                        *exception = ctx.error("Incorrect number of arguments!");
                                      } else {
                                        double degree = args[0].to_number();
                                        if (!ctx.ok()) {
                                          *exception = ctx.get_exception();
                                          return;
                                        }

                                        dispatch_on_main(^{
                                          if (auto strongSelf = weakSelf) {
                                            strongSelf->_logoSketcher.turn_right(degree);
                                          }
                                        });
                                      }
                                    });

  _environment->execute();
}

- (void)stopExecute {
  assert(_environment.has_value());

  _environment->terminate();
}

- (void)refreshImage {
#ifndef IOS
  for (NSImageRep *rep in self.imageView.image.representations) {
    [self.imageView.image removeRepresentation:rep];
  }
  [self.imageView.image addRepresentation:_drawContext.imageRep()];
  [self.imageView setNeedsDisplay:YES];
#endif
  _refresh_time = std::chrono::high_resolution_clock::now();
  _needRefreshImage = NO;

  __weak auto weakSelf = self;
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(refreshTimeInMS / 1000 * NSEC_PER_SEC)),
                 dispatch_get_main_queue(), ^{
                   if (auto strongSelf = weakSelf) {
                     if (strongSelf->_needRefreshImage) {
                       [strongSelf refreshImage];
                     }
                   }
                 });
}

@end
