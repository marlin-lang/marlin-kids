#import "MacEditorViewController.h"

@interface MacEditorViewController () <NSTextFieldDelegate>

@end

@implementation MacEditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.editorTextField.delegate = self;
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

#pragma mark - NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification*)obj {
  [self validate];
}

- (void)controlTextDidEndEditing:(NSNotification*)obj {
  if (self.isValid) {
    [self.delegate editorViewController:self
                   finishEditWithString:self.editorTextField.stringValue
                                 ofType:self.type];
  }
}

@end
