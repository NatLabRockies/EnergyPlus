-- Move the LaTeX title page (doc/title.tex.in: version, logo, build, copyright) onto index.html.
--
-- Everything before the first heading would otherwise become its own untitled chunk
-- (<guide>-1.html) that the table of contents never links to. The title page blocks are
-- removed from the body and stored in the `title-page` metadata, which template_chunked.html
-- renders on the index page only.

local function is_empty(block)
  return block.t == "Para" and pandoc.utils.stringify(block) == ""
end

function Pandoc(doc)
  local first_header = nil
  for i, block in ipairs(doc.blocks) do
    if block.t == "Header" then
      first_header = i
      break
    end
  end
  if first_header == nil or first_header == 1 then
    return nil
  end

  local date = doc.meta.date and pandoc.utils.stringify(doc.meta.date) or nil
  local title_page = pandoc.Blocks({})
  for i = 1, first_header - 1 do
    local block = doc.blocks[i]
    if block.t == "Div" and block.classes:includes("titlepage") then
      -- Flatten the title page and drop what the index header already shows (title, author, date)
      block:walk({
        Para = function(para)
          local text = pandoc.utils.stringify(para)
          if not is_empty(para) and text ~= date then
            title_page:insert(para)
          end
        end,
      })
    end
  end

  local body = pandoc.Blocks({})
  for i = first_header, #doc.blocks do
    body:insert(doc.blocks[i])
  end
  doc.blocks = body
  if #title_page > 0 then
    doc.meta["title-page"] = pandoc.MetaBlocks(title_page)
  end
  return doc
end
