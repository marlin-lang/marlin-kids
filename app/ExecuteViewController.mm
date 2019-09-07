#import "ExecuteViewController.h"

#include <chrono>

#include "logo_sketcher.hpp"
#include "native_func_utils.hpp"

#import "DrawContext.h"
#import "NSString+StringView.h"
#import "Theme.h"

struct SystemEnvironmentContainer {
  __weak ExecuteViewController* environment;

  SystemEnvironmentContainer(ExecuteViewController* _env) : environment{_env} {}
};

void dispatch_on_main(void (^block)(void)) {
  if (NSThread.isMainThread) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

template <typename... Args, typename SystemContainer, typename Callable>
void register_native_instruction(SystemContainer& system, const char* name, Callable&& callable) {
  system.set_native_function(name, SystemContainer::template callback<void(Args...)>::wrapped(
                                       [callable{std::forward<Callable>(callable)}](
                                           SystemEnvironmentContainer* container, Args... args) {
                                         dispatch_on_main(^{
                                           if (auto self = container->environment) {
                                             callable(self, args...);
                                           }
                                         });
                                       }));
}

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

  marlin::control::native_container<SystemEnvironmentContainer> system{*_environment, "system",
                                                                       self};

  register_native_instruction<std::string>(system, "print", [](auto self, std::string message) {
#ifndef IOS
    [self.outputTextView.textStorage
        replaceCharactersInRange:NSMakeRange(self.outputTextView.string.length, 0)
            withAttributedString:[[NSAttributedString alloc]
                                     initWithString:[NSString stringWithStringView:message]
                                         attributes:currentTheme().consoleAttrs]];
#endif
  });

  register_native_instruction<double, double, double, double>(
      system, "draw_line",
      [](auto self, double start_x, double start_y, double end_x, double end_y) {
        self->_drawContext.draw_line(start_x, start_y, end_x, end_y);
      });

  register_native_instruction<double>(system, "logo_forward", [](auto self, double length) {
    self->_logoSketcher.forward(length);
  });
  register_native_instruction<double>(system, "logo_backward", [](auto self, double length) {
    self->_logoSketcher.backward(length);
  });
  register_native_instruction<double>(system, "logo_turn_left", [](auto self, double degree) {
    self->_logoSketcher.turn_left(degree);
  });
  register_native_instruction<double>(system, "logo_turn_right", [](auto self, double degree) {
    self->_logoSketcher.turn_right(degree);
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
