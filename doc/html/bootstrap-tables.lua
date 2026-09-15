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
  el.attr.classes:insert("table")
  el.attr.classes:insert("table-bordered")
  el.attr.classes:insert("table-striped")
  el.attr.classes:insert("table-sm")
  return el
end
