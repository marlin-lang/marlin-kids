#import <Cocoa/Cocoa.h>

@protocol EditorViewControllerDelegate;

@interface EditorViewController : NSViewController

@property(weak) id<EditorViewControllerDelegate> delegate;

@end

@protocol EditorViewControllerDelegate<NSObject>

- (void)viewController:(EditorViewController*)vc finishEditWithString:(NSString*)string;

@end
