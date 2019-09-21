#import "MacFunctionViewController.h"

#include <vector>

#import "Formatter.h"
#import "ParameterItem.h"

@interface MacFunctionViewController () <NSCollectionViewDataSource,
                                         NSCollectionViewDelegateFlowLayout,
                                         NSTextFieldDelegate>

@property(weak) IBOutlet NSButton *okButton;
@property(weak) IBOutlet NSTextField *nameTextField;
@property(weak) IBOutlet NSCollectionView *parametersCollectionView;

@end

@implementation MacFunctionViewController {
  VariableFormatter *_formatter;
  NSMutableArray<NSString *> *_parameters;
  bool _isNameValidated;
  std::vector<bool> _isParametersValidated;
}

- (void)viewDidLoad {
  self.okButton.enabled = NO;
  self.nameTextField.delegate = self;
  self.nameTextField.tag = -1;
  _formatter = [[VariableFormatter alloc] init];
  _parameters = [[NSMutableArray alloc] init];
}

- (void)setFunctionSignature:(marlin::function_definition)signature {
  _nameTextField.stringValue = [NSString stringWithStringView:signature.name];
  _isNameValidated = true;
  for (auto &parameter : signature.parameters) {
    [_parameters addObject:[NSString stringWithStringView:parameter]];
    _isParametersValidated.push_back(true);
  }
  [self.parametersCollectionView reloadData];
  [self validateForTextField:_nameTextField];
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate viewController:self
          finishEditingWithName:self.nameTextField.stringValue
                     parameters:_parameters];
}

- (IBAction)addButtonPressed:(id)sender {
  [_parameters addObject:@""];
  _isParametersValidated.push_back(false);
  self.okButton.enabled = NO;
  [self.parametersCollectionView reloadData];
}

- (void)validateForTextField:(NSTextField *)textField {
  auto index = textField.tag;
  if ([_formatter getObjectValue:nil forString:textField.stringValue errorDescription:nil]) {
    if (index >= 0) {
      _isParametersValidated[index] = true;
    } else {
      _isNameValidated = true;
    }
    self.okButton.enabled =
        _isNameValidated && std::find(_isParametersValidated.begin(), _isParametersValidated.end(),
                                      false) == _isParametersValidated.end();
  } else {
    if (index >= 0) {
      _isParametersValidated[index] = false;
    } else {
      _isNameValidated = false;
    }
    self.okButton.enabled = NO;
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
  [self validateForTextField:[NSTextField cast:obj.object]];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  auto textField = [NSTextField cast:obj.object];
  if (textField.tag >= 0) {
    [_parameters replaceObjectAtIndex:textField.tag withObject:[textField.stringValue copy]];
  }
}

@end
