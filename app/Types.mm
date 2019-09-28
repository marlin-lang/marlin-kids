#import "Types.h"

@implementation NSObject (Casting)

+ (instancetype)cast:(id)object {
  return [object isKindOfClass:self] ? object : nil;
}

@end

@implementation NSString (StringView)

+ (instancetype)stringWithStringView:(std::string_view)sv {
  return [[NSString alloc] initWithStringView:sv];
}

- (instancetype)initWithStringView:(std::string_view)sv {
  return [self initWithBytes:sv.data() length:sv.size() encoding:NSUTF8StringEncoding];
}

- (std::string_view)stringView {
  return {self.UTF8String, self.length};
}

@end

@implementation NSData (DataView)

+ (instancetype)dataWithDataView:(marlin::store::data_view)data {
  return [[NSData alloc] initWithDataView:std::move(data)];
}

- (instancetype)initWithDataView:(marlin::store::data_view)data {
  return [self initWithBytes:data.begin() length:data.size()];
}

- (marlin::store::data_view)dataView {
  return {static_cast<const std::byte*>(self.bytes), self.length};
}

@end

using ColorElements = std::array<CGFloat, marlin::control::color_literal::data_dimension_max>;

inline ColorElements hslFromHSB(const ColorElements& hsb) {
  constexpr size_t h = 0, s = 1, l = 2, b = 2;
  ColorElements hsl;
  hsl[h] = hsb[h];
  hsl[l] = hsb[b] - hsb[b] * hsb[s] / 2;
  hsl[s] = hsl[l] > 0 && hsl[l] < 1 ? (hsb[b] - hsl[l]) / (hsl[l] < 0.5 ? hsl[l] : 1 - hsl[l]) : 0;
  return hsl;
}

inline ColorElements hsbFromHSL(const ColorElements& hsl) {
  constexpr size_t h = 0, s = 1, l = 2, b = 2;
  ColorElements hsb;
  hsb[h] = hsl[h];
  hsb[b] = hsl[l] + hsl[s] * (hsl[l] < 0.5 ? hsl[l] : 1 - hsl[l]);
  hsb[s] = hsb[b] > 0 ? 2 - 2 * hsl[l] / hsb[b] : 0;
  return hsb;
}

@implementation Color (ColorLiteral)

+ (instancetype)colorWithColorLiteral:(marlin::control::color_literal)colorLiteral {
  ColorElements colorElements;
  for (auto i = 0; i < colorLiteral.data_dimension(); ++i) {
    colorElements[i] = colorLiteral[i] / colorLiteral.data_max(i);
  }
  switch (colorLiteral.mode) {
    case marlin::ast::color_mode::rgb:
      NSAssert(colorLiteral.data_dimension() == 3, @"");
      return [Color colorWithCalibratedRed:colorElements[0]
                                     green:colorElements[1]
                                      blue:colorElements[2]
                                     alpha:1];
    case marlin::ast::color_mode::rgba:
      NSAssert(colorLiteral.data_dimension() == 4, @"");
      return [Color colorWithCalibratedRed:colorElements[0]
                                     green:colorElements[1]
                                      blue:colorElements[2]
                                     alpha:colorElements[3]];
    case marlin::ast::color_mode::hsl: {
      NSAssert(colorLiteral.data_dimension() == 3, @"");
      auto hsb = hsbFromHSL(colorElements);
      return [Color colorWithCalibratedHue:hsb[0] saturation:hsb[1] brightness:hsb[2] alpha:1];
    }
    case marlin::ast::color_mode::hsla: {
      NSAssert(colorLiteral.data_dimension() == 4, @"");
      auto hsb = hsbFromHSL(colorElements);
      return [Color colorWithCalibratedHue:hsb[0]
                                saturation:hsb[1]
                                brightness:hsb[2]
                                     alpha:colorElements[3]];
    }
  }
}

- (marlin::control::color_literal)colorLiteralWithMode:(marlin::ast::color_mode)mode {
  marlin::control::color_literal colorLiteral{mode};
  ColorElements colorElements;
  switch (mode) {
    case marlin::ast::color_mode::rgb:
      [self getRed:&colorElements[0] green:&colorElements[1] blue:&colorElements[2] alpha:nil];
      for (auto i = 0; i < colorLiteral.data_dimension(); ++i) {
        colorElements[i] *= colorLiteral.data_max(i);
      }
      colorLiteral.set(colorElements[0], colorElements[1], colorElements[2]);
      break;
    case marlin::ast::color_mode::rgba:
      [self getRed:&colorElements[0]
             green:&colorElements[1]
              blue:&colorElements[2]
             alpha:&colorElements[3]];
      for (auto i = 0; i < colorLiteral.data_dimension(); ++i) {
        colorElements[i] *= colorLiteral.data_max(i);
      }
      colorLiteral.set(colorElements[0], colorElements[1], colorElements[2], colorElements[3]);
      break;
    case marlin::ast::color_mode::hsl: {
      [self getHue:&colorElements[0]
          saturation:&colorElements[1]
          brightness:&colorElements[2]
               alpha:nil];
      auto hsl = hslFromHSB(colorElements);
      for (auto i = 0; i < colorLiteral.data_dimension(); ++i) {
        hsl[i] *= colorLiteral.data_max(i);
      }
      colorLiteral.set(hsl[0], hsl[1], hsl[2]);
      break;
    }
    case marlin::ast::color_mode::hsla: {
      [self getHue:&colorElements[0]
          saturation:&colorElements[1]
          brightness:&colorElements[2]
               alpha:&colorElements[3]];
      auto hsl = hslFromHSB(colorElements);
      for (auto i = 0; i < colorLiteral.data_dimension(); ++i) {
        hsl[i] *= colorLiteral.data_max(i);
      }
      colorLiteral.set(hsl[0], hsl[1], hsl[2], colorElements[3]);
      break;
    }
  }
  return colorLiteral;
}

@end

#ifdef IOS

@implementation UISegmentedControl (MacInterface)

- (NSUInteger)selectedSegment {
  return self.selectedSegmentIndex;
}

@end

@implementation UITextField (MacInterface)

- (NSString*)stringValue {
  return self.text;
}

- (void)setStringValue:(NSString*)stringValue {
  self.text = stringValue;
}

@end

@implementation UITextView (MacInterface)

- (NSString*)string {
  return self.text;
}

@end

#endif
