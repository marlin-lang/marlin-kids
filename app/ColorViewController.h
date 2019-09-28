#import <Types.h>

NS_ASSUME_NONNULL_BEGIN

@class ColorViewController;

@protocol ColorViewControllerDelegate

- (void)colorViewController:(ColorViewController *)vc finishEditingWithColor:(Color *)color;

@end

@interface ColorViewController : ViewController

@property(weak) id<ColorViewControllerDelegate> delegate;

- (void)setColor:(Color *)color showAlpha:(BOOL)showAlpha;

@end

NS_ASSUME_NONNULL_END
