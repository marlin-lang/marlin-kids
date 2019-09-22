#include "source_selection.hpp"

namespace marlin::control {

document_update source_selection::remove_from_document() && {
  assert(dragging_type().has_value());

  document_update result;
  _doc->start_recording_side_effects();

  if (is<pasteboard_t::block>()) {
    if (!_selection->is<ast::on_start>()) {
      if (_selection->is<ast::function>()) {
        auto& signature = *_selection->as<ast::function>().signature();
        if (signature.is<ast::function_signature>()) {
          _doc->remove_function(signature.as<ast::function_signature>().name);
        }
      }

      result.source_updates.emplace_back(
          std::move(*this).remove_line(result.selection_update));
    }
  } else if (is<pasteboard_t::statement>()) {
    result.source_updates.emplace_back(
        std::move(*this).remove_line(result.selection_update));
  } else if (is<pasteboard_t::expression>() || is<pasteboard_t::reference>()) {
    result.source_updates.emplace_back(
        std::move(*this).remove_expression(result.selection_update));
  } else {
    // These things are not draggable
    assert(false);
  }

  _doc->gather_side_effects(result.source_updates);
  return result;
}

source_update source_selection::remove_line(
    std::optional<source_selection>& result_selection) && {
  assert(is<pasteboard_t::block>() || is<pasteboard_t::statement>());
  assert(_selection->has_parent());
  assert(!result_selection.has_value());

  source_range line_range{{_selection->source_code_range.begin.line, 1},
                          {_selection->source_code_range.end.line + 1, 1}};
  auto line_offset{static_cast<ptrdiff_t>(line_range.begin.line) -
                   static_cast<ptrdiff_t>(line_range.end.line)};
  _doc->update_source_line_after_node(*_selection, line_offset);
  _doc->remove_line(*_selection);
  return source_update{line_range, {"", {}}};
}

source_update source_selection::remove_expression(
    std::optional<source_selection>& result_selection) && {
  assert(is<pasteboard_t::expression>() || is<pasteboard_t::reference>());
  assert(_selection->has_parent());
  assert(!result_selection.has_value());

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

  auto placeholder{_selection->inherits<ast::lvalue>()
                       ? ast::make<ast::variable_placeholder>(
                             std::string{std::move(placeholder_name)})
                       : ast::make<ast::expression_placeholder>(
                             std::string{std::move(placeholder_name)})};
  result_selection = source_selection{*_doc, *placeholder};

  auto display{formatter.format(placeholder, *_selection)};
  _doc->replace_expression(*_selection, std::move(placeholder));
  return source_update{original_range, std::move(display)};
}

document_update source_selection::insert_literal(literal_data_type type,
                                                 std::string_view literal) && {
  switch (type) {
    case literal_data_type::variable_name:
      [[fallthrough]];
    case literal_data_type::identifier:
      return std::move(*this)
          .as_inserter<pasteboard_t::reference>()
          .insert_literal(type, std::move(literal));
    case literal_data_type::number:
      [[fallthrough]];
    case literal_data_type::string:
      return std::move(*this)
          .as_inserter<pasteboard_t::expression>()
          .insert_literal(type, std::move(literal));
  }
}

document_update source_selection::replace_function_signature(
    function_definition signature) && {
  document_update result;
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
    result.selection_update = source_selection{*_doc, *node};

    format::in_place_formatter formatter;
    auto display{formatter.format(node, *_selection)};

    result.source_updates.emplace_back(original_range, std::move(display));

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

  _doc->gather_side_effects(result.source_updates);
  return result;
}

}  // namespace marlin::control
