-- Add the table, figure, and equation numbering that Pandoc's LaTeX reader
-- does not preserve completely in HTML.  The report document class resets
-- all three counters at every chapter, so the HTML numbers use
-- chapter.counter too.

local chapter = 0
local table_counter = 0
local figure_counter = 0
local equation_counter = 0
local references = {}
local source_table_numbers = {}
local source_figure_numbers = {}

local numbered_environments = {
  equation = "single",
  align = "rows",
  gather = "rows",
  multline = "single",
  eqnarray = "rows",
}

local function attr(identifier, classes, attributes)
  return pandoc.Attr(identifier or "", classes or {}, attributes or {})
end

local function number_string(counter)
  return tostring(chapter) .. "." .. tostring(counter)
end

local function has_class(el, class_name)
  return el.classes and el.classes:includes(class_name)
end

local function read_file(path)
  local file = io.open(path, "r")
  if not file then
    return nil
  end
  local contents = file:read("*a")
  file:close()
  return contents
end

local function without_comments(source)
  local lines = {}
  for line in (source .. "\n"):gmatch("(.-)\n") do
    local search_from = 1
    while true do
      local comment = line:find("%", search_from, true)
      if not comment then
        break
      end

      local backslashes = 0
      local index = comment - 1
      while index > 0 and line:sub(index, index) == "\\" do
        backslashes = backslashes + 1
        index = index - 1
      end
      if backslashes % 2 == 0 then
        line = line:sub(1, comment - 1)
        break
      end
      search_from = comment + 1
    end
    lines[#lines + 1] = line
  end
  return table.concat(lines, "\n")
end

local function with_tex_extension(path)
  if path:match("%.tex$") then
    return path
  end
  return path .. ".tex"
end

local function expanded_source(path, document_directory, active)
  path = with_tex_extension(path)
  local source = read_file(path)
  if not source or active[path] then
    return ""
  end

  active[path] = true
  local current_directory = pandoc.path.directory(path)
  source = without_comments(source)
  source = source:gsub("\\input%s*{([^}]+)}", function(input)
    local candidates = {
      pandoc.path.join({ document_directory, with_tex_extension(input) }),
      pandoc.path.join({ current_directory, with_tex_extension(input) }),
    }
    for _, candidate in ipairs(candidates) do
      if read_file(candidate) then
        return expanded_source(candidate, document_directory, active)
      end
    end
    return ""
  end)
  active[path] = nil
  return source
end

local function next_source_structure(source, position)
  local candidates = {}
  local chapter_start, chapter_end = source:find("\\chapter%s*%b{}", position)
  if chapter_start then
    candidates[#candidates + 1] = { chapter_start, chapter_end, "chapter" }
  end
  for _, environment in ipairs({ "table", "longtable", "figure" }) do
    local start_at, end_at = source:find("\\begin%s*{" .. environment .. "}", position)
    if start_at then
      candidates[#candidates + 1] = { start_at, end_at, environment }
    end
  end
  table.sort(candidates, function(left, right) return left[1] < right[1] end)
  return candidates[1]
end

local function collect_source_object_numbers()
  local input_file = PANDOC_STATE.input_files[1]
  if not input_file or input_file == "-" then
    return
  end

  local document_directory = pandoc.path.directory(input_file)
  local source = expanded_source(input_file, document_directory, {})
  local source_chapter = 0
  local source_table_counter = 0
  local source_figure_counter = 0
  local position = 1

  while true do
    local item = next_source_structure(source, position)
    if not item then
      break
    end

    if item[3] == "chapter" then
      source_chapter = source_chapter + 1
      source_table_counter = 0
      source_figure_counter = 0
      position = item[2] + 1
    else
      local environment = item[3]
      local _, environment_end = source:find("\\end%s*{" .. environment .. "}", item[2] + 1)
      if not environment_end then
        position = item[2] + 1
      else
        local body = source:sub(item[2] + 1, environment_end)
        local has_numbered_caption = body:find("\\caption%s*[%[{]") ~= nil
        if environment == "figure" then
          if has_numbered_caption then
            source_figure_counter = source_figure_counter + 1
          end
          for label in body:gmatch("\\label%s*{([^}]+)}") do
            source_figure_numbers[label] = tostring(source_chapter) .. "." .. tostring(source_figure_counter)
          end
        else
          if environment == "longtable" or has_numbered_caption then
            source_table_counter = source_table_counter + 1
          end
          for label in body:gmatch("\\label%s*{([^}]+)}") do
            if label:match("^table:") or label:match("^tab:") then
              source_table_numbers[label] = tostring(source_chapter) .. "." .. tostring(source_table_counter)
            end
          end
        end
        position = environment_end + 1
      end
    end
  end
end

collect_source_object_numbers()

local function prepend_caption_number(caption, kind, number)
  local prefix = pandoc.Inlines({
    pandoc.Str(kind),
    pandoc.Space(),
    pandoc.Str(number .. ":"),
    pandoc.Space(),
  })

  if #caption.long == 0 then
    caption.long = pandoc.Blocks({ pandoc.Plain(prefix) })
    return
  end

  local first = caption.long[1]
  if first.t == "Plain" or first.t == "Para" then
    for index = #prefix, 1, -1 do
      first.content:insert(1, prefix[index])
    end
  else
    caption.long:insert(1, pandoc.Plain(prefix))
  end
end

local function strip_outer_environment(tex)
  local environment = tex:match("^%s*\\begin%s*{([^}]+)}")
  if not environment then
    return nil, nil
  end

  local body = tex:gsub("^%s*\\begin%s*{[^}]+}%s*", "", 1)
  body = body:gsub("%s*\\end%s*{[^}]+}%s*$", "", 1)
  return environment, body
end

local function split_top_level_rows(body)
  local rows = {}
  local row_start = 1
  local nested_depth = 0
  local index = 1

  while index <= #body do
    if body:sub(index, index + 6) == "\\begin{" then
      nested_depth = nested_depth + 1
      index = index + 7
    elseif body:sub(index, index + 4) == "\\end{" then
      nested_depth = math.max(0, nested_depth - 1)
      index = index + 5
    elseif nested_depth == 0 and body:sub(index, index + 1) == "\\\\" then
      rows[#rows + 1] = body:sub(row_start, index - 1)
      row_start = index + 2
      index = index + 2
    else
      index = index + 1
    end
  end

  rows[#rows + 1] = body:sub(row_start)
  return rows
end

local function labels_in(value)
  local labels = {}
  for label in value:gmatch("\\label%s*{([^}]+)}") do
    labels[#labels + 1] = label
  end
  return labels
end

local function clean_equation_tex(tex)
  tex = tex:gsub("\\label%s*{[^}]+}", "")
  tex = tex:gsub("\\nonumber", "")
  tex = tex:gsub("\\notag", "")
  return tex
end

local function number_equation(math)
  local environment, body = strip_outer_environment(math.text)
  local mode = environment and numbered_environments[environment]
  if not mode or environment:sub(-1) == "*" then
    return nil
  end

  local rows = mode == "rows" and split_top_level_rows(body) or { body }
  local numbers = {}

  for _, row in ipairs(rows) do
    local suppressed = row:match("\\nonumber") or row:match("\\notag")
    if not suppressed then
      equation_counter = equation_counter + 1
      local number = number_string(equation_counter)
      local labels = labels_in(row)
      numbers[#numbers + 1] = { number = number, labels = labels }
      for _, label in ipairs(labels) do
        references[label] = { number = number, kind = "equation" }
      end
    end
  end

  math.text = clean_equation_tex(math.text)
  return numbers
end

local function equation_number_blocks(numbers)
  local blocks = pandoc.Blocks({})
  for _, entry in ipairs(numbers) do
    local identifier = entry.labels[1] or ""
    local number = pandoc.Span(
      { pandoc.Str("(" .. entry.number .. ")") },
      attr(identifier, { "equation-number" }, { ["data-number"] = entry.number })
    )
    local inlines = pandoc.Inlines({ number })

    -- It is unusual for one equation row to have multiple labels, but every
    -- LaTeX label still needs a valid HTML destination.
    for index = 2, #entry.labels do
      inlines:insert(pandoc.Span({}, attr(entry.labels[index], { "equation-alias" })))
    end
    blocks:insert(pandoc.Plain(inlines))
  end
  return blocks
end

local function display_equation(math, numbers)
  local number_values = {}
  for _, entry in ipairs(numbers) do
    number_values[#number_values + 1] = entry.number
  end

  return pandoc.Div(
    {
      pandoc.Div({ pandoc.Plain({ math }) }, attr("", { "equation-math" })),
      pandoc.Div(equation_number_blocks(numbers), attr("", { "equation-numbers" })),
    },
    attr("", { "equation" }, { ["data-number"] = table.concat(number_values, ",") })
  )
end

local function inline_equation(math, numbers)
  local number_inlines = pandoc.Inlines({})
  for _, block in ipairs(equation_number_blocks(numbers)) do
    for _, inline in ipairs(block.content) do
      number_inlines:insert(inline)
    end
  end
  return pandoc.Span(
    {
      pandoc.Span({ math }, attr("", { "equation-math" })),
      pandoc.Span(number_inlines, attr("", { "equation-numbers" })),
    },
    attr("", { "equation", "equation-inline" })
  )
end

local function meaningful_inlines(inlines)
  local result = {}
  for _, inline in ipairs(inlines) do
    if inline.t ~= "Space" and inline.t ~= "SoftBreak" and inline.t ~= "LineBreak" then
      result[#result + 1] = inline
    end
  end
  return result
end

local numbering_filter = {
  traverse = "topdown",

  Header = function(el)
    if el.level == 1 and not has_class(el, "unnumbered") then
      chapter = chapter + 1
      table_counter = 0
      figure_counter = 0
      equation_counter = 0
    end
  end,

  Table = function(el)
    -- LaTeX advances the table counter at \caption, not at \begin{table}.
    -- Pandoc also represents bare tabular environments as Table nodes, so do
    -- not number those uncaptioned layout tables.
    if pandoc.utils.stringify(el.caption) == "" then
      return nil
    end

    local number = source_table_numbers[el.identifier]
    if number then
      table_counter = tonumber(number:match("%.(%d+)$")) or table_counter
    else
      table_counter = table_counter + 1
      number = number_string(table_counter)
    end
    el.attributes["data-number"] = number
    prepend_caption_number(el.caption, "Table", number)

    if el.identifier ~= "" then
      references[el.identifier] = { number = number, kind = "table" }
    end
    return el
  end,

  Figure = function(el)
    if pandoc.utils.stringify(el.caption) == "" then
      return nil
    end

    local number = source_figure_numbers[el.identifier]
    if number then
      figure_counter = tonumber(number:match("%.(%d+)$")) or figure_counter
    else
      figure_counter = figure_counter + 1
      number = number_string(figure_counter)
    end
    el.attributes["data-number"] = number
    prepend_caption_number(el.caption, "Figure", number)

    if el.identifier ~= "" then
      references[el.identifier] = { number = number, kind = "figure" }
      local remove_duplicate_anchor = {
        Span = function(span)
          if span.identifier == el.identifier then
            return {}
          end
        end,
      }
      el.caption.long = el.caption.long:walk(remove_duplicate_anchor)
      el.content = el.content:walk(remove_duplicate_anchor)
    end
    return el
  end,

  Para = function(el)
    local meaningful = meaningful_inlines(el.content)
    if #meaningful == 1 and meaningful[1].t == "Math" and meaningful[1].mathtype == "DisplayMath" then
      local numbers = number_equation(meaningful[1])
      if numbers and #numbers > 0 then
        return display_equation(meaningful[1], numbers), false
      end
      return nil
    end

    local changed = false
    for index, inline in ipairs(el.content) do
      if inline.t == "Math" and inline.mathtype == "DisplayMath" then
        local numbers = number_equation(inline)
        if numbers and #numbers > 0 then
          el.content[index] = inline_equation(inline, numbers)
          changed = true
        end
      end
    end
    if changed then
      return el, false
    end
    return nil
  end,

  -- Some LaTeX grouping commands cause Pandoc to put a display equation
  -- inside a Span.  Those equations are not direct children of their Para,
  -- so handle them during the normal inline traversal.
  Math = function(el)
    if el.mathtype ~= "DisplayMath" then
      return nil
    end
    local numbers = number_equation(el)
    if numbers and #numbers > 0 then
      return inline_equation(el, numbers), false
    end
    return nil
  end,
}

local reference_filter = {
  Link = function(el)
    local label = el.attributes.reference or el.target:match("^#(.+)$")
    local reference = label and references[label]
    if not reference then
      return nil
    end

    local reference_type = el.attributes["reference-type"]
    local text = reference.number
    if reference.kind == "equation" and reference_type == "eqref" then
      text = "(" .. text .. ")"
    end
    el.content = pandoc.Inlines({ pandoc.Str(text) })
    return el
  end,
}

return { numbering_filter, reference_filter }
