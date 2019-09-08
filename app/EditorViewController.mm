#import "EditorViewController.h"

@interface NumberFormatter : NSNumberFormatter

@end

@implementation NumberFormatter

- (instancetype)init {
  if (self = [super init]) {
    self.minimumFractionDigits = 0;
    self.maximumFractionDigits = 10;
  }
  return self;
}

@end

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

@property(weak) IBOutlet Button *okButton;

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

- (void)setType:(EditorType)type {
  _type = type;
  auto *titles = @[ @"Number", @"String", @"Variable" ];
  switch (_type) {
    case EditorType::variable_name:
      _formatter = [VariableFormatter new];
      [self setupSegmentControlWithTitles:@[ @"Variable" ] selection:0];
      break;
    case EditorType::number: {
      _formatter = [[NumberFormatter alloc] init];
      [self setupSegmentControlWithTitles:titles selection:0];
      break;
    }
    case EditorType::string:
      _formatter = [[StringFormatter alloc] init];
      [self setupSegmentControlWithTitles:titles selection:1];
      break;
    case EditorType::identifier:
      _formatter = [[VariableFormatter alloc] init];
      [self setupSegmentControlWithTitles:titles selection:2];
      break;
  }
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
      _formatter = [[NumberFormatter alloc] init];
      self.editorTextField.stringValue = _numberStr;
      break;
    case 1:
      _type = EditorType::string;
      _formatter = [[StringFormatter alloc] init];
      self.editorTextField.stringValue = _stringStr;
      break;
    case 2:
      _type = EditorType::identifier;
      _formatter = [[VariableFormatter alloc] init];
      self.editorTextField.stringValue = _identifierStr;
      break;
    default:
      break;
  }
  [self validate];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self
           finishEditWithString:self.editorTextField.stringValue
                         ofType:self.type];
}

- (void)setupSegmentControlWithTitles:(NSArray<NSString *> *)titles
                            selection:(NSUInteger)selection {
  NSAssert(NO, @"Implemented by subclass");
}

@end
