#import "SourceViewController.h"

#import "SourceView.h"

@interface SourceViewController ()

@property(weak, nonatomic) IBOutlet UICollectionView *toolboxView;
@property(weak, nonatomic) IBOutlet SourceView *sourceView;

@end

@implementation SourceViewController

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  // Access the document
  [self.document openWithCompletionHandler:^(BOOL success) {
    if (success) {
    } else {
    }
  }];
}

- (IBAction)dismissDocumentViewController {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             [self.document closeWithCompletionHandler:nil];
                           }];
}

@end
