#import "DuplicateViewController.h"

#import "DuplicateSourceTextField.h"

@interface DuplicateViewController ()

@property(weak) IBOutlet DuplicateSourceTextField *sourceTextField;

@end

@implementation DuplicateViewController

- (void)setSourceString:(NSString *)string {
  self.sourceTextField.sourceString = string;
}

- (void)setDraggingData:(const DraggingData &)draggingData {
  self.sourceTextField.draggingData = draggingData;
}

- (IBAction)deleteButtonPressed:(id)sender {
  [self.delegate performDeleteForDuplicateViewController:self];
}

@end
