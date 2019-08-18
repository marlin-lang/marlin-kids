#import <Cocoa/Cocoa.h>

#import "chrono"

@protocol DrawContextDelegate <NSObject>

- (void)setNeedRefreshImage;

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
      [_delegate setNeedRefreshImage];
    } else {
      dispatch_sync(dispatch_get_main_queue(), ^{
        block();
        [_delegate setNeedRefreshImage];
      });
    }
  }

  __weak id<DrawContextDelegate> _delegate;
};
