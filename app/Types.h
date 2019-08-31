#ifdef IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#ifdef IOS

#define AppleDocument UIDocument

#define Point CGPoint
#define MakePoint CGPointMake
#define PointInRect CGPointInRect
#define Size CGSize
#define MakeSize CGSizeMake
#define Rect CGRect
#define ZeroRect CGRectZero
#define MakeRect CGRectMake

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
#define ImageView UIImageView
#define TextView UITextView
#define CollectionView UICollectionView
#define CollectionViewItem UICollectionViewCell
#define CollectionViewLayout UICollectionViewLayout
#define CollectionViewDataSource UICollectionViewDataSource
#define CollectionViewDelegateFlowLayout UICollectionViewDelegateFlowLayout
#define ViewController UIViewController

#else

#define AppleDocument NSDocument

#define Point NSPoint
#define MakePoint NSMakePoint
#define PointInRect NSPointInRect
#define Size NSSize
#define MakeSize NSMakeSize
#define Rect NSRect
#define ZeroRect NSZeroRect
#define MakeRect NSMakeRect

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
#define ImageView NSImageView
#define TextView NSTextView
#define CollectionView NSCollectionView
#define CollectionViewItem NSCollectionViewItem
#define CollectionViewLayout NSCollectionViewLayout
#define CollectionViewDataSource NSCollectionViewDataSource
#define CollectionViewDelegateFlowLayout NSCollectionViewDelegateFlowLayout
#define ViewController NSViewController

#endif
