#import <Cocoa/Cocoa.h>

@protocol EditorViewControllerDelegate;

enum class EditorType { Number, String, Variable };

@interface EditorViewController : NSViewController <NSTextFieldDelegate>

@property(nonatomic) EditorType type;

@property(weak) id<EditorViewControllerDelegate> delegate;

@end

@protocol EditorViewControllerDelegate <NSObject>

- (void)viewController:(EditorViewController*)vc finishEditWithString:(NSString*)string;

@end
