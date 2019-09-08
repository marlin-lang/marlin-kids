#import "IosSourceViewController.h"

#include "toolbox_model.hpp"

#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"

#import "IosSourceView.h"
#import "ToolboxCell.h"
#import "ToolboxHeaderView.h"

@interface IosSourceViewController () <UICollectionViewDataSource,
                                       UICollectionViewDragDelegate,
                                       UIScrollViewDelegate,
                                       SourceViewDelegate>

@property(strong, nonatomic) IosSourceView *sourceView;

@property(weak, nonatomic) IBOutlet UIScrollView *scrollView;

@end

@implementation IosSourceViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.toolboxView.dragDelegate = self;
  self.scrollView.delegate = self;
  self.sourceView = [[IosSourceView alloc] initWithEnclosingScrollView:self.scrollView
                                                            dataSource:self];
  self.sourceView.delegate = self;
  [self.scrollView addSubview:self.sourceView];

  auto *lineNumberView = [[LineNumberView alloc] initWithSourceView:self.sourceView];
  self.lineNumberView = lineNumberView;
  self.sourceView.lineNumberView = lineNumberView;
  lineNumberView.translatesAutoresizingMaskIntoConstraints = NO;
  self.scrollView.contentInset = UIEdgeInsetsMake(0, lineNumberView.ruleThickness, 0, 0);
  [self.view addSubview:lineNumberView];
  [lineNumberView.topAnchor constraintEqualToAnchor:self.scrollView.topAnchor].active = YES;
  [lineNumberView.leftAnchor constraintEqualToAnchor:self.scrollView.leftAnchor].active = YES;
  [lineNumberView.heightAnchor constraintEqualToAnchor:self.view.heightAnchor].active = YES;
  [lineNumberView.widthAnchor constraintEqualToConstant:lineNumberView.ruleThickness].active = YES;

    [self.document openWithCompletionHandler:^(BOOL success) {
        if (success) {
            if (auto initialData = [self.document initialize]) {
                [self.sourceView insertStatementsBeforeLine:1
                                                 withSource:std::move(initialData->source)
                                                 highlights:std::move(initialData->highlights)];
                [self.lineNumberView setNeedsDisplay];
            }
        } else {
        }
    }];
}

- (IBAction)run:(id)sender {
    [self execute];
}

- (UIViewController*)destinationViewControllerOfSegue:(UIStoryboardSegue*)segue {
    return segue.destinationViewController;
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

#pragma mark - UIScrollViewDelegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.lineNumberView setNeedsDisplay];
}

#pragma mark - SourceViewDelegate

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(CGRect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  EditorViewController *vc =
      [self.storyboard instantiateViewControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;
  vc.modalPresentationStyle = UIModalPresentationPopover;
  vc.popoverPresentationController.permittedArrowDirections = UIMenuControllerArrowUp;
  vc.popoverPresentationController.sourceView = view;
  vc.popoverPresentationController.sourceRect = rect;
  [self presentViewController:vc animated:YES completion:nil];
  vc.type = type;
  vc.editorTextField.text = [NSString stringWithStringView:data];
}

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view {
  [self dismissViewControllerAnimated:YES completion:nil];
}

@end
