#import <Cocoa/Cocoa.h>

#import "Document.h"

NS_ASSUME_NONNULL_BEGIN

@class ExecuteViewController;

@protocol ExecuteViewControllerDelegate <NSObject>

- (void)addErrorAt:(const marlin::ast::base &)node message:(const std::string &)message;

@end

@interface ExecuteViewController : NSViewController

@property(nonatomic, weak) id<ExecuteViewControllerDelegate> delegate;

@property(nonatomic, weak) Document* document;

@end

NS_ASSUME_NONNULL_END
