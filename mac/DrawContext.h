#import <Cocoa/Cocoa.h>

#import "chrono"

@protocol DrawContextDelegate <NSObject>

- (void)refreshImage;

@end

struct DrawContext {
  NSBitmapImageRep* imageRep;

  void initWithImage(NSImage* image, id<DrawContextDelegate> delegate);

  void drawLine(NSPoint from, NSPoint to);

 private:
  template <typename Block>
  void execute(Block&& block) {
    if (NSThread.isMainThread) {
      block();
      [_delegate refreshImage];
    } else {
      dispatch_sync(dispatch_get_main_queue(), ^{
        block();
        [_delegate refreshImage];
      });
    }
  }

  __weak id<DrawContextDelegate> _delegate;
};
