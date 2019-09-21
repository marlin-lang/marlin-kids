#import "MacToolboxViewController.h"

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
  auto button = [NSButton new];
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
  return self.model.current_category_size();
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxItem *item = [collectionView makeItemWithIdentifier:@"ToolboxItem" forIndexPath:indexPath];
  item.textField.stringValue =
      [NSString stringWithStringView:self.model.current_category_prototype(indexPath.item).name];
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
  auto &prototype = self.model.use_current_category_prototype(item);
  NSData *data = [NSData dataWithDataView:prototype.data];
  return [pasteboard setData:data forType:pasteboardOfType(prototype.type)];
}

@end
