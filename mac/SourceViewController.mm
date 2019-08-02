#import "SourceViewController.h"

#import "Document.h"
#import "SourceTheme.h"

@interface SourceViewController ()

@property(nonatomic, weak) IBOutlet SourceTextView *sourceTextView;
@property(nonatomic, weak) IBOutlet NSTextField *outputTextField;

@end

@implementation SourceViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.sourceTextView.dataSource = self;
}

- (void)setNeedsUpdate {
  [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(update) object:nil];
  [self performSelector:@selector(update) withObject:nil afterDelay:0];
}

- (void)update {
  auto &doc = self.document.content;
  auto &source_str = doc.source_str();
  self.sourceTextView.string = [NSString stringWithCString:source_str.c_str()
                                                  encoding:NSUTF8StringEncoding];
  auto *theme = [SourceTheme new];
  [self.sourceTextView.textStorage setAttributes:theme.allAttrs
                                           range:NSMakeRange(0, source_str.size())];
}

- (IBAction)execute:(id)sender {
  auto &doc = self.document.content;
  doc.execute();
  self.outputTextField.stringValue = [NSString stringWithCString:doc.output().c_str()
                                                        encoding:NSUTF8StringEncoding];
}

- (NSRange)textView:(SourceTextView *)textView selectRageContainsIndex:(NSUInteger)index {
  return NSMakeRange(0, 0);
}

@end
