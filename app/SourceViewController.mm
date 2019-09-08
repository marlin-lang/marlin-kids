#import "SourceViewController.h"

#include <optional>

#include "toolbox_model.hpp"

#import "Document.h"
#import "ExecuteViewController.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "Theme.h"

@interface SourceViewController ()

- (Document *)document;

- (SourceView *)sourceView;

@end

@implementation SourceViewController {
  std::optional<marlin::control::exec_environment> _exec_env;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  setCurrentTheme([[DefaultTheme alloc] init]);
}

- (Document *)document {
  NSAssert(NO, @"Implement by subclass");
  return nil;
}

- (SourceView *)sourceView {
  NSAssert(NO, @"Implement by subclass");
  return nil;
}

- (void)prepareForSegue:(StoryboardSegue *)segue sender:(id)sender {
#ifndef IOS
  if ([segue.destinationController isKindOfClass:[ExecuteViewController class]]) {
    assert(_exec_env.has_value());

    auto *vc = (ExecuteViewController *)segue.destinationController;
    vc.environment = *std::move(_exec_env);
    _exec_env = std::nullopt;
  }
#endif
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

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

- (CGSize)collectionView:(CollectionView *)collectionView
                             layout:(CollectionViewLayout *)collectionViewLayout
    referenceSizeForHeaderInSection:(NSInteger)section {
  return CGSizeMake(collectionView.bounds.size.width - 10, 35);
}

#pragma mark - NSTextViewDelegate

- (void)textDidChange:(NSNotification *)notification {
  [self.sourceView clearErrors];
  [self.lineNumberView clearErrors];
}

#pragma mark - SourceViewDataSource

- (marlin::control::source_selection)sourceView:(SourceView *)view
                                    selectionAt:(marlin::source_loc)loc {
  return {self.document.content, loc};
}

- (marlin::control::statement_inserter)statementInserterForSourceView:(SourceView *)view {
  return {self.document.content};
}

- (marlin::control::expression_inserter)expressionInserterForSourceView:(SourceView *)view {
  return {self.document.content};
}

@end
