#import <WebKit/WebKit.h>

#import "ExecuteViewController.h"

#include <chrono>

#include "logo_sketcher.hpp"

#import "DrawContext.h"
#import "NSString+StringView.h"
#import "NativeFuncUtils.h"
#import "Theme.h"

constexpr double refreshTimeInMS = 40;

@interface ExecuteViewController ()

@property(weak) IBOutlet WKWebView* wkWebView;

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

  auto url = [[NSBundle mainBundle] URLForResource:@"exec_env" withExtension:@"html"];
  auto request = [NSURLRequest requestWithURL:url];
  [self.wkWebView loadRequest:request];

  self.outputTextView.layer.borderWidth = 1;
  self.outputTextView.layer.borderColor = [Color blackColor].CGColor;
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
