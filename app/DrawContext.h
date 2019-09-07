#import "Types.h"

#import "chrono"

@protocol DrawContextDelegate <NSObject>

- (void)setNeedRefreshImage;

@end

struct DrawContext {
  void initWithImage(Image* image, id<DrawContextDelegate> delegate);

#ifndef IOS
  NSBitmapImageRep* imageRep() const { return _imageRep; }
#endif
  double width() const { return _size.width; }
  double height() const { return _size.height; }

  void draw_line(double from_x, double from_y, double to_x, double to_y);

 private:
#ifndef IOS
  NSBitmapImageRep* _imageRep;
#endif
  Size _size;
  __weak id<DrawContextDelegate> _delegate;
};
