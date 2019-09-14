#import "Types.h"

NS_ASSUME_NONNULL_BEGIN

@interface ToolboxViewController : ViewController

@property(weak) IBOutlet CollectionView *toolboxView;

@property(readonly) NSInteger currentSection;

@end

NS_ASSUME_NONNULL_END
