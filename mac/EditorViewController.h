#import <Cocoa/Cocoa.h>

#import "expression_inserter.hpp"

@protocol EditorViewControllerDelegate;

using EditorType = marlin::control::literal_data_type;

@interface EditorViewController : NSViewController <NSTextFieldDelegate>

@property(nonatomic) EditorType type;

@property(weak) IBOutlet NSTextField *editorTextField;

@property(weak) id<EditorViewControllerDelegate> delegate;

@end

@protocol EditorViewControllerDelegate <NSObject>

- (void)viewController:(EditorViewController *)vc finishEditWithString:(NSString *)string;

@end
