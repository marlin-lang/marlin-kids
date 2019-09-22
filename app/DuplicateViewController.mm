#import "DuplicateViewController.h"

@implementation DuplicateViewController

- (IBAction)deleteButtonPressed:(id)sender {
  [self.delegate performDeleteForDuplicateViewController:self];
}

@end
