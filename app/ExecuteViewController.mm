#import "ExecuteViewController.h"

#include <WebKit/WebKit.h>

#include <chrono>

#include "logo_sketcher.hpp"

#import "DrawContext.h"
#import "NSString+StringView.h"
#import "Theme.h"

constexpr double refreshTimeInMS = 40;

@interface ExecuteViewController () <WKScriptMessageHandler>

@property(weak) IBOutlet WKWebView* wkWebView;

@property(weak) IBOutlet TextView* outputTextView;

@end

@implementation ExecuteViewController {
  NSString* _executableCode;
  bool _appeared;
  bool _loaded;

  NSString* _outputText;
  bool _needRefreshImage;
  std::chrono::high_resolution_clock::time_point _image_refresh_time;
  std::chrono::high_resolution_clock::time_point _output_refresh_time;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  _appeared = false;
  _loaded = false;

  _outputText = @"";

  [self.wkWebView.configuration.userContentController addScriptMessageHandler:self name:@"system"];

  auto url = [[NSBundle mainBundle] URLForResource:@"exec_env" withExtension:@"html"];
  auto request = [NSURLRequest requestWithURL:url];
  [self.wkWebView loadRequest:request];
  [self.wkWebView addObserver:self forKeyPath:@"loading" options:0 context:nil];

  self.outputTextView.layer.borderWidth = 1;
  self.outputTextView.layer.borderColor = [Color blackColor].CGColor;
  _needRefreshImage = NO;
}

#ifdef IOS
- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
#else
- (void)viewDidAppear {
  [super viewDidAppear];
#endif

  _appeared = true;
  [self tryStartExecute];
}

- (void)setExecutable:(NSString*)executable {
  _executableCode = executable;
  NSLog(@"%@", _executableCode);
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id>*)change
                       context:(void*)context {
  if (object == self.wkWebView && [keyPath isEqualToString:@"loading"] &&
      !self.wkWebView.isLoading) {
    _loaded = true;
    [self.wkWebView removeObserver:self forKeyPath:@"loading"];

    [self tryStartExecute];
  }
}

#pragma mark - WKScriptMessageHandler implementation

- (void)userContentController:(WKUserContentController*)userContentController
      didReceiveScriptMessage:(WKScriptMessage*)message {
  if ([message.body isKindOfClass:[NSDictionary class]]) {
    NSDictionary* payload = message.body;
    id typeData = payload[@"type"];
    if (typeData != nil && [typeData isKindOfClass:[NSString class]]) {
      NSString* type = typeData;
      if ([type isEqualToString:@"log"]) {
        _outputText = [_outputText stringByAppendingString:payload[@"message"]];
        auto time = std::chrono::high_resolution_clock::now();
        auto diff =
            std::chrono::duration_cast<std::chrono::milliseconds>(time - _output_refresh_time);
        if (diff.count() >= refreshTimeInMS) {
          [self refreshOutput];
        }
      }
    }
  }
}

#pragma mark - DrawContextDelegate implementation

- (void)setNeedRefreshImage {
  auto time = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _image_refresh_time);
  if (diff.count() < refreshTimeInMS) {
    _needRefreshImage = YES;
  }
}

#pragma mark - Private methods

- (void)tryStartExecute {
  if (_appeared && _loaded) {
    [self.wkWebView evaluateJavaScript:_executableCode
                     completionHandler:^(id _Nullable result, NSError* _Nullable error) {
                       if (error != nil) {
                         NSLog(@"%@", error);
                       } else {
                         NSLog(@"%@", result);
                       }
                     }];
  }
}

// - (void)startExecute {
//   assert(_environment.has_value());

//   NativeEnvironment<ExecuteViewController> system{*_environment, "system", self};

//   system.register_native_instruction<std::string>("print", [](auto self, std::string message) {
//     _outputText += message;
//     auto time = std::chrono::high_resolution_clock::now();
//     auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time -
//     _output_refresh_time); if (diff.count() >= refreshTimeInMS) {
//       [self refreshOutput];
//     }
//   });

//   system.register_native_instruction<double, double, double, double>(
//       "draw_line", [](auto self, double start_x, double start_y, double end_x, double end_y) {
//         self->_drawContext.draw_line(start_x, start_y, end_x, end_y);
//       });

//   auto logo = system.makeSubEnvironment([](auto self) { return &self->_logoSketcher; });
//   decltype(self->_logoSketcher)::register_instructions(logo);

//   _environment->execute();
// }

// - (void)stopExecute {
//   assert(_environment.has_value());

//   _environment->terminate();
// }

- (void)refreshOutput {
  if (_outputText.length > 0) {
    [self.outputTextView.textStorage
        replaceCharactersInRange:NSMakeRange(self.outputTextView.string.length, 0)
            withAttributedString:[[NSAttributedString alloc]
                                     initWithString:_outputText
                                         attributes:currentTheme().consoleAttrs]];

    _output_refresh_time = std::chrono::high_resolution_clock::now();
    _outputText = @"";

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
