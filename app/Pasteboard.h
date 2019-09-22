#import <Foundation/Foundation.h>

#include "prototypes.hpp"

struct DraggingData {
  marlin::control::pasteboard_t type;
  NSData *data;

  DraggingData() : type{marlin::control::pasteboard_t::block}, data{nil} {}
  DraggingData(marlin::control::pasteboard_t _type, NSData *_data) : type{_type}, data{_data} {}
};

inline NSString *pasteboardOfType(marlin::control::pasteboard_t type) {
  switch (type) {
    case marlin::control::pasteboard_t::block:
      return @"marlin.block";
    case marlin::control::pasteboard_t::statement:
      return @"marlin.statement";
    case marlin::control::pasteboard_t::expression:
      return @"marlin.expression";
    case marlin::control::pasteboard_t::reference:
      return @"marlin.reference";
  }
}
