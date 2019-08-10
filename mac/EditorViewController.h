#import <Cocoa/Cocoa.h>

@protocol EditorViewControllerDelegate;

enum class EditorType { Variable, Number, String, Identifier };

@interface EditorViewController : NSViewController <NSTextFieldDelegate>

@property(nonatomic) EditorType type;

@property(weak) IBOutlet NSTextField *editorTextField;

@property(weak) id<EditorViewControllerDelegate> delegate;

@end

@protocol EditorViewControllerDelegate <NSObject>

- (void)viewController:(EditorViewController *)vc finishEditWithString:(NSString *)string;

@end
