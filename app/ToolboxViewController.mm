#import "ToolboxViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "mac/ToolboxItem.h"

@interface ToolboxViewController () <NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout>

@property(weak) IBOutlet NSStackView *stackView;
@property(weak) IBOutlet NSCollectionView *toolboxView;

@end

@implementation ToolboxViewController {
  NSInteger _currentSection;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  [self createButtonWithTitle:@"r" tag:-1];
  auto index = 0;
  for (auto &section : marlin::control::toolbox_model::sections) {
    auto title = [NSString stringWithStringView:{section.data(), 1}];
    [self createButtonWithTitle:title tag:index++];
  }
}

- (void)createButtonWithTitle:(NSString *)title tag:(NSInteger)tag {
  auto button = [[NSButton alloc] init];
  button.bordered = NO;
  button.target = self;
  button.action = @selector(sectionButtonPressed:);
  button.tag = tag;
  button.translatesAutoresizingMaskIntoConstraints = NO;
  [button setTitle:title];
  [self.stackView addArrangedSubview:button];
  [button.widthAnchor constraintEqualToAnchor:self.stackView.widthAnchor].active = YES;
  [button.widthAnchor constraintEqualToAnchor:button.heightAnchor].active = YES;
}

- (void)sectionButtonPressed:(NSButton *)sender {
  if (sender.tag >= 0) {
    _currentSection = sender.tag;
    [self.toolboxView reloadData];
  }
}

#pragma mark - NSCollectionViewDataSource

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[_currentSection].size();
}

- (CollectionViewItem *)collectionView:(CollectionView *)collectionView
    itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxItem *item = [collectionView makeItemWithIdentifier:@"ToolboxItem" forIndexPath:indexPath];
  item.textField.stringValue =
      [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(_currentSection,
                                                                                  indexPath.item)
                                         .name()];
  return item;
}

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

#pragma mark - NSCollectionViewDelegate

- (BOOL)collectionView:(NSCollectionView *)collectionView
    canDragItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
                   withEvent:(NSEvent *)event {
  return YES;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView
    writeItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
              toPasteboard:(NSPasteboard *)pasteboard {
  auto item = indexPaths.anyObject.item;
  NSData *data = [NSData
      dataWithDataView:marlin::control::toolbox_model::prototype_at(_currentSection, item).data()];
  return [pasteboard
      setData:data
      forType:pasteboardOfType(marlin::control::toolbox_model::items[_currentSection][item].type)];
}

@end
