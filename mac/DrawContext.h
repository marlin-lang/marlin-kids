#import <Cocoa/Cocoa.h>

#import "chrono"

@protocol DrawContextDelegate <NSObject>

- (void)applyImageRep:(NSBitmapImageRep*)imageRep;

@end

struct DrawContext {
  void initWithImage(NSImage* image, id<DrawContextDelegate> delegate);

  void drawLine(NSPoint from, NSPoint to);

 private:
  __weak id<DrawContextDelegate> _delegate;
  NSBitmapImageRep* _imageRep;
};