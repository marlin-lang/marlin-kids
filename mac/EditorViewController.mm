#import "EditorViewController.h"

@interface EditorViewController ()

@property(weak) IBOutlet NSTextField *editorTextField;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

@end
