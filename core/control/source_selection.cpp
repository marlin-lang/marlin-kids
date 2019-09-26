#include "source_selection.hpp"

namespace marlin::control {

std::vector<source_update> source_selection::set_new_array_elements_count(
    size_t count) {
  assert(is_new_array());
  assert(count >= get_new_array_minimum_count());

  std::vector<source_update> result;
  _doc->start_recording_side_effects();

  auto elements{_selection->as<ast::new_array>().elements()};
  const bool need_refresh = elements.size() != count;
  if (elements.size() < count) {
    for (auto i{elements.size()}; i < count; i++) {
      elements.emplace_back(ast::make<ast::expression_placeholder>(
          placeholder::get<ast::new_array>({0, i})));
    }
  } else if (elements.size() > count) {
    for (auto i{elements.size() - 1}; elements.size() > count; i--) {
      if (elements[i]->is<ast::expression_placeholder>()) {
        elements.pop(i);
      }
      if (i == 0) {
        break;
      }
    }
  }

  assert(elements.size() == count);
  if (need_refresh) {
    result.emplace_back(_doc->refresh_node_display(*_selection));
  }

  _doc->gather_side_effects(result);
  return result;
}

std::vector<source_update> source_selection::set_color_literal(
    color_literal literal) {
  assert(is_color_literal());

  std::vector<source_update> result;
  _doc->start_recording_side_effects();

  _selection->as<ast::new_color>().mode = literal.mode;
  auto args{_selection->as<ast::new_color>().arguments()};
  args.clear();
  for (size_t i{0}; i < literal.data_dimension(); i++) {
    std::ostringstream os;
    os << literal[i];
    args.emplace_back(ast::make<ast::number_literal>(os.str()));
  }

  assert(args.size() == literal.data_dimension());
  result.emplace_back(_doc->refresh_node_display(*_selection));

  _doc->gather_side_effects(result);
  return result;
}

document_update source_selection::remove_from_document() && {
  assert(is_removable());

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
  } else if (_selection->is<ast::parameter>()) {
    result.source_updates.emplace_back(
        std::move(*this).remove_parameter(result.selection_update));
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
  return {line_range, {}};
}

source_update source_selection::remove_parameter(
    std::optional<source_selection>& result_selection) && {
  assert(_selection->is<ast::parameter>());
  assert(_selection->has_parent());
  assert(!result_selection.has_value());

  auto& parent{_selection->parent()};
  source_range result_range;
  if (parent.is<ast::function_signature>()) {
    auto& signature{parent.as<ast::function_signature>()};
    std::tie(std::ignore, result_range) =
        _doc->remove_argument(signature.parameters(), *_selection);

    _doc->refresh_function_signature(signature.name, signature);
  } else {
    assert(_selection->parent().is<ast::function_placeholder>());
    std::tie(std::ignore, result_range) = _doc->remove_argument(
        parent.as<ast::function_placeholder>().parameters(), *_selection);
  }

  return {result_range, {}};
}

source_update source_selection::remove_expression(
    std::optional<source_selection>& result_selection) && {
  assert(is<pasteboard_t::expression>() || is<pasteboard_t::reference>());
  assert(_selection->has_parent());
  assert(!result_selection.has_value());

  auto placeholder_name{placeholder::get_replacing_node(*_selection)};
  if (_selection->parent().is<ast::user_function_call>() &&
      placeholder::is_empty(placeholder_name)) {
    auto& call{_selection->parent().as<ast::user_function_call>()};
    auto [node, range]{_doc->remove_argument(call.arguments(), *_selection)};
    return source_update{range, {"", {}}};
  }

  auto original_range{_selection->source_code_range};

  auto placeholder{
      _selection->inherits<ast::lvalue>()
          ? ast::make<ast::variable_placeholder>(std::move(placeholder_name))
          : ast::make<ast::expression_placeholder>(
                std::move(placeholder_name))};
  result_selection = source_selection{*_doc, *placeholder, dropping_rule};

  format::in_place_formatter formatter;
  auto display{formatter.format(placeholder, *_selection)};
  _doc->replace_expression(*_selection, std::move(placeholder));
  return source_update{original_range, std::move(display)};
}

document_update source_selection::insert_literal(literal_data_type type,
                                                 std::string_view literal) && {
  switch (type) {
    case literal_data_type::parameter:
      [[fallthrough]];
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
      params.emplace_back(ast::make<ast::parameter>(param));
    }

    ast::node node;
    if (signature.name.length() > 0) {
      node =
          ast::make<ast::function_signature>(signature.name, std::move(params));
    } else {
      node = ast::make<ast::function_placeholder>(
          placeholder::get<ast::function>({0}), std::move(params));
    }
    result.selection_update = source_selection{*_doc, *node, dropping_rule};

    format::in_place_formatter formatter;
    auto display{formatter.format(node, *_selection)};

    result.source_updates.emplace_back(original_range, std::move(display));

    if (_selection->is<ast::function_signature>()) {
      const auto& previous_name{_selection->as<ast::function_signature>().name};
      if (signature.name.length() > 0) {
        _doc->replace_function(previous_name, std::move(signature));
      } else {
        _doc->remove_function(previous_name);
      }
    } else if (signature.name.length() > 0) {
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
