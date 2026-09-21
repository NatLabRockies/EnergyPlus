local function trim(value)
  return value:gsub("\194\160", ""):match("^%s*(.-)%s*$")
end

local function split_cells(line)
  local cells = {}
  local start = 1
  while #cells < 9 do
    local separator = line:find("&", start, true)
    if not separator then
      return nil
    end
    table.insert(cells, trim(line:sub(start, separator - 1)))
    start = separator + 1
  end
  table.insert(cells, trim(line:sub(start)))
  return cells
end

local function extract_lines(el)
  local lines = {}
  for _, block in ipairs(el.content) do
    local current = pandoc.Inlines({})
    for _, inline in ipairs(block.content or {}) do
      if inline.t == "LineBreak" then
        table.insert(lines, trim(pandoc.utils.stringify(current)))
        current = pandoc.Inlines({})
      elseif inline.t ~= "Span" then
        current:insert(inline)
      end
    end
    table.insert(lines, trim(pandoc.utils.stringify(current)))
  end
  return lines
end

local function is_description(value)
  return value:match("^Air mass 1%.5")
    or value:match("^37 degree tilted")
    or value:match("^Based on ISO")
    or value:match("^ISO%-9845")
    or value:match("^Photopic response")
    or value:match("^Based on CIE")
    or value:match("^derived from Optics5")
    or value:match("^which is the same")
    or value:match("^Wavelengths %(microns%)")
end

local function text_blocks(value)
  return { pandoc.Plain({ pandoc.Str(value) }) }
end

local function cell_text(cell)
  return trim(pandoc.utils.stringify(cell.contents or {}))
end

local function row_is_empty(row)
  for _, cell in ipairs(row.cells or {}) do
    if cell_text(cell) ~= "" then
      return false
    end
  end
  return true
end

local function rows_match(first, second)
  local first_cells = first.cells or {}
  local second_cells = second.cells or {}
  if #first_cells ~= #second_cells then
    return false
  end

  for index, cell in ipairs(first_cells) do
    if cell_text(cell) ~= cell_text(second_cells[index]) then
      return false
    end
  end
  return true
end

local function rows_match_at(rows, start_index, expected_rows)
  for offset, expected in ipairs(expected_rows) do
    local row = rows[start_index + offset - 1]
    if not row or not rows_match(expected, row) then
      return false
    end
  end
  return true
end

local function clear_booktabs_rule_artifacts(row)
  for _, cell in ipairs(row.cells or {}) do
    -- Pandoc sometimes renders a \cmidrule(r){2-4} range as a literal
    -- "2-4" span in the next header row.
    if cell_text(cell):match("^%d+%s*%-%s*%d+$") then
      cell.contents = pandoc.Blocks({})
    end
  end
end

local function remove_repeated_header(el)
  local head_rows = el.head and el.head.rows or {}
  if #head_rows == 0 then
    return
  end

  for _, body in ipairs(el.bodies or {}) do
    local body_rows = body.body or {}

    -- A longtable's first-page header can have continuation rows that Pandoc
    -- places at the start of the body.  The repeated \endhead block then
    -- follows those rows, often separated by an empty row.  Locate that
    -- repeated copy near the beginning instead of requiring it at row one.
    local repeated_header_start
    for index = 1, math.min(#body_rows, 20) do
      if rows_match_at(body_rows, index, head_rows) then
        repeated_header_start = index
        break
      end
    end

    if repeated_header_start then
      local continuation_rows = {}
      local last_continuation_index = 0
      for index = 1, repeated_header_start - 1 do
        if not row_is_empty(body_rows[index]) then
          continuation_rows[#continuation_rows + 1] = body_rows[index]
          last_continuation_index = index
        end
      end

      local duplicate_end = repeated_header_start + #head_rows - 1
      local duplicate_continuation_start = duplicate_end + 1
      while body_rows[duplicate_continuation_start]
          and row_is_empty(body_rows[duplicate_continuation_start]) do
        duplicate_continuation_start = duplicate_continuation_start + 1
      end
      if #continuation_rows > 0
          and rows_match_at(body_rows, duplicate_continuation_start, continuation_rows) then
        duplicate_end = duplicate_continuation_start + #continuation_rows - 1
      end

      for _, row in ipairs(continuation_rows) do
        clear_booktabs_rule_artifacts(row)
      end

      local first_removal = last_continuation_index + 1
      local rows_to_remove = duplicate_end - first_removal + 1
      for _ = 1, rows_to_remove do
        table.remove(body_rows, first_removal)
      end
    end
  end
end

function Div(el)
  if not el.classes:includes("tabular") then
    return nil
  end

  local lines = extract_lines(el)
  local rows = {}
  local identifier
  local caption

  for _, line in ipairs(lines) do
    local values = split_cells(line)
    if values then
      if values[1]:match("^Air mass 1%.5") then
        identifier = "table:solar-spectral-irradiance-function"
        caption = "Solar spectral irradiance function."
      elseif values[1]:match("^Photopic response") then
        identifier = "table:photopic-response-function"
        caption = "Photopic response function."
      end

      local cells = {}
      if is_description(values[1]) then
        cells[1] = pandoc.Cell(text_blocks(values[1]), pandoc.AlignLeft, 1, 10)
      else
        for column, value in ipairs(values) do
          cells[column] = pandoc.Cell(text_blocks(value), pandoc.AlignRight)
        end
      end
      table.insert(rows, pandoc.Row(cells))
    end
  end

  if not identifier then
    return nil
  end

  local colspecs = {}
  for _ = 1, 10 do
    table.insert(colspecs, { pandoc.AlignRight })
  end

  return pandoc.Table(
    { long = text_blocks(caption) },
    colspecs,
    pandoc.TableHead({}),
    {
      {
        attr = pandoc.Attr(),
        head = {},
        body = rows,
        row_head_columns = 0,
      },
    },
    pandoc.TableFoot({}),
    pandoc.Attr(identifier, { "table", "table-bordered", "table-striped", "table-sm" })
  )
end

function Span(el)
  if el.identifier == "table:solar-spectral-irradiance-function"
      or el.identifier == "table:photopic-response-function" then
    return {}
  end
end

function Table(el)
  remove_repeated_header(el)
  el.attr.classes:insert("table")
  el.attr.classes:insert("table-bordered")
  el.attr.classes:insert("table-striped")
  el.attr.classes:insert("table-sm")
  return el
end
