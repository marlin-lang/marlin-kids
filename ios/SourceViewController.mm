#import "SourceViewController.h"

@interface SourceViewController ()

@property IBOutlet UILabel *documentNameLabel;

@end

@implementation SourceViewController

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  // Access the document
  [self.document openWithCompletionHandler:^(BOOL success) {
    if (success) {
      // Display the content of the document, e.g.:
      self.documentNameLabel.text = self.document.fileURL.lastPathComponent;
    } else {
      // Make sure to handle the failed import appropriately, e.g., by presenting an error message
      // to the user.
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
