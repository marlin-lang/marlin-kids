#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface WindowController : NSWindowController

@property(strong, nonatomic) IBInspectable NSString *frameAutosaveIdentifier;

@end

@interface MainWindowController : WindowController<NSToolbarDelegate>

@end

@interface ExecuteWindowController : WindowController<NSToolbarDelegate>

@end

NS_ASSUME_NONNULL_END
