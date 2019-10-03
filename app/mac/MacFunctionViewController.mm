#import "MacFunctionViewController.h"

#include <vector>

#import "ParameterItem.h"

@interface MacFunctionViewController () <NSCollectionViewDataSource,
                                         NSCollectionViewDelegateFlowLayout,
                                         NSTextFieldDelegate>

@end

@implementation MacFunctionViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.nameTextField.delegate = self;
}

#pragma mark - NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return self.parameters.count;
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ParameterItem *item = [collectionView makeItemWithIdentifier:@"ParameterItem"
                                                  forIndexPath:indexPath];
  item.indexLabel.cell.stringValue = [NSString stringWithFormat:@"%2ld", indexPath.item + 1];
  item.nameTextField.stringValue = [self.parameters objectAtIndex:indexPath.item];
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
    [self.parameters replaceObjectAtIndex:textField.tag withObject:textField.stringValue];
  }
  [self validate];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  [self finishEditing];
}

@end
