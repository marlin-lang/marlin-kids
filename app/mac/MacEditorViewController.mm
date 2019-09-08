#import "MacEditorViewController.h"

@interface MacEditorViewController () <NSTextFieldDelegate>

@end

@implementation MacEditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.editorTextField.delegate = self;
}

- (void)controlTextDidChange:(NSNotification*)obj {
  return [self validate];
}

- (void)setupSegmentControlWithTitles:(NSArray<NSString*>*)titles selection:(NSUInteger)selection {
  self.typeSegmentControl.segmentCount = titles.count;
  auto index = 0;
  for (NSString* title in titles) {
    [self.typeSegmentControl setLabel:title forSegment:index];
    ++index;
  }
  [self.typeSegmentControl setSelected:YES forSegment:selection];
}

- (NSUInteger)selectionOfSegment {
  return self.typeSegmentControl.selectedSegment;
}

- (NSString*)textOfEditor {
  return self.editorTextField.stringValue;
}

- (void)setTextOfEditor:(NSString*)string {
  self.editorTextField.stringValue = string;
}

@end
