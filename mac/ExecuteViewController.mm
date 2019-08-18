#import "ExecuteViewController.h"

#import "chrono"

#import "DrawContext.h"
#import "NSString+StringView.h"

@interface ExecuteViewController ()

@property(weak) IBOutlet NSImageView *imageView;

@property(weak) IBOutlet NSTextField *outputTextField;

@end

@implementation ExecuteViewController {
  DrawContext _drawContext;
  std::chrono::high_resolution_clock::time_point _refresh_time;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.imageView.image = [NSImage imageWithSize:self.imageView.bounds.size
                                        flipped:NO
                                 drawingHandler:^BOOL(NSRect dstRect) {
                                   return YES;
                                 }];
  _drawContext.initWithImage(self.imageView.image, self);
}

- (void)viewDidAppear {
  [super viewDidAppear];

  [self startExecute];
}

- (void)viewWillDisappear {
  [self stopExecute];
}

#pragma mark - DrawContextDelegate implementation

- (void)applyImageRep:(NSBitmapImageRep *)imageRep {
  auto time = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _refresh_time);
  if (diff.count() > 40) {
    for (NSImageRep *rep in self.imageView.image.representations) {
      [self.imageView.image removeRepresentation:rep];
    }
    [self.imageView.image addRepresentation:imageRep];
    [self.imageView setNeedsDisplay:YES];
  }
}

#pragma mark - Private methods

- (void)startExecute {
  assert(self.environment.has_value());

  self.environment->execute([self](std::string value) {
    dispatch_sync(dispatch_get_main_queue(), ^{
      self.outputTextField.stringValue = [self.outputTextField.stringValue
          stringByAppendingString:[NSString stringWithStringView:value]];
    });
  });
}

- (void)stopExecute {
  if (self.environment.has_value()) {
    self.environment->terminate();
  }
}

@end
