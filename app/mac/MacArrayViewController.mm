#import "MacArrayViewController.h"

@interface MacArrayViewController () <NSTextFieldDelegate>

@end

@implementation MacArrayViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.countTextField.delegate = self;
}

#pragma mark - NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)obj {
  [self validate];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  if (self.isValid) {
    [self.delegate arrayViewController:self
                finishEditingWithCount:self.countTextField.integerValue];
  }
}

@end
