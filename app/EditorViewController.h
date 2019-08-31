#import <Types.h>

#import "expression_inserter.hpp"

@protocol EditorViewControllerDelegate;

using EditorType = marlin::control::literal_data_type;

@interface EditorViewController : ViewController <TextFieldDelegate>

@property(nonatomic) EditorType type;

@property(weak) IBOutlet TextField *editorTextField;

@property(weak) id<EditorViewControllerDelegate> delegate;

@end

@protocol EditorViewControllerDelegate <NSObject>

- (void)viewController:(EditorViewController *)vc
    finishEditWithString:(NSString *)string
                  ofType:(EditorType)type;

@end
