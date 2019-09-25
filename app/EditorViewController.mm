#import "EditorViewController.h"

#import "Formatter.h"

@interface EditorViewController ()

@property(weak) IBOutlet NSButton *okButton;

@end

@implementation EditorViewController {
  NSFormatter *_formatter;

  NSString *_numberStr;
  NSString *_stringStr;
  NSString *_identifierStr;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.okButton.enabled = NO;
  _numberStr = @"";
  _stringStr = @"";
  _identifierStr = @"";
}

- (void)viewDidAppear {
  [super viewDidAppear];
  [self.editorTextField becomeFirstResponder];
}

- (void)setType:(EditorType)type {
  _type = type;
  auto *titles = @[ @"Num", @"Str", @"Var" ];
  switch (_type) {
    case EditorType::parameter:
      _formatter = [VariableFormatter new];
      [self setupSegmentControlWithTitles:@[ @"Param" ] selection:0];
      break;
    case EditorType::variable_name:
      _formatter = [VariableFormatter new];
      [self setupSegmentControlWithTitles:@[ @"Var" ] selection:0];
      break;
    case EditorType::number: {
      _formatter = [NumberFormatter new];
      [self setupSegmentControlWithTitles:titles selection:0];
      break;
    }
    case EditorType::string:
      _formatter = [StringFormatter new];
      [self setupSegmentControlWithTitles:titles selection:1];
      break;
    case EditorType::identifier:
      _formatter = [VariableFormatter new];
      [self setupSegmentControlWithTitles:titles selection:2];
      break;
  }
  [self validate];
}

- (void)setEditorString:(NSString *)string {
  self.editorTextField.stringValue = string;
  [self validate];
}

- (void)validate {
  if ([_formatter getObjectValue:nil
                       forString:self.editorTextField.stringValue
                errorDescription:nil]) {
    self.okButton.enabled = YES;
  } else {
    self.okButton.enabled = NO;
  }
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate editorViewController:self
                 finishEditWithString:self.editorTextField.stringValue
                               ofType:self.type];
}

- (IBAction)typeSegmentControlChanged:(id)sender {
  switch (self.type) {
    case EditorType::number:
      _numberStr = self.editorTextField.stringValue;
      break;
    case EditorType::string:
      _stringStr = self.editorTextField.stringValue;
      break;
    case EditorType::identifier:
      _identifierStr = self.editorTextField.stringValue;
      break;
    default:
      break;
  }
  switch (self.typeSegmentControl.selectedSegment) {
    case 0:
      _type = EditorType::number;
      _formatter = [NumberFormatter new];
      self.editorTextField.stringValue = _numberStr;
      break;
    case 1:
      _type = EditorType::string;
      _formatter = [StringFormatter new];
      self.editorTextField.stringValue = _stringStr;
      break;
    case 2:
      _type = EditorType::identifier;
      _formatter = [VariableFormatter new];
      self.editorTextField.stringValue = _identifierStr;
      break;
    default:
      break;
  }
  [self validate];
}

- (void)setupSegmentControlWithTitles:(NSArray<NSString *> *)titles
                            selection:(NSUInteger)selection {
  NSAssert(NO, @"Implemented by subclass");
}

@end
