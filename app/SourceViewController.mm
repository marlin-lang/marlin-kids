#import "SourceViewController.h"

#include <optional>

#include "toolbox_model.hpp"

#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "Theme.h"

#ifdef IOS
#import "ios/ToolboxCell.h"
#import "ios/ToolboxHeaderView.h"
#else
#import "mac/ToolboxHeaderView.h"
#import "mac/ToolboxItem.h"
#endif

@interface SourceViewController ()

@property(weak) IBOutlet CollectionView *toolboxView;
@property(weak) IBOutlet SourceView *sourceView;

@property(strong) LineNumberView *lineNumberView;

@end

@implementation SourceViewController {
  std::optional<marlin::control::exec_environment> _exec_env;
}

#ifndef IOS

- (void)setDocument:(Document *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceView insertStatementsBeforeLine:1
                                     withSource:std::move(initialData->source)
                                     highlights:std::move(initialData->highlights)];
  }
}

#endif

- (void)viewDidLoad {
  [super viewDidLoad];

  setCurrentTheme([[DefaultTheme alloc] init]);

#ifdef IOS
  self.toolboxView.dragDelegate = self;
#endif
  self.sourceView.dataSource = self;

  /*self.sourceView.enclosingScrollView.rulersVisible = YES;
  self.sourceView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceView.enclosingScrollView.hasVerticalRuler = YES;
  self.lineNumberView = [[LineNumberView alloc] initWithSourceView:self.sourceView];
  self.sourceView.enclosingScrollView.verticalRulerView = self.lineNumberView;*/
}

#ifdef IOS

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

#endif

- (void)prepareForSegue:(StoryboardSegue *)segue sender:(id)sender {
  /*if ([segue.destinationController isKindOfClass:[ExecuteViewController class]]) {
    assert(_exec_env.has_value());

    auto *vc = (ExecuteViewController *)segue.destinationController;
    vc.environment = *std::move(_exec_env);
    _exec_env = std::nullopt;
  }*/
}

- (void)execute {
  [self.lineNumberView clearErrors];
  _exec_env = std::nullopt;
  try {
    _exec_env = self.document.content.generate_exec_environment();
  } catch (marlin::exec::collected_generation_error &e) {
    for (auto &err : e.errors()) {
      [self.sourceView addErrorInSourceRange:err.node().source_code_range];
      [self.lineNumberView addError:[NSString stringWithCString:err.what()
                                                       encoding:NSUTF8StringEncoding]
                             atLine:err.node().source_code_range.begin.line];
    }
  }

  if (_exec_env.has_value()) {
    [self performSegueWithIdentifier:@"ExecuteViewController" sender:self];
  }
}

#pragma mark - CollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(CollectionView *)collectionView {
  return marlin::control::toolbox_model::sections.size();
}

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[section].size();
}

#ifdef IOS

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

#else

- (CollectionViewItem *)collectionView:(CollectionView *)collectionView
    itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolboxItem *item = [collectionView makeItemWithIdentifier:@"ToolboxItem" forIndexPath:indexPath];
  item.textField.stringValue =
      [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(indexPath.section,
                                                                                  indexPath.item)
                                         .name()];
  return item;
}

- (NSView *)collectionView:(NSCollectionView *)collectionView
    viewForSupplementaryElementOfKind:(NSCollectionViewSupplementaryElementKind)kind
                          atIndexPath:(NSIndexPath *)indexPath {
  ToolboxHeaderView *view =
      [collectionView makeSupplementaryViewOfKind:NSCollectionElementKindSectionHeader
                                   withIdentifier:@"ToolboxHeaderView"
                                     forIndexPath:indexPath];
  view.titleTextField.stringValue =
      [NSString stringWithStringView:marlin::control::toolbox_model::sections[indexPath.section]];
  return view;
}

#endif

#pragma mark - CollectionViewDelegateFlowLayout

- (Size)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return MakeSize(collectionView.bounds.size.width - 10, 25);
}

- (Size)collectionView:(CollectionView *)collectionView
                             layout:(CollectionViewLayout *)collectionViewLayout
    referenceSizeForHeaderInSection:(NSInteger)section {
  return MakeSize(collectionView.bounds.size.width - 10, 35);
}

#ifdef IOS

#pragma mark - UICollectionViewDragDelegate

- (NSArray<UIDragItem *> *)collectionView:(UICollectionView *)collectionView
             itemsForBeginningDragSession:(id<UIDragSession>)session
                              atIndexPath:(NSIndexPath *)indexPath {
  auto section = indexPath.section;
  auto item = indexPath.item;
  auto *data =
      [NSData dataWithDataView:marlin::control::toolbox_model::prototype_at(section, item).data()];
  auto *itemProvider = [[NSItemProvider alloc] init];
  [itemProvider
      registerDataRepresentationForTypeIdentifier:
          pasteboardOfType(marlin::control::toolbox_model::items[section][item].type)
                                       visibility:NSItemProviderRepresentationVisibilityAll
                                      loadHandler:^NSProgress *_Nullable(
                                          void (^_Nonnull completionHandler)(NSData *_Nullable,
                                                                             NSError *_Nullable)) {
                                        completionHandler(data, nil);
                                        return nil;
                                      }];
  return @[ [[UIDragItem alloc] initWithItemProvider:itemProvider] ];
}

#else

- (BOOL)collectionView:(NSCollectionView *)collectionView
    canDragItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
                   withEvent:(NSEvent *)event {
  return YES;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView
    writeItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
              toPasteboard:(NSPasteboard *)pasteboard {
  auto section = indexPaths.anyObject.section;
  auto item = indexPaths.anyObject.item;
  NSData *data =
      [NSData dataWithDataView:marlin::control::toolbox_model::prototype_at(section, item).data()];
  return [pasteboard
      setData:data
      forType:pasteboardOfType(marlin::control::toolbox_model::items[section][item].type)];
}

#endif

#pragma mark - NSTextViewDelegate

- (void)textDidChange:(NSNotification *)notification {
  [self.sourceView clearErrors];
  [self.lineNumberView clearErrors];
}

#pragma mark - SourceViewDataSource

- (marlin::control::source_selection)textView:(SourceView *)view
                                  selectionAt:(marlin::source_loc)loc {
  return {_document.content, loc};
}

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceView *)view {
  return {_document.content};
}

- (marlin::control::expression_inserter)expressionInserterForTextView:(SourceView *)view {
  return {_document.content};
}

@end
