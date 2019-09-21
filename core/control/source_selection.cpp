#include "source_selection.hpp"

namespace marlin::control {

std::vector<source_update> source_selection::remove_from_document() const&& {
  std::vector<source_update> updates;
  assert(dragging_type().has_value());

  _doc->start_recording_side_effects();

  if (is_block()) {
    if (!_selection->is<ast::on_start>()) {
      if (_selection->is<ast::function>()) {
        auto& signature = *_selection->as<ast::function>().signature();
        if (signature.is<ast::function_signature>()) {
          _doc->remove_function(signature.as<ast::function_signature>().name);
        }
      }

      updates.emplace_back(std::move(*this).remove_line());
    }
  } else if (is_statement()) {
    updates.emplace_back(std::move(*this).remove_line());
  } else if (is_reference() || is_expression()) {
    updates.emplace_back(std::move(*this).remove_expression());
  } else {
    // These things are not draggable
    assert(false);
  }

  _doc->gather_side_effects(updates);
  return updates;
}

source_update source_selection::remove_expression() const&& {
  assert(is_reference() || is_expression());
  assert(_selection->has_parent());

  format::in_place_formatter formatter;

  auto placeholder_name{placeholder::get_replacing_node(*_selection)};
  if (_selection->parent().is<ast::user_function_call>() &&
      placeholder_name == placeholder::empty) {
    auto& call{_selection->parent().as<ast::user_function_call>()};
    auto original_range{call.source_code_range};

    auto args{call.arguments()};
    for (auto i{0}; i < args.size(); i++) {
      if (args[i].get() == _selection) {
        args.pop(i);

        auto display{formatter.format(call, call)};
        return source_update{original_range, std::move(display)};
      }
    }
    // _selection not found, this is unexpected
    assert(false);
  }

  auto original_range{_selection->source_code_range};

  auto placeholder{_selection->is<ast::variable_placeholder>() ||
                           _selection->inherits<ast::lvalue>()
                       ? ast::make<ast::variable_placeholder>(
                             std::string{std::move(placeholder_name)})
                       : ast::make<ast::expression_placeholder>(
                             std::string{std::move(placeholder_name)})};

  auto display{formatter.format(placeholder, *_selection)};
  _doc->replace_expression(*_selection, std::move(placeholder));
  return source_update{original_range, std::move(display)};
}

std::vector<source_update> source_selection::replace_function_signature(
    function_definition signature) const&& {
  std::vector<source_update> updates;
  _doc->start_recording_side_effects();

  if (is_function_signature()) {
    auto original_range{_selection->source_code_range};

    assert(signature.name.length() > 0);
    std::vector<ast::node> params;
    for (auto& param : signature.parameters) {
      params.emplace_back(ast::make<ast::variable_name>(param));
    }
    auto node{
        ast::make<ast::function_signature>(signature.name, std::move(params))};

    format::in_place_formatter formatter;
    auto display{formatter.format(node, *_selection)};

    updates.emplace_back(original_range, std::move(display));

    if (_selection->is<ast::function_signature>()) {
      const auto& previous_name{_selection->as<ast::function_signature>().name};
      _doc->replace_function(previous_name, std::move(signature));
    } else {
      _doc->add_function(std::move(signature));
    }

    _doc->replace_expression(*_selection, std::move(node));
  } else {
    assert(false);
  }

  _doc->gather_side_effects(updates);
  return updates;
}

}  // namespace marlin::control
