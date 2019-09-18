#import "MacSourceViewController.h"

#import "LineNumberView.h"
#import "MacFunctionViewController.h"
#import "NSObject+Casting.h"
#import "NSString+StringView.h"

@interface MacSourceViewController () <SourceViewDelegate>

@property(weak) IBOutlet SourceView *sourceView;

@end

@implementation MacSourceViewController {
  NSPopover *_popover;
}

- (void)setDocument:(MacDocument *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceView initializeWithDisplay:std::move(initialData->display)];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];
  self.sourceView.dataSource = self;
  self.sourceView.delegate = self;
  self.sourceView.enclosingScrollView.rulersVisible = YES;
  self.sourceView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceView.enclosingScrollView.hasVerticalRuler = YES;
  auto lineNumberView = [[LineNumberView alloc] initWithSourceView:self.sourceView];
  self.lineNumberView = lineNumberView;
  self.sourceView.enclosingScrollView.verticalRulerView = lineNumberView;
}

- (NSViewController *)destinationViewControllerOfSegue:(NSStoryboardSegue *)segue {
  return segue.destinationController;
}

#pragma mark - SourceViewDelegate

- (void)sourceViewChanged:(SourceView *)view {
  [self.document updateChangeCount:NSChangeDone];
}

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(CGRect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  EditorViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;

  _popover = [[NSPopover alloc] init];
  _popover.behavior = NSPopoverBehaviorTransient;
  _popover.contentViewController = vc;
  [_popover showRelativeToRect:rect ofView:view preferredEdge:NSMinYEdge];

  vc.type = type;
  vc.editorTextField.stringValue = [NSString stringWithStringView:data];
}

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                                       fromRect:(CGRect)rect
                          withFunctionSignature:(marlin::function_definition)signature {
  MacFunctionViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"FunctionViewController"];
  vc.delegate = view;
  _popover = [[NSPopover alloc] init];
  _popover.behavior = NSPopoverBehaviorTransient;
  _popover.contentViewController = vc;
  [_popover showRelativeToRect:rect ofView:view preferredEdge:NSMinYEdge];

  [vc setFunctionSignature:std::move(signature)];
}

- (void)dismissPopoverViewControllerForSourceView:(SourceView *)view {
  [_popover close];
  _popover = nil;
}

@end
