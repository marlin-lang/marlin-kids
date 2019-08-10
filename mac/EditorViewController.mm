#import "EditorViewController.h"

@interface EditorViewController ()

@property(weak) IBOutlet NSSegmentedControl* typeSegmentControl;
@property(weak) IBOutlet NSTextField* editorTextField;
@property(weak) IBOutlet NSButton* okButton;

@property(nonatomic, strong) NSFormatter* formatter;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.typeSegmentControl.segmentCount = 2;
  [self.typeSegmentControl setLabel:@"Number" forSegment:0];
  [self.typeSegmentControl setLabel:@"String" forSegment:1];

  NSNumberFormatter* formatter = [NSNumberFormatter new];
  formatter.minimumFractionDigits = 0;
  formatter.maximumFractionDigits = 10;
  self.formatter = formatter;
  self.editorTextField.delegate = self;

  self.okButton.enabled = NO;
}

- (void)setForNumber {
  [self.typeSegmentControl setSelected:YES forSegment:0];
}

- (void)setForString {
  [self.typeSegmentControl setSelected:YES forSegment:1];
}

- (void)setForVariable {
  self.typeSegmentControl.segmentCount = 1;
  [self.typeSegmentControl setLabel:@"Variable" forSegment:0];
  [self.typeSegmentControl setSelected:YES forSegment:0];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

- (void)controlTextDidChange:(NSNotification*)obj {
  if ([self.formatter getObjectValue:nil
                           forString:self.editorTextField.stringValue
                    errorDescription:nil]) {
    self.okButton.enabled = YES;
  } else {
    self.okButton.enabled = NO;
  }
}

@end
