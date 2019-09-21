#import "MacDocument.h"

#import "MacSourceViewController.h"
#import "ToolboxViewController.h"

@implementation MacDocument

+ (BOOL)autosavesInPlace {
  return YES;
}

- (void)makeWindowControllers {
  NSWindowController *controller = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]
      instantiateControllerWithIdentifier:@"WindowController"];
  [self addWindowController:controller];
  if (auto splitViewController = [NSSplitViewController cast:controller.contentViewController]) {
    if (auto *vc =
            [MacSourceViewController cast:splitViewController.splitViewItems[1].viewController]) {
      vc.document = self;
    }
    if (auto *toolbox =
            [ToolboxViewController cast:splitViewController.splitViewItems[0].viewController]) {
      [toolbox registerModelToDocument:self.content];
    }
  }
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
  return [NSData dataWithDataView:self.content.write()];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
  self.initialData = data;
  return YES;
}

@end
