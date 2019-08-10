#import "EditorViewController.h"

@interface StringFormatter : NSFormatter

@end

@implementation StringFormatter

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  return YES;
}

@end

@interface VariableFormatter : NSFormatter

@end

@implementation VariableFormatter

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  for (auto i = 0; i < string.length; ++i) {
    auto ch = [string characterAtIndex:i];
    if (i == 0 && isdigit(ch)) {
      return NO;
    }
    if (ch != '_' && !isalnum(ch)) {
      return NO;
    }
  }
  return YES;
}

@end

@interface EditorViewController ()

@property(weak) IBOutlet NSSegmentedControl *typeSegmentControl;
@property(weak) IBOutlet NSTextField *editorTextField;
@property(weak) IBOutlet NSButton *okButton;

@property(nonatomic, strong) NSFormatter *formatter;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.editorTextField.delegate = self;
  self.okButton.enabled = NO;
}

- (void)setType:(EditorType)type {
  _type = type;
  switch (_type) {
    case EditorType::Number: {
      NSNumberFormatter *formatter = [NSNumberFormatter new];
      formatter.minimumFractionDigits = 0;
      formatter.maximumFractionDigits = 10;
      self.formatter = formatter;
      [self setupSegmentControlForLiteral];
      [self.typeSegmentControl setSelected:YES forSegment:0];
    } break;
    case EditorType::String:
      self.formatter = [StringFormatter new];
      [self setupSegmentControlForLiteral];
      [self.typeSegmentControl setSelected:YES forSegment:1];
      break;
    case EditorType::Variable:
      self.formatter = [VariableFormatter new];
      self.typeSegmentControl.segmentCount = 1;
      [self.typeSegmentControl setLabel:@"Variable" forSegment:0];
      [self.typeSegmentControl setSelected:YES forSegment:0];
    default:
      break;
  }
}

- (IBAction)typeSegmentControlChanged:(id)sender {
  switch (self.typeSegmentControl.selectedSegment) {
    case 0:
      self.type = EditorType::Number;
      break;
    case 1:
      self.type = EditorType::String;
    default:
      break;
  }
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

- (void)controlTextDidChange:(NSNotification *)obj {
  if ([self.formatter getObjectValue:nil
                           forString:self.editorTextField.stringValue
                    errorDescription:nil]) {
    self.okButton.enabled = YES;
  } else {
    self.okButton.enabled = NO;
  }
}

- (void)setupSegmentControlForLiteral {
  self.typeSegmentControl.segmentCount = 2;
  [self.typeSegmentControl setLabel:@"Number" forSegment:0];
  [self.typeSegmentControl setLabel:@"String" forSegment:1];
}

@end
