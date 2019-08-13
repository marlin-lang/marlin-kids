#import "SourceViewController.h"

#import "toolbox_model.hpp"

#import "Document.h"
#import "Pasteboard.h"
#import "SourceTheme.h"
#import "ToolBoxHeaderView.h"
#import "ToolBoxItem.h"

@interface SourceViewController ()

@property(weak) IBOutlet NSCollectionView *toolBoxView;
@property(weak) IBOutlet SourceTextView *sourceTextView;
@property(weak) IBOutlet NSTextField *outputTextField;

@end

@implementation SourceViewController

- (void)setDocument:(Document *)document {
  _document = document;

  auto initialData = [self.document initialize];
  [self.sourceTextView updateInRange:NSMakeRange(0, 0)
                          withSource:std::move(initialData.source)
                          highlights:std::move(initialData.highlights)];
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.sourceTextView.dataSource = self;
}

- (IBAction)execute:(id)sender {
  auto &doc = self.document.content;
  doc.execute();
  self.outputTextField.stringValue = [NSString stringWithCString:doc.output().c_str()
                                                        encoding:NSUTF8StringEncoding];
}

#pragma mark - NSCollectionViewDataSource implementation

- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView *)collectionView {
  return marlin::control::toolbox_model::sections.size();
}

- (NSInteger)collectionView:(NSCollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[section].size();
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolBoxItem *item = [collectionView makeItemWithIdentifier:@"ToolBoxItem" forIndexPath:indexPath];
  item.textField.stringValue =
      @(marlin::control::toolbox_model::nameOfItemAt(indexPath.section, indexPath.item).c_str());
  return item;
}

- (NSView *)collectionView:(NSCollectionView *)collectionView
    viewForSupplementaryElementOfKind:(NSCollectionViewSupplementaryElementKind)kind
                          atIndexPath:(NSIndexPath *)indexPath {
  ToolBoxHeaderView *view =
      [collectionView makeSupplementaryViewOfKind:NSCollectionElementKindSectionHeader
                                   withIdentifier:@"ToolBoxHeaderView"
                                     forIndexPath:indexPath];
  view.titleTextField.stringValue = @(marlin::control::toolbox_model::sections[indexPath.section]);
  return view;
}

#pragma mark - NSCollectionViewDelegateFlowLayout implementation

- (NSSize)collectionView:(NSCollectionView *)collectionView
                    layout:(NSCollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return NSMakeSize(collectionView.bounds.size.width - 40, 30);
}

- (NSSize)collectionView:(NSCollectionView *)collectionView
                             layout:(NSCollectionViewLayout *)collectionViewLayout
    referenceSizeForHeaderInSection:(NSInteger)section {
  return NSMakeSize(collectionView.bounds.size.width - 40, 30);
}

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
  NSString *string = [NSString
      stringWithFormat:@"%ld", marlin::control::toolbox_model::items[section][item].index];
  return [pasteboard
      setString:string
        forType:pasteboardOfType(marlin::control::toolbox_model::items[section][item].type)];
}

#pragma mark - SourceTextViewDataSource implementation

- (marlin::control::source_selection)textView:(SourceTextView *)textView
                                  selectionAt:(marlin::source_loc)loc {
  return {_document.content, loc};
}

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView *)textView {
  return {_document.content};
}

- (marlin::control::expression_inserter)expressionInserterForTextView:(SourceTextView *)textView {
  return {_document.content};
}

@end
