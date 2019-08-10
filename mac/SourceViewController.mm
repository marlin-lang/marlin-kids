#import "SourceViewController.h"

#import "prototypes.hpp"

#import "Document.h"
#import "SourceTheme.h"
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

- (NSInteger)collectionView:(NSCollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::statement_prototypes.size();
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolBoxItem *item = [collectionView makeItemWithIdentifier:@"ToolBoxItem" forIndexPath:indexPath];
  const auto &name = marlin::control::statement_prototypes[indexPath.item]->name();
  item.textField.stringValue = [NSString stringWithCString:name.c_str()
                                                  encoding:NSUTF8StringEncoding];
  return item;
}

- (NSSize)collectionView:(NSCollectionView *)collectionView
                    layout:(NSCollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return NSMakeSize(collectionView.bounds.size.width - 40, 30);
}

- (NSRange)textView:(SourceTextView *)textView selectRangeContainsIndex:(NSUInteger)index {
  return NSMakeRange(0, 0);
}

- (BOOL)collectionView:(NSCollectionView *)collectionView
    canDragItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
                   withEvent:(NSEvent *)event {
  return YES;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView
    writeItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
              toPasteboard:(NSPasteboard *)pasteboard {
  return [pasteboard setString:[NSString stringWithFormat:@"%ld", indexPaths.anyObject.item]
                       forType:@"marlin.statement"];
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
        case EditorType::Number:
            return _document.content.replace_expression_with_number_literal(node, {string.UTF8String});
        case EditorType::String:
            return _document.content.replace_expression_with_string_literal(node, {string.UTF8String});
        case EditorType::Variable:
            return _document.content.replace_variable_name(node, {string.UTF8String});
    }
}

@end
