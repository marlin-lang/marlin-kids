#import "FunctionViewController.h"

#import "Formatter.h"

@implementation FunctionViewController {
  VariableFormatter *_formatter;
  BOOL _isValid;
}

- (void)viewDidLoad {
  self.nameTextField.tag = -1;
  _formatter = [VariableFormatter new];
  _parameters = [NSMutableArray new];
}

VIEWDIDAPPEAR_BEGIN

    [self.nameTextField becomeFirstResponder];

VIEWDIDAPPEAR_END

- (IBAction)addButtonPressed:(id)sender {
  [self.parameters addObject:@""];
  _isValid = NO;
  [self.parametersCollectionView reloadData];
}

- (void)removeButtonPressed:(Button *)sender {
  auto index = sender.tag;
  NSAssert(index >= 0 && index < _parameters.count, @"");
  [self.parameters removeObjectAtIndex:index];
  [self.parametersCollectionView reloadData];
  [self validate];
  [self finishEditing];
}

- (void)finishEditing {
  if (_isValid) {
    [self.delegate functionViewController:self
                    finishEditingWithName:self.nameTextField.stringValue
                               parameters:self.parameters];
  }
}

- (void)setFunctionSignature:(marlin::function_definition)signature {
  _nameTextField.stringValue = [NSString stringWithStringView:signature.name];
  for (auto &parameter : signature.parameters) {
    [self.parameters addObject:[NSString stringWithStringView:parameter]];
  }
  [self.parametersCollectionView reloadData];
  [self validate];
}

- (void)validate {
  _isValid = YES;
  if (![_formatter getObjectValue:nil
                        forString:self.nameTextField.stringValue
                 errorDescription:nil]) {
    _isValid = NO;
  } else {
    for (NSString *parameter in _parameters) {
      if (![_formatter getObjectValue:nil forString:parameter errorDescription:nil]) {
        _isValid = NO;
      }
    }
  }
  _isValid = _isValid && !self.haveDuplicatedParameters;
}

- (BOOL)haveDuplicatedParameters {
  auto parameters = [NSSet setWithArray:self.parameters];
  return parameters.count != self.parameters.count;
}

@end
