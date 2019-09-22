#import "Types.h"

#include "document.hpp"
#include "toolbox.hpp"

#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface ToolboxViewController : ViewController

@property(weak) IBOutlet CollectionView *toolboxView;

- (marlin::control::toolbox &)model;

- (void)registerModelToDocument:(marlin::control::document &)document;

- (void)showDuplicateViewControllerForSourceView:(SourceView *)view
                                withDraggingData:(const DraggingData &)draggingData;

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data;

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                          withFunctionSignature:(marlin::function_definition)signature;

- (void)dismissEditorViewController;

- (void)dismissChildViewControllers;

@end

NS_ASSUME_NONNULL_END
