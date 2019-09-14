#import "MacToolboxViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSObject+Casting.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "ToolboxItem.h"

@interface MacToolboxViewController () <NSCollectionViewDataSource>

@end

@implementation MacToolboxViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.wantsLayer = YES;
  self.view.layer.backgroundColor = NSColor.whiteColor.CGColor;
  self.toolboxView.backgroundColors = @[ [NSColor colorWithWhite:0.92 alpha:1] ];
}

- (NSButton *)buttonWithTitle:(NSString *)title action:(SEL)selector {
  auto button = [[NSButton alloc] init];
  button.bordered = NO;
  button.title = title;
  button.target = self;
  button.action = selector;
  return button;
}

- (void)setBackgroundColor:(NSColor *)color forButton:(NSButton *)button {
  [NSButtonCell cast:button.cell].backgroundColor = color;
}

#pragma mark - NSCollectionViewDataSource

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[self.currentSection].size();
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxItem *item = [collectionView makeItemWithIdentifier:@"ToolboxItem" forIndexPath:indexPath];
  item.textField.stringValue =
      [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(
                                         self.currentSection, indexPath.item)
                                         .name()];
  return item;
}

#pragma mark - CollectionViewDelegate

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
      dataWithDataView:marlin::control::toolbox_model::prototype_at(self.currentSection, item)
                           .data()];
  return [pasteboard
      setData:data
      forType:pasteboardOfType(
                  marlin::control::toolbox_model::items[self.currentSection][item].type)];
}

@end
