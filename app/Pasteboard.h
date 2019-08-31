#import <Foundation/Foundation.h>

#include "toolbox_model.hpp"

inline NSString* pasteboardOfType(marlin::control::pasteboard_t type) {
  switch (type) {
    case marlin::control::pasteboard_t::statement:
      return @"marlin.statement";
    case marlin::control::pasteboard_t::expression:
      return @"marlin.expression";
  }
}
