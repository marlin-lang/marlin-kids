#import "Types.h"

#include "toolbox_model.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ToolboxViewController : ViewController

@property(weak) IBOutlet CollectionView *toolboxView;

@property(readonly) NSInteger currentCategory;

- (NSInteger)sizeOfCurrentCategory;
- (const marlin::control::base_prototype &)prototypeOfCurrentCategoryItem:(NSInteger)item;
- (NSString *)pasteboardTypeOfCurrentCategoryItem:(NSInteger)item;

- (void)addRecentForCurrentCategoryItem:(NSInteger)item;

@end

NS_ASSUME_NONNULL_END
