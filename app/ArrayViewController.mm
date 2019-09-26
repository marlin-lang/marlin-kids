#import "ArrayViewController.h"

#import "Formatter.h"

@interface ArrayViewController () <NSTextFieldDelegate>

@property(weak) IBOutlet NSTextField *countTextField;

@end

@implementation ArrayViewController {
  ArrayCountFormatter *_formater;
  NSUInteger _minimalCount;
  BOOL _isValid;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.countTextField.delegate = self;
  _formater = [ArrayCountFormatter new];
}

- (void)viewDidAppear {
  [super viewDidAppear];
  [self.countTextField becomeFirstResponder];
}

- (void)setCount:(NSUInteger)count {
  self.countTextField.stringValue = [NSString stringWithFormat:@"%lu", count];
}

- (void)setMinimalCount:(NSUInteger)minimalCount {
  _minimalCount = minimalCount;
  _formater.minimalCount = minimalCount;
  [self validate];
}

- (void)validate {
  if ([_formater getObjectValue:nil
                      forString:self.countTextField.stringValue
               errorDescription:nil]) {
    self.countTextField.textColor = Color.blackColor;
    _isValid = YES;
  } else {
    self.countTextField.textColor = Color.redColor;
    _isValid = NO;
  }
}

#pragma mark - NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)obj {
  [self validate];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  if (_isValid) {
    [self.delegate arrayViewController:self
                finishEditingWithCount:self.countTextField.integerValue];
  }
}

@end
