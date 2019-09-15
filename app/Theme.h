#import <Types.h>

#import "formatter.hpp"

@protocol Theme <NSObject>

@property(readonly) NSDictionary<NSAttributedStringKey, id>* allAttrs;

@property(readonly) NSDictionary<NSAttributedStringKey, id>* lineNumberAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* consoleAttrs;

@property(readonly) NSDictionary<NSAttributedStringKey, id>* keywordAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* opAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* booleanAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* numberAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* stringAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* placeholderAttrs;

@end

@interface DefaultTheme : NSObject <Theme>

@property(readonly) NSDictionary<NSAttributedStringKey, id>* allAttrs;

@property(readonly) NSDictionary<NSAttributedStringKey, id>* lineNumberAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* consoleAttrs;

@property(readonly) NSDictionary<NSAttributedStringKey, id>* keywordAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* opAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* booleanAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* numberAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* stringAttrs;
@property(readonly) NSDictionary<NSAttributedStringKey, id>* placeholderAttrs;

@end

id<Theme> currentTheme();

void setCurrentTheme(id<Theme> theme);

CGSize characterSizeWithAttributes(NSDictionary<NSAttributedStringKey, id>* attrs);

void applyTheme(id<Theme> theme, NSMutableAttributedString* attributedString, NSRange range,
                const std::vector<marlin::format::highlight_token>& highlights);
