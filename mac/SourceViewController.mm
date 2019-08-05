#import "SourceViewController.h"

#import "Document.h"
#import "SourceTheme.h"
#import "ToolBoxItem.h"

#import <iostream>

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
  return 2;
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
  ToolBoxItem *item = [collectionView makeItemWithIdentifier:@"ToolBoxItem" forIndexPath:indexPath];
  auto *names = @[ @"let", @"+" ];
  item.textField.stringValue = names[indexPath.item];
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

- (NSString *)textView:(SourceTextView *)textView
    insertStatementByIndex:(NSUInteger)index
                    atLine:(NSUInteger)line {
  auto loc = _document.content.find_statement_insert_location(line);
  auto update = _document.content.insert_statement(*loc, marlin::control::statement_prototypes[0]);
  return [NSString stringWithCString:update.source.c_str() encoding:NSUTF8StringEncoding];
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
                       forType:NSPasteboardTypeString];
}

- (NSRange)textView:(SourceTextView *)textView selectRageContainsIndex:(NSUInteger)index {
  return NSMakeRange(0, 0);
}

@end
