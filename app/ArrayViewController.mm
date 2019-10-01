#import "ArrayViewController.h"

#import "Formatter.h"

@implementation ArrayViewController {
  ArrayCountFormatter *_formater;
  NSUInteger _minimalCount;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  _formater = [ArrayCountFormatter new];
}

VIEWDIDAPPEAR_BEGIN

    [self.countTextField becomeFirstResponder];

VIEWDIDAPPEAR_END

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
    self.valid = YES;
  } else {
    self.countTextField.textColor = Color.redColor;
    self.valid = NO;
  }
}

@end
