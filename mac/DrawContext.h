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
  __weak id<DrawContextDelegate> _delegate;
};
