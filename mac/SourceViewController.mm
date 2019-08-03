#import "SourceViewController.h"

#import "Document.h"
#import "SourceTheme.h"
#import "ToolBoxItem.h"

@interface SourceViewController ()

@property(weak) IBOutlet NSCollectionView *toolBoxView;
@property(weak) IBOutlet SourceTextView *sourceTextView;
@property(weak) IBOutlet NSTextField *outputTextField;

@end

@implementation SourceViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.sourceTextView.dataSource = self;
}

- (void)setNeedsUpdate {
  [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(update) object:nil];
  [self performSelector:@selector(update) withObject:nil afterDelay:0];
}

- (void)update {
  auto &doc = self.document.content;
  auto &source_str = doc.source_str();
  self.sourceTextView.string = [NSString stringWithCString:source_str.c_str()
                                                  encoding:NSUTF8StringEncoding];
  auto *theme = [SourceTheme new];
  [self.sourceTextView.textStorage setAttributes:theme.allAttrs
                                           range:NSMakeRange(0, source_str.size())];
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
  NSArray *names = @[ @"let", @"+" ];
  item.textField.stringValue = names[indexPath.item];
  return item;
}

- (NSSize)collectionView:(NSCollectionView *)collectionView
                    layout:(NSCollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return NSMakeSize(collectionView.bounds.size.width - 40, 30);
}

- (NSRange)textView:(SourceTextView *)textView selectRageContainsIndex:(NSUInteger)index {
  return NSMakeRange(0, 0);
}

@end
