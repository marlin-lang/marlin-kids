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
}

- (void)viewDidLoad {
  self.nameTextField.delegate = self;
  self.nameTextField.tag = -1;
  _formatter = [VariableFormatter new];
  _parameters = [NSMutableArray new];
}

VIEWDIDAPPEAR_BEGIN

[self.nameTextField becomeFirstResponder];

VIEWDIDAPPEAR_END

- (void)setFunctionSignature:(marlin::function_definition)signature {
  _nameTextField.stringValue = [NSString stringWithStringView:signature.name];
  for (auto &parameter : signature.parameters) {
    [_parameters addObject:[NSString stringWithStringView:parameter]];
  }
  [self.parametersCollectionView reloadData];
  [self validate];
}

- (IBAction)addButtonPressed:(id)sender {
  [_parameters addObject:@""];
  _isValid = NO;
  [self.parametersCollectionView reloadData];
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

- (void)removeButtonPressed:(NSButton *)sender {
  auto index = sender.tag;
  NSAssert(index >= 0 && index < _parameters.count, @"");
  [_parameters removeObjectAtIndex:index];
  [self.parametersCollectionView reloadData];
  [self validate];
  [self finishEditing];
}

- (void)finishEditing {
  if (_isValid) {
    [self.delegate functionViewController:self
                    finishEditingWithName:self.nameTextField.stringValue
                               parameters:_parameters];
  }
}

- (BOOL)haveDuplicatedParameters {
  auto parameters = [NSSet setWithArray:_parameters];
  return parameters.count != _parameters.count;
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
  [self validate];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  [self finishEditing];
}

@end
