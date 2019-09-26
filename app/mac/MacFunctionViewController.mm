#import "MacFunctionViewController.h"

#include <vector>

#import "Formatter.h"
#import "ParameterItem.h"

@interface MacFunctionViewController () <NSCollectionViewDataSource,
                                         NSCollectionViewDelegateFlowLayout,
                                         NSTextFieldDelegate>

@property(weak) IBOutlet NSTextField *nameTextField;
@property(weak) IBOutlet NSCollectionView *parametersCollectionView;

@end

@implementation MacFunctionViewController {
  VariableFormatter *_formatter;
  NSMutableArray<NSString *> *_parameters;
  BOOL _isValid;
  BOOL _isNameValid;
  std::vector<bool> _isParameterValid;
}

- (void)viewDidLoad {
  self.nameTextField.delegate = self;
  self.nameTextField.tag = -1;
  _formatter = [[VariableFormatter alloc] init];
  _parameters = [[NSMutableArray alloc] init];
}

- (void)viewDidAppear {
  [super viewDidAppear];
  [self.nameTextField becomeFirstResponder];
}

- (void)setFunctionSignature:(marlin::function_definition)signature {
  _nameTextField.stringValue = [NSString stringWithStringView:signature.name];
  _isNameValid = true;
  for (auto &parameter : signature.parameters) {
    [_parameters addObject:[NSString stringWithStringView:parameter]];
    _isParameterValid.push_back(true);
  }
  [self.parametersCollectionView reloadData];
  [self validateForTextField:_nameTextField];
}

- (IBAction)addButtonPressed:(id)sender {
  [_parameters addObject:@""];
  _isParameterValid.push_back(false);
  _isValid = NO;
  [self.parametersCollectionView reloadData];
}

- (void)validateForTextField:(NSTextField *)textField {
  auto index = textField.tag;
  if ([_formatter getObjectValue:nil forString:textField.stringValue errorDescription:nil]) {
    if (index >= 0) {
      _isParameterValid[index] = true;
    } else {
      _isNameValid = YES;
    }
    _isValid = _isNameValid && std::find(_isParameterValid.begin(), _isParameterValid.end(),
                                         false) == _isParameterValid.end();
  } else {
    if (index >= 0) {
      _isParameterValid[index] = false;
    } else {
      _isNameValid = false;
    }
    _isValid = NO;
  }
}

- (void)removeButtonPressed:(NSButton *)sender {
  auto index = sender.tag;
  NSAssert(index >= 0 && index < _parameters.count, @"");
  [_parameters removeObjectAtIndex:index];
  [self.parametersCollectionView reloadData];
}

#pragma mark - NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return _parameters.count;
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ParameterItem *item = [collectionView makeItemWithIdentifier:@"ParameterItem"
                                                  forIndexPath:indexPath];
  item.indexLabel.cell.stringValue = [NSString stringWithFormat:@"%2ld", indexPath.item + 1];
  item.nameTextField.stringValue = [_parameters objectAtIndex:indexPath.item];
  item.nameTextField.tag = indexPath.item;
  item.nameTextField.delegate = self;
  item.removeButton.tag = indexPath.item;
  item.removeButton.target = self;
  item.removeButton.action = @selector(removeButtonPressed:);
  return item;
}

#pragma mark - NSCollectionViewDelegateFlowLayout

- (CGSize)collectionView:(NSCollectionView *)collectionView
                    layout:(NSCollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

#pragma mark - NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)obj {
  auto textField = [NSTextField cast:obj.object];
  if (textField.tag >= 0) {
    [_parameters replaceObjectAtIndex:textField.tag withObject:[textField.stringValue copy]];
  }
  [self validateForTextField:textField];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  if (_isValid) {
    [self.delegate functionViewController:self
                    finishEditingWithName:self.nameTextField.stringValue
                               parameters:_parameters];
  }
}

@end
