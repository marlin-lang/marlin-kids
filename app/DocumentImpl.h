#import <Foundation/Foundation.h>

#include <optional>

#include "document.hpp"

#import "NSData+DataView.h"

struct DocumentImpl {
  DocumentImpl() : initialData{nil} {}

  inline std::optional<marlin::control::source_update> initialize() {
    if (auto result{initialData == nil
                        ? marlin::control::document::make_document()
                        : marlin::control::document::make_document(initialData.dataView)}) {
      auto [doc, source]{*std::move(result)};
      initialData = nil;
      content = std::move(doc);
      return source;
    } else {
      return std::nullopt;
    }
  }

  std::optional<marlin::control::document> content;
  NSData *initialData;
};
