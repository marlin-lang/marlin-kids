#import "DuplicateViewController.h"

#import "DuplicateSourceLabel.h"

@interface DuplicateViewController ()

@property(weak) IBOutlet DuplicateSourceLabel *sourceLabel;

@end

@implementation DuplicateViewController

- (void)setSourceString:(NSString *)string {
  self.sourceLabel.sourceString = string;
}

- (void)setDraggingData:(const DraggingData &)draggingData {
  self.sourceLabel.draggingData = draggingData;
}

- (IBAction)deleteButtonPressed:(id)sender {
  [self.delegate performDeleteForDuplicateViewController:self];
}

@end
