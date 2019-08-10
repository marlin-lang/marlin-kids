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
@property(weak) IBOutlet NSButton *okButton;

@property(nonatomic, strong) NSFormatter *formatter;

@property(nonatomic, strong) NSString *numberStr;
@property(nonatomic, strong) NSString *stringStr;
@property(nonatomic, strong) NSString *identifierStr;

@end

@implementation EditorViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.editorTextField.delegate = self;
  self.okButton.enabled = NO;
  self.numberStr = @"";
  self.stringStr = @"";
  self.identifierStr = @"";
}

- (void)setType:(EditorType)type {
  _type = type;
  switch (_type) {
    case EditorType::Variable:
      self.formatter = [VariableFormatter new];
      self.typeSegmentControl.segmentCount = 1;
      [self.typeSegmentControl setLabel:@"Variable" forSegment:0];
      [self.typeSegmentControl setSelected:YES forSegment:0];
      break;
    case EditorType::Number: {
      NSNumberFormatter *formatter = [NSNumberFormatter new];
      formatter.minimumFractionDigits = 0;
      formatter.maximumFractionDigits = 10;
      self.formatter = formatter;
      [self setupSegmentControlForRightValue];
      [self.typeSegmentControl setSelected:YES forSegment:0];
    } break;
    case EditorType::String:
      self.formatter = [StringFormatter new];
      [self setupSegmentControlForRightValue];
      [self.typeSegmentControl setSelected:YES forSegment:1];
      break;
    case EditorType::Identifier:
      self.formatter = [VariableFormatter new];
      [self setupSegmentControlForRightValue];
      [self.typeSegmentControl setSelected:YES forSegment:2];
      break;
  }
}

- (IBAction)typeSegmentControlChanged:(id)sender {
  switch (self.type) {
    case EditorType::Number:
      self.numberStr = self.editorTextField.stringValue;
      break;
    case EditorType::String:
      self.stringStr = self.editorTextField.stringValue;
      break;
    case EditorType::Identifier:
      self.identifierStr = self.editorTextField.stringValue;
      break;
    default:
      break;
  }
  switch (self.typeSegmentControl.selectedSegment) {
    case 0:
      self.type = EditorType::Number;
      self.editorTextField.stringValue = self.numberStr;
      break;
    case 1:
      self.type = EditorType::String;
      self.editorTextField.stringValue = self.stringStr;
      break;
    case 2:
      self.type = EditorType::Identifier;
      self.editorTextField.stringValue = self.identifierStr;
      break;
    default:
      break;
  }
  [self validate];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self finishEditWithString:self.editorTextField.stringValue];
}

- (void)controlTextDidChange:(NSNotification *)obj {
  [self validate];
}

- (void)setupSegmentControlForRightValue {
  self.typeSegmentControl.segmentCount = 3;
  [self.typeSegmentControl setLabel:@"Number" forSegment:0];
  [self.typeSegmentControl setLabel:@"String" forSegment:1];
  [self.typeSegmentControl setLabel:@"Identifier" forSegment:2];
}

- (void)validate {
  if ([self.formatter getObjectValue:nil
                           forString:self.editorTextField.stringValue
                    errorDescription:nil]) {
    self.okButton.enabled = YES;
  } else {
    self.okButton.enabled = NO;
  }
}

@end
