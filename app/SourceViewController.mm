#import "SourceViewController.h"

#include <optional>

#include "toolbox_model.hpp"

#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "Theme.h"

#ifdef IOS
#else
#endif

@interface SourceViewController ()

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

  self.sourceView.dataSource = self;
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
