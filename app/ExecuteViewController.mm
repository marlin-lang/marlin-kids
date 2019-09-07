#import "ExecuteViewController.h"

#include <chrono>

#include "logo_sketcher.hpp"
#include "native_func_utils.hpp"

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

struct SystemEnvironmentContainer {
  __weak ExecuteViewController* environment;

  SystemEnvironmentContainer(ExecuteViewController* _env) : environment{_env} {}
};

constexpr double refreshTimeInMS = 40;

@interface ExecuteViewController ()

@property(weak) IBOutlet ImageView* imageView;
@property(weak) IBOutlet TextView* outputTextView;

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

- (void)viewDidAppear {
  [super viewDidAppear];

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

  marlin::control::native_container<SystemEnvironmentContainer> system{*_environment, "system",
                                                                       self};

  system.set_native_function("print", [](auto& ctx, auto this_obj, auto args, auto exception) {
    if (args.size() != 1) {
      *exception = ctx.error("Incorrect number of arguments!");
    } else {
      std::string output = args[0].to_string();
      if (!ctx.ok()) {
        *exception = ctx.get_exception();
        return;
      }

      dispatch_on_main(^{
        if (auto self = this_obj->environment) {
#ifndef IOS
          [self.outputTextView.textStorage
              replaceCharactersInRange:NSMakeRange(self.outputTextView.string.length, 0)
                  withAttributedString:[[NSAttributedString alloc]
                                           initWithString:[NSString stringWithStringView:output]
                                               attributes:currentTheme().consoleAttrs]];
#endif
        }
      });
    }
  });

  system.set_native_function("draw_line", [](auto& ctx, auto this_obj, auto args, auto exception) {
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
        if (auto self = this_obj->environment) {
          self->_drawContext.draw_line(start_x, start_y, end_x, end_y);
        }
      });
    }
  });

  system.set_native_function("logo_forward",
                             [](auto& ctx, auto this_obj, auto args, auto exception) {
                               if (args.size() != 1) {
                                 *exception = ctx.error("Incorrect number of arguments!");
                               } else {
                                 double length = args[0].to_number();
                                 if (!ctx.ok()) {
                                   *exception = ctx.get_exception();
                                   return;
                                 }

                                 dispatch_on_main(^{
                                   if (auto self = this_obj->environment) {
                                     self->_logoSketcher.forward(length);
                                   }
                                 });
                               }
                             });
  system.set_native_function("logo_backward",
                             [](auto& ctx, auto this_obj, auto args, auto exception) {
                               if (args.size() != 1) {
                                 *exception = ctx.error("Incorrect number of arguments!");
                               } else {
                                 double length = args[0].to_number();
                                 if (!ctx.ok()) {
                                   *exception = ctx.get_exception();
                                   return;
                                 }

                                 dispatch_on_main(^{
                                   if (auto self = this_obj->environment) {
                                     self->_logoSketcher.backward(length);
                                   }
                                 });
                               }
                             });
  system.set_native_function("logo_turn_left",
                             [](auto& ctx, auto this_obj, auto args, auto exception) {
                               if (args.size() != 1) {
                                 *exception = ctx.error("Incorrect number of arguments!");
                               } else {
                                 double degree = args[0].to_number();
                                 if (!ctx.ok()) {
                                   *exception = ctx.get_exception();
                                   return;
                                 }

                                 dispatch_on_main(^{
                                   if (auto self = this_obj->environment) {
                                     self->_logoSketcher.turn_left(degree);
                                   }
                                 });
                               }
                             });
  system.set_native_function("logo_turn_right",
                             [](auto ctx, auto this_obj, auto args, auto exception) {
                               if (args.size() != 1) {
                                 *exception = ctx.error("Incorrect number of arguments!");
                               } else {
                                 double degree = args[0].to_number();
                                 if (!ctx.ok()) {
                                   *exception = ctx.get_exception();
                                   return;
                                 }

                                 dispatch_on_main(^{
                                   if (auto self = this_obj->environment) {
                                     self->_logoSketcher.turn_right(degree);
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
  for (NSImageRep* rep in self.imageView.image.representations) {
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
