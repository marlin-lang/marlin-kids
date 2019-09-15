#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface ParameterItem : NSCollectionViewItem

@property (weak) IBOutlet NSTextField *indexLabel;
@property (weak) IBOutlet NSTextField *nameTextField;

@end

NS_ASSUME_NONNULL_END
