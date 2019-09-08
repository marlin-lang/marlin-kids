#import "ExecuteViewController.h"

#include <chrono>

#include "logo_sketcher.hpp"

#import "DrawContext.h"
#import "NSString+StringView.h"
#import "NativeFuncUtils.h"
#import "Theme.h"

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
  std::chrono::high_resolution_clock::time_point _image_refresh_time;
  std::string _outputText;
  std::chrono::high_resolution_clock::time_point _output_refresh_time;
}

- (void)viewDidLoad {
  [super viewDidLoad];

#ifndef IOS
  self.imageView.image = [Image imageWithSize:self.imageView.bounds.size
                                      flipped:NO
                               drawingHandler:^BOOL(CGRect dstRect) {
                                 return YES;
                               }];
#endif
  self.outputTextView.layer.borderWidth = 1;
  self.outputTextView.layer.borderColor = [Color blackColor].CGColor;
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
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _image_refresh_time);
  if (diff.count() < refreshTimeInMS) {
    _needRefreshImage = YES;
  } else {
    [self refreshImage];
  }
}

#pragma mark - Private methods

- (void)startExecute {
  assert(_environment.has_value());

  NativeEnvironment<ExecuteViewController> system{*_environment, "system", self};

  system.register_native_instruction<std::string>("print", [](auto self, std::string message) {
    _outputText += message;
    auto time = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _output_refresh_time);
    if (diff.count() >= refreshTimeInMS) {
      [self refreshOutput];
    }
  });

  system.register_native_instruction<double, double, double, double>(
      "draw_line", [](auto self, double start_x, double start_y, double end_x, double end_y) {
        self->_drawContext.draw_line(start_x, start_y, end_x, end_y);
      });

  auto logo = system.makeSubEnvironment([](auto self) { return &self->_logoSketcher; });
  decltype(self->_logoSketcher)::register_instructions(logo);

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
  _image_refresh_time = std::chrono::high_resolution_clock::now();
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

- (void)refreshOutput {
  if (!_outputText.empty()) {
    [self.outputTextView.textStorage
        replaceCharactersInRange:NSMakeRange(self.outputTextView.string.length, 0)
            withAttributedString:[[NSAttributedString alloc]
                                     initWithString:[NSString stringWithStringView:_outputText]
                                         attributes:currentTheme().consoleAttrs]];

    _output_refresh_time = std::chrono::high_resolution_clock::now();
    _outputText.clear();

    __weak auto weakSelf = self;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, (int64_t)(refreshTimeInMS / 1000 * NSEC_PER_SEC)),
        dispatch_get_main_queue(), ^{
          if (auto strongSelf = weakSelf) {
            [strongSelf refreshOutput];
          }
        });
  }
}
@end
