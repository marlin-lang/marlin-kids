#import <Types.h>

#import "literal_content.hpp"

NS_ASSUME_NONNULL_BEGIN

@class EditorViewController;

using EditorType = marlin::control::literal_data_type;

@protocol EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController *)vc
        finishEditWithString:(NSString *)string
                      ofType:(EditorType)type;

@end

@interface EditorViewController : ViewController

@property(nonatomic) EditorType type;

@property(weak) IBOutlet SegmentedControl *typeSegmentControl;
@property(weak) IBOutlet TextField *editorTextField;

@property(weak) id<EditorViewControllerDelegate> delegate;

@property(getter=isValid) BOOL valid;

- (void)setEditorString:(NSString *)string;

- (void)validate;

@end

NS_ASSUME_NONNULL_END
