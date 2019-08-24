#import "SourceViewController.h"

#import <optional>

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

@property(strong) LineNumberView *lineNumberView;

@end

@implementation SourceViewController {
  std::optional<marlin::control::exec_environment> _exec_env;
}

- (void)setDocument:(Document *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceTextView insertBeforeLine:1
                               withSource:std::move(initialData->source)
                               highlights:std::move(initialData->highlights)];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.sourceTextView.dataSource = self;

  self.sourceTextView.enclosingScrollView.rulersVisible = YES;
  self.sourceTextView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceTextView.enclosingScrollView.hasVerticalRuler = YES;
  self.lineNumberView = [[LineNumberView alloc] initWithTextView:self.sourceTextView];
  self.sourceTextView.enclosingScrollView.verticalRulerView = self.lineNumberView;
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
  if ([segue.destinationController isKindOfClass:[ExecuteViewController class]]) {
    assert(_exec_env.has_value());

    auto *vc = (ExecuteViewController *)segue.destinationController;
    vc.environment = *std::move(_exec_env);
    _exec_env = std::nullopt;
  }
}

- (void)execute {
  [self.lineNumberView clearErrors];
  _exec_env = std::nullopt;
  try {
    _exec_env = self.document.content.generate_exec_environment();
  } catch (marlin::exec::collected_generation_error &e) {
    for (auto &err : e.errors()) {
      [self.sourceTextView addErrorInSourceRange:err.node().source_code_range];
      [self.lineNumberView addError:[NSString stringWithCString:err.what()
                                                       encoding:NSUTF8StringEncoding]
                             atLine:err.node().source_code_range.begin.line];
    }
  }

  if (_exec_env.has_value()) {
    [self performSegueWithIdentifier:@"ExecuteViewController" sender:self];
  }
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
  return NSMakeSize(collectionView.bounds.size.width - 10, 25);
}

- (NSSize)collectionView:(NSCollectionView *)collectionView
                             layout:(NSCollectionViewLayout *)collectionViewLayout
    referenceSizeForHeaderInSection:(NSInteger)section {
  return NSMakeSize(collectionView.bounds.size.width - 10, 30);
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
  [self.sourceTextView clearErrors];
  [self.lineNumberView clearErrors];
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
