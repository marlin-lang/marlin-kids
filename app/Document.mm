#import "Document.h"

#import "NSData+DataView.h"
#import "NSObject+Casting.h"
#import "SourceViewController.h"

@interface Document () {
  std::optional<marlin::control::document> _content;
}

@end

@implementation Document

- (std::optional<marlin::control::source_update>)initialize {
  if (auto result{self.initialData == nil
                      ? marlin::control::document::make_document()
                      : marlin::control::document::make_document(self.initialData.dataView)}) {
    auto [doc, source]{*std::move(result)};
    self.initialData = nil;
    _content = std::move(doc);
    return source;
  } else {
    return std::nullopt;
  }
}

- (marlin::control::document &)content {
  return *_content;
}

@end
