#import "ArrayViewController.h"

#import "Formatter.h"

@interface ArrayViewController () <NSTextFieldDelegate>

@property(weak) IBOutlet Button *okButton;

@end

@implementation ArrayViewController {
  ArrayCountFormatter *_formater;
  NSUInteger _minimalCount;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.countTextField.delegate = self;
  _formater = [ArrayCountFormatter new];
  self.okButton.enabled = NO;
}

- (void)setMinimalCount:(NSUInteger)minimalCount {
  _minimalCount = minimalCount;
  _formater.minimalCount = minimalCount;
  [self validate];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate arrayViewController:self finishEditingWithCount:self.countTextField.integerValue];
}

- (void)validate {
  if ([_formater getObjectValue:nil
                      forString:self.countTextField.stringValue
               errorDescription:nil]) {
    self.okButton.enabled = YES;
  } else {
    self.okButton.enabled = NO;
  }
}

#pragma mark - NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)obj {
  [self validate];
}

@end
