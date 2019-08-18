#import "ExecuteViewController.h"

#import "NSString+StringView.h"

@interface ExecuteViewController ()

@property(weak) IBOutlet NSTextField *outputTextField;

@end

@implementation ExecuteViewController

- (void)viewDidLoad {
  [super viewDidLoad];
}

- (void)viewDidAppear {
  [super viewDidAppear];

  assert(self.environment.has_value());

  self.environment->execute([self](std::string value) {
    dispatch_sync(dispatch_get_main_queue(), ^{
      self.outputTextField.stringValue = [self.outputTextField.stringValue
          stringByAppendingString:[NSString stringWithStringView:value]];
    });
  });
}

- (void)viewWillDisappear {
  if (self.environment.has_value()) {
    self.environment->terminate();
  }
}

@end
