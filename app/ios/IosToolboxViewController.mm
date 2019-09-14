#import "IosToolboxViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "ToolboxCell.h"

@interface IosToolboxViewController () <UICollectionViewDataSource, UICollectionViewDragDelegate>

@end

@implementation IosToolboxViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.toolboxView.contentInset = UIEdgeInsetsMake(10, 0, 0, 0);
  self.toolboxView.dragDelegate = self;
}

- (UIButton *)buttonWithTitle:(NSString *)title action:(SEL)selector {
  auto button = [UIButton buttonWithType:UIButtonTypeSystem];
  [button setTitle:title forState:UIControlStateNormal];
  [button addTarget:self action:selector forControlEvents:UIControlEventTouchUpInside];
  return button;
}

- (void)setBackgroundColor:(UIColor *)color forButton:(UIButton *)button {
  button.backgroundColor = color;
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)collectionView:(UICollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[self.currentSection].size();
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                  cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"ToolboxCell"
                                                                forIndexPath:indexPath];
  cell.textLabel.text = [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(
                                                           self.currentSection, indexPath.item)
                                                           .name()];
  return cell;
}

#pragma mark - UICollectionViewDragDelegate

- (NSArray<UIDragItem *> *)collectionView:(UICollectionView *)collectionView
             itemsForBeginningDragSession:(id<UIDragSession>)session
                              atIndexPath:(NSIndexPath *)indexPath {
  auto item = indexPath.item;
  auto *data = [NSData
      dataWithDataView:marlin::control::toolbox_model::prototype_at(self.currentSection, item)
                           .data()];
  auto *itemProvider = [[NSItemProvider alloc] init];
  auto *typeIdentifier =
      pasteboardOfType(marlin::control::toolbox_model::items[self.currentSection][item].type);
  [itemProvider
      registerDataRepresentationForTypeIdentifier:typeIdentifier
                                       visibility:NSItemProviderRepresentationVisibilityAll
                                      loadHandler:^NSProgress *_Nullable(
                                          void (^_Nonnull completionHandler)(NSData *_Nullable,
                                                                             NSError *_Nullable)) {
                                        completionHandler(data, nil);
                                        return nil;
                                      }];

  auto *dragItem = [[UIDragItem alloc] initWithItemProvider:itemProvider];
  return @[ dragItem ];
}

@end
