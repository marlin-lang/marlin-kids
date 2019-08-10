#import "EditorViewController.h"

@interface EditorViewController ()

@property(weak) IBOutlet NSTextField* editorTextField;
@property(weak) IBOutlet NSSegmentedControl* typeSegmentControl;

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
  self.editorTextField.cell.formatter = formatter;
  self.editorTextField.placeholderString = @"Please input number";
}

- (void)setForNumber {
  [self.typeSegmentControl setSelected:YES forSegment:0];
}

- (void)setForString {
  [self.typeSegmentControl setSelected:YES forSegment:1];
}

- (void)setForVariable {
  self.typeSegmentControl.segmentCount = 2;
  [self.typeSegmentControl setLabel:@"Variable" forSegment:0];
  [self.typeSegmentControl setSelected:YES forSegment:0];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

@end
