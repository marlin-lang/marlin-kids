#import "IosSourceViewController.h"

#include "toolbox_model.hpp"

#import "LineNumberView.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "IosSourceView.h"

@interface IosSourceViewController () <
                                       UIScrollViewDelegate,
                                       SourceViewDelegate>

@property(strong, nonatomic) IosSourceView *sourceView;

@property(weak, nonatomic) IBOutlet UIScrollView *scrollView;

@end

@implementation IosSourceViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.scrollView.delegate = self;
  self.sourceView = [[IosSourceView alloc] initWithEnclosingScrollView:self.scrollView
                                                            dataSource:self];
  self.sourceView.delegate = self;
  [self.scrollView addSubview:self.sourceView];

  auto *lineNumberView = [[LineNumberView alloc] initWithSourceView:self.sourceView];
  self.lineNumberView = lineNumberView;
  self.sourceView.lineNumberView = lineNumberView;
  lineNumberView.translatesAutoresizingMaskIntoConstraints = NO;
  self.scrollView.contentInset = UIEdgeInsetsMake(0, lineNumberView.ruleThickness, 0, 0);
  [self.view addSubview:lineNumberView];
  [lineNumberView.topAnchor constraintEqualToAnchor:self.scrollView.topAnchor].active = YES;
  [lineNumberView.leftAnchor constraintEqualToAnchor:self.scrollView.leftAnchor].active = YES;
  [lineNumberView.heightAnchor constraintEqualToAnchor:self.view.heightAnchor].active = YES;
  [lineNumberView.widthAnchor constraintEqualToConstant:lineNumberView.ruleThickness].active = YES;

  [self.document openWithCompletionHandler:^(BOOL success) {
    if (success) {
      if (auto initialData = [self.document initialize]) {
        [self.sourceView insertStatementsBeforeLine:1
                                         withSource:std::move(initialData->source)
                                         highlights:std::move(initialData->highlights)
                                       isInitialize:true];
        [self.lineNumberView setNeedsDisplay];
      }
    } else {
    }
  }];
}

- (IBAction)close:(id)sender {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             [self.document closeWithCompletionHandler:nil];
                           }];
}

- (IBAction)run:(id)sender {
  [self execute];
}

- (UIViewController *)destinationViewControllerOfSegue:(UIStoryboardSegue *)segue {
  return segue.destinationViewController;
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(CollectionView *)collectionView {
  return marlin::control::toolbox_model::sections.size();
}

- (NSInteger)collectionView:(CollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return marlin::control::toolbox_model::items[section].size();
}

#pragma mark - UIScrollViewDelegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.lineNumberView setNeedsDisplay];
}

#pragma mark - SourceViewDelegate

- (void)sourceViewChanged:(SourceView *)view {
  [self.document updateChangeCount:UIDocumentChangeDone];
}

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(CGRect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  EditorViewController *vc =
      [self.storyboard instantiateViewControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;
  vc.modalPresentationStyle = UIModalPresentationPopover;
  vc.popoverPresentationController.permittedArrowDirections = UIMenuControllerArrowUp;
  vc.popoverPresentationController.sourceView = view;
  vc.popoverPresentationController.sourceRect = rect;
  [self presentViewController:vc animated:YES completion:nil];
  vc.type = type;
  vc.editorTextField.text = [NSString stringWithStringView:data];
}

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view {
  [self dismissViewControllerAnimated:YES completion:nil];
}

@end
