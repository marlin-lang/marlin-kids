#import <Foundation/Foundation.h>

#include "prototypes.hpp"

inline NSString* pasteboardOfType(marlin::control::pasteboard_t type) {
  switch (type) {
    case marlin::control::pasteboard_t::block:
      return @"marlin.block";
    case marlin::control::pasteboard_t::statement:
      return @"marlin.statement";
    case marlin::control::pasteboard_t::expression:
      return @"marlin.expression";
  }
}
