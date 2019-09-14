#import "DocumentBrowserViewController.h"

#include "document.hpp"

#import "IosDocument.h"
#import "IosSourceViewController.h"
#import "NSData+DataView.h"
#import "NSObject+Casting.h"

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
  auto filePath = [[NSTemporaryDirectory() stringByAppendingPathComponent:@"Untitled"]
      stringByAppendingPathExtension:@"mar"];
  auto url = [NSURL fileURLWithPath:filePath];
  auto data = [NSData dataWithDataView:marlin::control::document::default_data()];
  [data writeToURL:url atomically:YES];
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
  UISplitViewController* splitViewController =
      [storyBoard instantiateViewControllerWithIdentifier:@"SplitViewController"];
    auto vc = [IosSourceViewController cast:splitViewController.viewControllers[1]];
  vc.document = [[IosDocument alloc] initWithFileURL:documentURL];
  [self presentViewController:splitViewController animated:YES completion:nil];
}

@end
