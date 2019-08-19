#import <Cocoa/Cocoa.h>

#import "chrono"

@protocol DrawContextDelegate <NSObject>

- (void)setNeedRefreshImage;

@end

struct DrawContext {
  NSBitmapImageRep* imageRep;

  void initWithImage(NSImage* image, id<DrawContextDelegate> delegate);

  void draw_line(double from_x, double from_y, double to_x, double to_y);

 private:
  __weak id<DrawContextDelegate> _delegate;
};
