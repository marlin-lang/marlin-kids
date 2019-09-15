#import "Types.h"

#include "toolbox.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ToolboxViewController : ViewController

@property(weak) IBOutlet CollectionView *toolboxView;

- (marlin::control::toolbox &)model;

- (void)addRecentForCurrentCategoryItem:(NSInteger)item;

@end

NS_ASSUME_NONNULL_END
