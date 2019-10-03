#import <Types.h>

#include "function_definition.hpp"

NS_ASSUME_NONNULL_BEGIN

@class FunctionViewController;

@protocol FunctionViewControllerDelegate

- (void)functionViewController:(FunctionViewController *)vc
    finishEditingWithName:(NSString *)name
               parameters:(NSArray<NSString *> *)parameters;

@end

@interface FunctionViewController : ViewController

@property(weak) IBOutlet TextField *nameTextField;
@property(weak) IBOutlet CollectionView *parametersCollectionView;

@property(weak) id<FunctionViewControllerDelegate> delegate;

@property(strong) NSMutableArray<NSString *> *parameters;

- (void)finishEditing;

- (void)setFunctionSignature:(marlin::function_definition)signature;

- (void)validate;

@end

NS_ASSUME_NONNULL_END
