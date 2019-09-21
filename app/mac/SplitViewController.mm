#import "SplitViewController.h"

#import "MacToolboxViewController.h"
#import "NSObject+Casting.h"

@implementation SplitViewController

- (MacToolboxViewController *)toolboxViewController {
  auto vc = [MacToolboxViewController cast:[self.splitViewItems objectAtIndex:0].viewController];
  NSAssert(vc, @"");
  return vc;
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification {
  [self.toolboxViewController.toolboxView reloadData];
}

@end
