#import "SplitViewController.h"

#import "MacSourceViewController.h"
#import "MacToolboxViewController.h"

@implementation SplitViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.sourceViewController.toolboxViewController = self.toolboxViewController;
}

- (MacToolboxViewController *)toolboxViewController {
  auto vc = [MacToolboxViewController cast:[self.splitViewItems objectAtIndex:0].viewController];
  NSAssert(vc, @"");
  return vc;
}

- (MacSourceViewController *)sourceViewController {
  auto vc = [MacSourceViewController cast:[self.splitViewItems objectAtIndex:1].viewController];
  NSAssert(vc, @"");
  return vc;
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification {
  [self.toolboxViewController.toolboxView reloadData];
}

@end
