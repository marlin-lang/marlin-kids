#import <Cocoa/Cocoa.h>

@protocol EditorViewControllerDelegate;

@interface EditorViewController : NSViewController<NSTextFieldDelegate>

@property(weak) id<EditorViewControllerDelegate> delegate;

- (void)setForNumber;
- (void)setForString;
- (void)setForVariable;

@end

@protocol EditorViewControllerDelegate<NSObject>

- (void)viewController:(EditorViewController*)vc finishEditWithString:(NSString*)string;

@end
