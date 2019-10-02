#import "MacSourceViewController.h"

#import "MacLineNumberView.h"
#import "MacFunctionViewController.h"

@interface MacSourceViewController ()

@property(weak) IBOutlet SourceView *sourceView;

@end

@implementation MacSourceViewController

- (void)setDocument:(MacDocument *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceView initializeWithDisplay:std::move(initialData->display)];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];
  self.sourceView.dataSource = self;
  self.sourceView.enclosingScrollView.rulersVisible = YES;
  self.sourceView.enclosingScrollView.hasHorizontalRuler = NO;
  self.sourceView.enclosingScrollView.hasVerticalRuler = YES;
  auto lineNumberView = [[MacLineNumberView alloc] initWithSourceView:self.sourceView];
  self.lineNumberView = lineNumberView;
  self.sourceView.enclosingScrollView.verticalRulerView = lineNumberView;
}

- (NSViewController *)destinationViewControllerOfSegue:(NSStoryboardSegue *)segue {
  return segue.destinationController;
}

@end
