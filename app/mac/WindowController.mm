#import "WindowController.h"

#import "SourceViewController.h"

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
  auto *vc = (SourceViewController *)self.contentViewController;
  [vc execute];
}

@end
