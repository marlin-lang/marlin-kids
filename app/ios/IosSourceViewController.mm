#import "IosSourceViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"

#import "IosSourceView.h"
#import "ToolboxCell.h"
#import "ToolboxHeaderView.h"

@interface IosSourceViewController () <UICollectionViewDataSource, UICollectionViewDragDelegate>

@property(strong, nonatomic) IosSourceView *sourceView;

@property(weak, nonatomic) IBOutlet UIScrollView *scrollView;

@end

@implementation IosSourceViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.toolboxView.dragDelegate = self;
  self.sourceView = [[IosSourceView alloc] initWithEnclosingScrollView:self.scrollView
                                                            dataSource:self];
  [self.scrollView addSubview:self.sourceView];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  [self.document openWithCompletionHandler:^(BOOL success) {
    if (success) {
      if (auto initialData = [self.document initialize]) {
        [self.sourceView insertStatementsBeforeLine:1
                                         withSource:std::move(initialData->source)
                                         highlights:std::move(initialData->highlights)];
      }
    } else {
    }
  }];
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(CollectionView *)collectionView {
  return marlin::control::toolbox_model::sections.size();
}

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[section].size();
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                  cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"ToolboxCell"
                                                                forIndexPath:indexPath];
  cell.textLabel.text = [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(
                                                           indexPath.section, indexPath.item)
                                                           .name()];
  return cell;
}

- (UICollectionReusableView *)collectionView:(UICollectionView *)collectionView
           viewForSupplementaryElementOfKind:(NSString *)kind
                                 atIndexPath:(NSIndexPath *)indexPath {
  if (kind == UICollectionElementKindSectionHeader) {
    ToolboxHeaderView *view =
        [collectionView dequeueReusableSupplementaryViewOfKind:kind
                                           withReuseIdentifier:@"ToolboxHeaderView"
                                                  forIndexPath:indexPath];
    view.textLabel.text =
        [NSString stringWithStringView:marlin::control::toolbox_model::sections[indexPath.section]];
    return view;
  } else {
    NSAssert(NO, @"Unsupport kind");
    return nil;
  }
}

#pragma mark - UICollectionViewDragDelegate

- (NSArray<UIDragItem *> *)collectionView:(UICollectionView *)collectionView
             itemsForBeginningDragSession:(id<UIDragSession>)session
                              atIndexPath:(NSIndexPath *)indexPath {
  auto section = indexPath.section;
  auto item = indexPath.item;
  auto *data =
      [NSData dataWithDataView:marlin::control::toolbox_model::prototype_at(section, item).data()];
  auto *itemProvider = [[NSItemProvider alloc] init];
  auto *typeIdentifier =
      pasteboardOfType(marlin::control::toolbox_model::items[section][item].type);
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
