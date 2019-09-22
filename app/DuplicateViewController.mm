#import "DuplicateViewController.h"

#import "DuplicateSourceView.h"

@interface DuplicateViewController ()

@property(weak) IBOutlet DuplicateSourceView *sourceView;

@end

@implementation DuplicateViewController

- (void)setDraggingData:(const DraggingData &)draggingData {
  self.sourceView.draggingData = draggingData;
}

- (IBAction)deleteButtonPressed:(id)sender {
  [self.delegate performDeleteForDuplicateViewController:self];
}

@end
