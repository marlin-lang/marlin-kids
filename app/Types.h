#ifdef IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#include <string_view>

#include "byte_span.hpp"
#include "color_literal.hpp"

#ifdef IOS

#define AppleDocument UIDocument
#define DocumentChangeDone UIDocumentChangeDone

#define Color UIColor
#define Font UIFont
#define Image UIImage
#define BezierPath UIBezierPath
#define EdgeInsets UIEdgeInsets
#define EdgeInsetsMake UIEdgeInsetsMake

#define Button UIButton
#define TextField UITextField
#define TextFieldDelegate UITextFieldDelegate
#define SegmentedControl UISegmentedControl
#define StoryboardSegue UIStoryboardSegue
#define View UIView
#define RulerView UIView
#define ImageView UIImageView
#define StackView UIStackView
#define TextView UITextView
#define CollectionView UICollectionView
#define CollectionViewItem UICollectionViewCell
#define CollectionViewLayout UICollectionViewLayout
#define CollectionViewDataSource UICollectionViewDataSource
#define CollectionViewDelegateFlowLayout UICollectionViewDelegateFlowLayout
#define ViewController UIViewController

#else

#define AppleDocument NSDocument
#define DocumentChangeDone NSChangeDone

#define Color NSColor
#define Font NSFont
#define Image NSImage
#define BezierPath NSBezierPath
#define EdgeInsets NSEdgeInsets
#define EdgeInsetsMake NSEdgeInsetsMake

#define Button NSButton
#define TextField NSTextField
#define TextFieldDelegate NSTextFieldDelegate
#define SegmentedControl NSSegmentedControl
#define StoryboardSegue NSStoryboardSegue
#define View NSView
#define RulerView NSRulerView
#define ImageView NSImageView
#define StackView NSStackView
#define TextView NSTextView
#define CollectionView NSCollectionView
#define CollectionViewItem NSCollectionViewItem
#define CollectionViewLayout NSCollectionViewLayout
#define CollectionViewDataSource NSCollectionViewDataSource
#define CollectionViewDelegateFlowLayout NSCollectionViewDelegateFlowLayout
#define ViewController NSViewController

#endif

#ifdef IOS

#define VIEWWILLAPPEAR_BEGIN               \
  -(void)viewWillAppear : (BOOL)animated { \
    [super viewWillAppear:animated];

#define VIEWDIDAPPEAR_BEGIN               \
  -(void)viewDidAppear : (BOOL)animated { \
    [super viewDidAppear:animated];

#else

#define VIEWWILLAPPEAR_BEGIN \
  -(void)viewWillAppear {    \
    [super viewWillAppear];

#define VIEWDIDAPPEAR_BEGIN \
  -(void)viewDidAppear {    \
    [super viewDidAppear];

#endif

#define VIEWWILLAPPEAR_END }

#define VIEWDIDAPPEAR_END }

@interface NSObject (Casting)

+ (instancetype)cast:(id)object;

@end

@interface NSString (StringView)

+ (instancetype)stringWithStringView:(std::string_view)sv;

- (instancetype)initWithStringView:(std::string_view)sv;

- (std::string_view)stringView;

@end

@interface NSData (DataView)

+ (instancetype)dataWithDataView:(marlin::store::data_view)data;

- (instancetype)initWithDataView:(marlin::store::data_view)data;

- (marlin::store::data_view)dataView;

@end

@interface Color (ColorLiteral)

+ (instancetype)colorWithColorLiteral:(marlin::control::color_literal)colorLiteral;

- (marlin::control::color_literal)colorLiteralWithMode:(marlin::ast::color_mode)mode;

@end

#pragma mark - ios implementation of mac interface

#ifdef IOS

@interface UIColor (MacInterface)

+ (UIColor *)colorWithCalibratedRed:(CGFloat)red
                              green:(CGFloat)green
                               blue:(CGFloat)blue
                              alpha:(CGFloat)alpha;

+ (UIColor *)colorWithCalibratedHue:(CGFloat)hue
                         saturation:(CGFloat)saturation
                         brightness:(CGFloat)brightness
                              alpha:(CGFloat)alpha;

@end

@interface UIStoryboard (MacInterface)

- (UIViewController *)instantiateControllerWithIdentifier:(NSString *)identifier;

@end

@interface UIView (MacInterface)

- (void)setFrameSize:(CGSize)size;

@end

@interface UISegmentedControl (MacInterface)

- (NSUInteger)selectedSegment;

@end

@interface UITextField (MacInterface)

- (NSString *)stringValue;

- (void)setStringValue:(NSString *)stringValue;

@end

@interface UITextView (MacInterface)

- (NSString *)string;

@end

#else

@interface NSViewController (IosInterface)

- (void)willMoveToParentViewController:(NSViewController*)vc;

- (void)didMoveToParentViewController:(NSViewController*)vc;

@end

#endif
