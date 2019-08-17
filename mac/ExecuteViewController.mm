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

  auto &doc = self.document.content;
  doc.execute([self](const marlin::ast::base &node, const std::string &message) {
    [self.delegate addErrorAt:node message:message];
  });
  self.outputTextField.stringValue = [NSString stringWithStringView:doc.output()];
}

@end
