#import "ExecuteViewController.h"

#import "chrono"

#import "DrawContext.h"
#import "NSString+StringView.h"

void dispatch_on_main(void (^block)(void)) {
  if (NSThread.isMainThread) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

constexpr double refreshTimeInMS = 40;

@interface ExecuteViewController ()

@property(weak) IBOutlet NSImageView *imageView;

@property(weak) IBOutlet NSTextField *outputTextField;

@end

@implementation ExecuteViewController {
  std::optional<marlin::control::exec_environment> _environment;
  DrawContext _drawContext;

  bool _needRefreshImage;
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
  _needRefreshImage = NO;
}

- (void)viewDidAppear {
  [super viewDidAppear];

  [self startExecute];
}

- (void)viewWillDisappear {
  [self stopExecute];
}

- (void)setEnvironment:(marlin::control::exec_environment)environment {
  _environment = std::move(environment);
}

#pragma mark - DrawContextDelegate implementation

- (void)setNeedRefreshImage {
  auto time = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time - _refresh_time);
  if (diff.count() < refreshTimeInMS) {
    _needRefreshImage = YES;
  } else {
    [self refreshImage];
  }
}

#pragma mark - Private methods

- (void)startExecute {
  assert(_environment.has_value());

  __weak auto weakSelf = self;
  _environment->register_print_callback([weakSelf](std::string value) {
    dispatch_on_main(^{
      weakSelf.outputTextField.stringValue = [weakSelf.outputTextField.stringValue
          stringByAppendingString:[NSString stringWithStringView:value]];
    });
  });
  _environment->add_custom_callback(
      "draw_line", [weakSelf](auto ctx, auto, auto args, auto exception) {
        if (args.size() != 4) {
          *exception = ctx.error("Incorrect number of arguments!");
        } else {
          double start_x = args[0].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double start_y = args[1].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double end_x = args[2].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          double end_y = args[3].to_number();
          if (!ctx.ok()) {
            *exception = ctx.get_exception();
            return;
          }

          auto start = NSMakePoint(static_cast<CGFloat>(start_x), static_cast<CGFloat>(start_y));
          auto end = NSMakePoint(static_cast<CGFloat>(end_x), static_cast<CGFloat>(end_y));
          dispatch_on_main(^{
            if (auto strongSelf = weakSelf) {
              strongSelf->_drawContext.drawLine(start, end);
            }
          });
        }
      });

  _environment->execute();
}

- (void)stopExecute {
  assert(_environment.has_value());

  _environment->terminate();
}

- (void)refreshImage {
  for (NSImageRep *rep in self.imageView.image.representations) {
    [self.imageView.image removeRepresentation:rep];
  }
  [self.imageView.image addRepresentation:_drawContext.imageRep];
  [self.imageView setNeedsDisplay:YES];
  _refresh_time = std::chrono::high_resolution_clock::now();
  _needRefreshImage = NO;

  __weak auto weakSelf = self;
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(refreshTimeInMS / 1000 * NSEC_PER_SEC)),
                 dispatch_get_main_queue(), ^{
                   if (auto strongSelf = weakSelf) {
                     if (strongSelf->_needRefreshImage) {
                       [strongSelf refreshImage];
                     }
                   }
                 });
}

@end
