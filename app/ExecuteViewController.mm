#import "ExecuteViewController.h"

#include <WebKit/WebKit.h>

#include <chrono>

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
  self.outputTextView.layer.borderColor = Color.blackColor.CGColor;
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
      } else if ([type isEqualToString:@"error"]) {
        NSLog(@"Name: %@", payload[@"name"]);
        NSLog(@"Message: %@", payload[@"message"]);
        NSLog(@"Stacktrace: %@", payload[@"stacktrace"]);
      }
    }
  }
}

#pragma mark - Private methods

- (void)tryStartExecute {
  if (_appeared && _loaded) {
    [self.wkWebView evaluateJavaScript:_executableCode
                     completionHandler:^(id _Nullable result, NSError* _Nullable error) {
                       if (error != nil) {
                         NSLog(@"%@", error);
                       }
                     }];
  }
}

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
