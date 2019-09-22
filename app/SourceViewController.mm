#import "SourceViewController.h"

#import "Document.h"
#import "ExecuteViewController.h"
#import "Pasteboard.h"
#import "Theme.h"

@interface SourceViewController () <SourceViewDelegate>

- (Document *)document;

- (SourceView *)sourceView;

@end

@implementation SourceViewController {
  NSString *_executableCode;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.sourceView.delegate = self;
  setCurrentTheme([DefaultTheme new]);
}

- (Document *)document {
  NSAssert(NO, @"Implemented by subclass");
  return nil;
}

- (SourceView *)sourceView {
  NSAssert(NO, @"Implemented by subclass");
  return nil;
}

- (void)prepareForSegue:(StoryboardSegue *)segue sender:(id)sender {
  auto *destinationViewController = [self destinationViewControllerOfSegue:segue];
  if ([destinationViewController isKindOfClass:[ExecuteViewController class]]) {
    assert(_executableCode != nil);

    auto *vc = (ExecuteViewController *)destinationViewController;
    vc.executable = _executableCode;
  }
}

- (void)execute {
  [self.lineNumberView clearErrors];
  _executableCode = nil;
  try {
    _executableCode =
        [NSString stringWithStringView:self.document.content.generate_executable_code()];
  } catch (marlin::exec::collected_generation_error &e) {
    for (auto &err : e.errors()) {
      [self.sourceView addErrorInSourceRange:err.node().source_code_range];
      [self.lineNumberView addError:[NSString stringWithCString:err.what()
                                                       encoding:NSUTF8StringEncoding]
                             atLine:err.node().source_code_range.begin.line];
    }
  }

  if (_executableCode != nil) {
    [self performSegueWithIdentifier:@"ExecuteViewController" sender:self];
  }
}

- (ViewController *)destinationViewControllerOfSegue:(StoryboardSegue *)segue {
  NSAssert(NO, @"Implemented by subclass");
  return nil;
}

#pragma mark - SourceViewDelegate

- (void)sourceViewChanged:(SourceView *)view {
  [self.document updateChangeCount:NSChangeDone];
}

- (void)showDuplicateViewControllerForSourceView:(SourceView *)view {
  [self.toolboxViewController showDuplicateViewControllerForSourceView:view];
}

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  [self.toolboxViewController showEditorViewControllerForSourceView:view withType:type data:data];
}

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                          withFunctionSignature:(marlin::function_definition)signature {
  [self.toolboxViewController showFunctionViewControllerForSourceView:view
                                                withFunctionSignature:signature];
}

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view {
  [self.toolboxViewController dismissEditorViewController];
}

- (void)dismissChildViewControllersForSourceView:(SourceView *)view {
  [self.toolboxViewController dismissChildViewControllers];
}

@end
