#import "Types.h"

#include "document.hpp"
#include "toolbox.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ToolboxViewController : ViewController

@property(weak) IBOutlet CollectionView *toolboxView;

- (marlin::control::toolbox &)model;

- (void)registerModelToDocument:(marlin::control::document &)document;

@end

NS_ASSUME_NONNULL_END
