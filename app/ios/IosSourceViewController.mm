#import "IosSourceViewController.h"

#import "IosSourceView.h"
#import "LineNumberView.h"
#import "Pasteboard.h"

@interface IosSourceViewController () <UIScrollViewDelegate>

@property(strong, nonatomic) IosSourceView *sourceView;

@property(weak, nonatomic) IBOutlet UIScrollView *scrollView;

@end

@implementation IosSourceViewController

- (void)setDocument:(IosDocument *)document {
  _document = document;

  if (auto initialData = [self.document initialize]) {
    [self.sourceView initializeWithDisplay:std::move(initialData->display)];
    [self.lineNumberView setNeedsDisplay];
  }
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.scrollView.delegate = self;
  self.sourceView = [[IosSourceView alloc] initWithEnclosingScrollView:self.scrollView
                                                            dataSource:self];
  [self.scrollView addSubview:self.sourceView];

  auto *lineNumberView = [[LineNumberView alloc] initWithSourceView:self.sourceView];
  self.lineNumberView = lineNumberView;
  self.sourceView.lineNumberView = lineNumberView;
  lineNumberView.translatesAutoresizingMaskIntoConstraints = NO;
  self.scrollView.contentInset = UIEdgeInsetsMake(0, lineNumberView.ruleThickness, 0, 0);
  [self.view addSubview:lineNumberView];
  [lineNumberView.topAnchor constraintEqualToAnchor:self.view.topAnchor].active = YES;
  [lineNumberView.leftAnchor constraintEqualToAnchor:self.view.leftAnchor].active = YES;
  [lineNumberView.heightAnchor constraintEqualToAnchor:self.view.heightAnchor].active = YES;
  [lineNumberView.widthAnchor constraintEqualToConstant:lineNumberView.ruleThickness].active = YES;
}

- (IBAction)close:(id)sender {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             [self.document closeWithCompletionHandler:nil];
                           }];
}

- (UIViewController *)destinationViewControllerOfSegue:(UIStoryboardSegue *)segue {
  return segue.destinationViewController;
}

#pragma mark - UIScrollViewDelegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.lineNumberView setNeedsDisplay];
}

@end
