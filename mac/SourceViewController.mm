#import "SourceViewController.h"

#import "toolbox_model.hpp"

#import "Document.h"
#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "SourceTheme.h"
#import "ToolBoxHeaderView.h"
#import "ToolBoxItem.h"

@interface SourceViewController ()

@property(weak) IBOutlet NSCollectionView *toolBoxView;
@property(weak) IBOutlet SourceTextView *sourceTextView;
@property(weak) IBOutlet NSTextField *outputTextField;

@property(strong) LineNumberView *lineNumberView;

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
  self.sourceTextView.delegate = self;

  self.sourceTextView.rulerVisible = YES;
  self.sourceTextView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceTextView.enclosingScrollView.hasVerticalRuler = YES;
  self.lineNumberView = [[LineNumberView alloc] initWithTextView:self.sourceTextView];
  self.sourceTextView.enclosingScrollView.verticalRulerView = self.lineNumberView;
}

- (IBAction)execute:(id)sender {
  auto &doc = self.document.content;
  doc.execute([self](const marlin::ast::base &node, const std::string &message) {
    [self.sourceTextView addError:[NSString stringWithStringView:message]
                    atSourceRange:node.source_code_range];
  });
  [self.sourceTextView showErrors];
  self.outputTextField.stringValue = [NSString stringWithStringView:doc.output()];
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
      [NSString stringWithStringView:marlin::control::toolbox_model::prototype_at(indexPath.section,
                                                                                  indexPath.item)
                                         .name()];
  return item;
}

- (NSView *)collectionView:(NSCollectionView *)collectionView
    viewForSupplementaryElementOfKind:(NSCollectionViewSupplementaryElementKind)kind
                          atIndexPath:(NSIndexPath *)indexPath {
  ToolBoxHeaderView *view =
      [collectionView makeSupplementaryViewOfKind:NSCollectionElementKindSectionHeader
                                   withIdentifier:@"ToolBoxHeaderView"
                                     forIndexPath:indexPath];
  view.titleTextField.stringValue =
      [NSString stringWithStringView:marlin::control::toolbox_model::sections[indexPath.section]];
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
  NSData *data =
      [NSData dataWithDataView:marlin::control::toolbox_model::prototype_at(section, item).data()];
  return [pasteboard
      setData:data
      forType:pasteboardOfType(marlin::control::toolbox_model::items[section][item].type)];
}

#pragma mark - NSTextViewDelegate implementation

- (void)textDidChange:(NSNotification *)notification {
  [self.lineNumberView setNeedsDisplay:YES];
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
