#import "MacLineNumberView.h"

#import "MessageViewController.h"

@implementation MacLineNumberView

- (void)mouseDown:(NSEvent*)event {
  [super mouseDown:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  auto lineNumber = [self lineNumberOfLocation:location];
  if (NSString* message = [self errorMessageOfLine:lineNumber]) {
    auto* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    MessageViewController* vc =
        [storyboard instantiateControllerWithIdentifier:@"MessageViewController"];
    auto popover = [NSPopover new];
    popover.behavior = NSPopoverBehaviorTransient;
    popover.contentViewController = vc;
    auto* textView = (SourceView*)self.clientView;
    auto offset = [self convertPoint:NSZeroPoint fromView:textView];
    auto y = [textView lineTopOfNumber:lineNumber] + offset.y;
    auto errorRect = NSMakeRect(0, y, textView.lineHeight, textView.lineHeight);
    [popover showRelativeToRect:errorRect ofView:self preferredEdge:NSMaxXEdge];
    vc.messageTextField.stringValue = message;
  }
}

@end
