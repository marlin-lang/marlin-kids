#import "EditorViewController.h"

@interface TestFormater : NSFormatter

@end

@implementation TestFormater

- (NSString *)stringForObjectValue:(id)obj {
  return @"";
}

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  return YES;
}

- (BOOL)isPartialStringValid:(NSString *)partialString
            newEditingString:(NSString *__autoreleasing _Nullable *)newString
            errorDescription:(NSString *__autoreleasing _Nullable *)error {
  for (auto i = 0; i < partialString.length; ++i) {
    auto ch = [partialString characterAtIndex:i];
    if (ch < '0' || ch > '9') {
      NSBeep();
      return NO;
    }
  }
  return YES;
}

@end

@interface EditorViewController ()

@property(weak) IBOutlet NSTextField *editorTextField;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.editorTextField.cell.formatter = [TestFormater new];
  self.editorTextField.placeholderString = @"Please input number";
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

@end
