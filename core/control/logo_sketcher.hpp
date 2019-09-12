#ifndef marlin_control_logo_sketcher_hpp
#define marlin_control_logo_sketcher_hpp

#include <cmath>

namespace marlin::control {

constexpr double pi = 3.141592653589793238463;
constexpr double rad_per_deg = pi / 180;

template <typename draw_context_type>
struct logo_sketcher {
  void set_context(draw_context_type& ctx) {
    _ctx = &ctx;
    _x = _ctx->width() / 2;
    _y = _ctx->height() / 2;
  }

  void forward(double length) {
    auto original_x{_x};
    auto original_y{_y};
    _x -= length * sin(_dir);
    _y += length * cos(_dir);
    _ctx->draw_line(original_x, original_y, _x, _y);
  }
  void backward(double length) { forward(-length); }

  void turn_left(double degree) {
    _dir = remainder(_dir + degree * rad_per_deg, pi * 2);
  }
  void turn_right(double degree) { turn_left(-degree); }

  template <typename native_environment>
  static void register_instructions(native_environment& env) {
    env.template register_native_instruction<double>(
        "logo_forward",
        [](auto self, double length) { self->forward(length); });
    env.template register_native_instruction<double>(
        "logo_backward",
        [](auto self, double length) { self->backward(length); });
    env.template register_native_instruction<double>(
        "logo_turn_left",
        [](auto self, double degree) { self->turn_left(degree); });
    env.template register_native_instruction<double>(
        "logo_turn_right",
        [](auto self, double degree) { self->turn_right(degree); });
  }

 private:
  draw_context_type* _ctx;

  double _x{0};
  double _y{0};
  double _dir{0};
};

}  // namespace marlin::control

#endif  // marlin_control_logo_sketcher_hpp
