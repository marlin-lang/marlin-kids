#import "DocumentBrowserViewController.h"
#import "Document.h"
#import "SourceViewController.h"

@interface DocumentBrowserViewController () <UIDocumentBrowserViewControllerDelegate>

@end

@implementation DocumentBrowserViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.delegate = self;
  self.allowsDocumentCreation = YES;
  self.allowsPickingMultipleItems = NO;
}

#pragma mark - UIDocumentBrowserViewControllerDelegate

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller
    didRequestDocumentCreationWithHandler:
        (void (^)(NSURL *_Nullable, UIDocumentBrowserImportMode))importHandler {
  NSURL *url = [NSURL
      fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"Untitled.mar"]];
  [@"" writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:nil];
  importHandler(url, UIDocumentBrowserImportModeMove);
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller
    didPickDocumentsAtURLs:(NSArray<NSURL *> *)documentURLs {
  NSURL *sourceURL = documentURLs.firstObject;
  if (!sourceURL) {
    return;
  }
  [self presentDocumentAtURL:sourceURL];
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller
    didImportDocumentAtURL:(NSURL *)sourceURL
          toDestinationURL:(NSURL *)destinationURL {
  [self presentDocumentAtURL:destinationURL];
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller
    failedToImportDocumentAtURL:(NSURL *)documentURL
                          error:(NSError *_Nullable)error {
}

- (void)presentDocumentAtURL:(NSURL *)documentURL {
  auto *storyBoard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
  SourceViewController *vc =
      [storyBoard instantiateViewControllerWithIdentifier:@"SourceViewController"];
  vc.document = [[Document alloc] initWithFileURL:documentURL];
  auto *navigationController = [[UINavigationController alloc] initWithRootViewController:vc];
  [self presentViewController:navigationController animated:YES completion:nil];
}

@end
