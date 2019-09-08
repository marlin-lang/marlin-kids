#import "IosEditorViewController.h"

@implementation IosEditorViewController

- (void)setupSegmentControlWithTitles:(NSArray<NSString*>*)titles selection:(NSUInteger)selection {
  [self.typeSegmentControl removeAllSegments];
  for (NSString* title in titles) {
    [self.typeSegmentControl insertSegmentWithTitle:title
                                            atIndex:self.typeSegmentControl.numberOfSegments
                                           animated:NO];
  }
  self.typeSegmentControl.selectedSegmentIndex = selection;
}

- (IBAction)editorTextFieldDidChange:(id)sender {
  [self validate];
}

@end
