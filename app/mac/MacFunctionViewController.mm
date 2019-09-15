#import "MacFunctionViewController.h"

#import "NSObject+Casting.h"
#import "ParameterItem.h"

@interface MacFunctionViewController () <NSCollectionViewDataSource,
                                         NSCollectionViewDelegateFlowLayout,
                                         NSTextFieldDelegate>

@property(weak) IBOutlet NSTextField *nameTextField;

@property(weak) IBOutlet NSCollectionView *parametersCollectionView;

@end

@implementation MacFunctionViewController {
  NSMutableArray<NSString *> *_parameters;
}

- (void)viewDidLoad {
  _parameters = [[NSMutableArray alloc] init];
}

- (IBAction)applyButtonPressed:(id)sender {
  [self.delegate viewController:self
          finishEditingWithName:self.nameTextField.stringValue
                     parameters:_parameters];
}

- (IBAction)addButtonPressed:(id)sender {
  [_parameters addObject:@""];
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
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
  auto textField = [NSTextField cast:obj.object];
  [_parameters replaceObjectAtIndex:textField.tag withObject:[textField.stringValue copy]];
}

@end
