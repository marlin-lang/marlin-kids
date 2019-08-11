#import "SourceViewController.h"

#import "toolbox_model.hpp"

#import "Document.h"
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
  self.sourceTextView.string = [NSString stringWithCString:initialData.source.c_str()
                                                  encoding:NSUTF8StringEncoding];

  auto *theme = [SourceTheme new];
  [theme applyTo:self.sourceTextView.textStorage
               range:NSMakeRange(0, initialData.source.size())
      withHighlights:initialData.highlights];
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
  return marlin::control::toolbox_model::sections().size();
}

- (NSInteger)collectionView:(NSCollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items()[section].size();
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
  view.titleTextField.stringValue =
      @(marlin::control::toolbox_model::sections()[indexPath.section]);
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
  NSString *string = [NSString stringWithFormat:@"%ld,%ld", section, item];
  return [pasteboard setString:string forType:@(marlin::control::toolbox_model::pasteboard_type())];
}

#pragma mark - SourceTextViewDataSource implementation

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView *)textView {
  return {_document.content};
}

- (marlin::ast::base &)textView:(SourceTextView *)textView
          nodeContainsSourceLoc:(marlin::source_loc)loc {
  return _document.content.locate(loc);
}

- (marlin::control::source_replacement)textView:(SourceTextView *)textView
                           replacePlaceholderAt:(marlin::source_loc)loc
                                           type:(EditorType)type
                                     withString:(NSString *)string {
  auto &node = _document.content.locate(loc);
  switch (type) {
    case EditorType::Variable:
      return _document.content
          .replace_with_literal_prototype<marlin::control::variable_name_prototype>(
              node, std::string{string.UTF8String});
    case EditorType::Number:
      return _document.content.replace_with_literal_prototype<marlin::control::number_prototype>(
          node, std::string{string.UTF8String});
    case EditorType::String:
      return _document.content.replace_with_literal_prototype<marlin::control::string_prototype>(
          node, std::string{string.UTF8String});
    case EditorType::Identifier:
      return _document.content
          .replace_with_literal_prototype<marlin::control::identifier_prototype>(
              node, std::string{string.UTF8String});
  }
}

@end
