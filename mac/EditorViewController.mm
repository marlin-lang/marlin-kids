#import "EditorViewController.h"

@interface EditorViewController ()

@property(weak) IBOutlet NSTextField* editorTextField;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  NSNumberFormatter* formatter = [NSNumberFormatter new];
  formatter.minimumFractionDigits = 0;
  formatter.maximumFractionDigits = 10;
  self.editorTextField.cell.formatter = formatter;
  self.editorTextField.placeholderString = @"Please input number";
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

@end
