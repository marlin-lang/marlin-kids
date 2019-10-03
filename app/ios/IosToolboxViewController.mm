#import "IosToolboxViewController.h"

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
  return self.model.current_category_size();
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                  cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"ToolboxCell"
                                                                forIndexPath:indexPath];
  cell.textLabel.text =
      [NSString stringWithStringView:self.model.current_category_prototype(indexPath.item).name];
  return cell;
}

#pragma mark - UICollectionViewDragDelegate

- (NSArray<UIDragItem *> *)collectionView:(UICollectionView *)collectionView
             itemsForBeginningDragSession:(id<UIDragSession>)session
                              atIndexPath:(NSIndexPath *)indexPath {
  auto item = indexPath.item;
  auto &prototype = self.model.use_current_category_prototype(item);
  auto *data = [NSData dataWithDataView:prototype.data];
  auto *itemProvider = [NSItemProvider new];
  auto *typeIdentifier = pasteboardOfType(prototype.type);
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
