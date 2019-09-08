#import "MacSourceViewController.h"

#include "toolbox_model.hpp"

#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"

#import "MacSourceView.h"
#import "ToolboxHeaderView.h"
#import "ToolboxItem.h"

@interface MacSourceViewController () <NSCollectionViewDataSource, SourceViewDelegate>

@property(weak) IBOutlet MacSourceView *sourceView;

@end

@implementation MacSourceViewController {
  NSPopover *_popover;
}

- (void)setDocument:(MacDocument *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceView insertStatementsBeforeLine:1
                                     withSource:std::move(initialData->source)
                                     highlights:std::move(initialData->highlights)];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];
  self.sourceView.dataSource = self;
  self.sourceView.delegate = self;
  self.sourceView.enclosingScrollView.rulersVisible = YES;
  self.sourceView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceView.enclosingScrollView.hasVerticalRuler = YES;
  self.sourceView.enclosingScrollView.verticalRulerView =
      [[LineNumberView alloc] initWithSourceView:self.sourceView];
}

- (NSViewController*)destinationViewControllerOfSegue:(NSStoryboardSegue*)segue {
    return segue.destinationController;
}

#pragma mark - NSCollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(CollectionView *)collectionView {
  return marlin::control::toolbox_model::sections.size();
}

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[section].size();
}

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

#pragma mark - NSCollectionViewDelegate

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

#pragma mark - SourceViewDelegate

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(NSRect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  auto *storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
  EditorViewController *vc =
      [storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;

  _popover = [NSPopover new];
  _popover.behavior = NSPopoverBehaviorTransient;
  _popover.contentViewController = vc;
  [_popover showRelativeToRect:rect ofView:view preferredEdge:NSMinYEdge];

  vc.type = type;
  vc.editorTextField.stringValue = [NSString stringWithStringView:data];
}

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view {
  [_popover close];
}

@end
