#import "WindowController.h"

#import "SourceViewController.h"
#import "NSObject+Casting.h"

@interface WindowController ()

@end

@implementation WindowController

- (void)windowDidLoad {
  [super windowDidLoad];

  auto *toolbar = [NSToolbar new];
  toolbar.delegate = self;
  self.window.toolbar = toolbar;
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
  return @[ @"run" ];
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
  return [self toolbarAllowedItemIdentifiers:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
        itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier
    willBeInsertedIntoToolbar:(BOOL)flag {
  auto *toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
  toolbarItem.label = @"Run";
  toolbarItem.target = self;
  toolbarItem.action = @selector(run:);
  return toolbarItem;
}

- (void)run:(NSToolbarItem *)sender {
    if (auto splitViewController = [NSSplitViewController cast:self.contentViewController]) {
        if (auto *vc = [SourceViewController cast:splitViewController.splitViewItems[1].viewController]) {
  [vc execute];
        }
    }
}

@end
