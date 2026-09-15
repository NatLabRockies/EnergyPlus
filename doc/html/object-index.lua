-- Collect all h3 headings (object class names) and append an alphabetical Object Index

local objects = {}

function Header(el)
  if el.level == 3 and el.identifier ~= "" then
    table.insert(objects, {
      text = pandoc.utils.stringify(el),
      id = el.identifier
    })
  end
end

function Pandoc(doc)
  if #objects == 0 then return doc end

  -- Sort alphabetically (case-insensitive)
  table.sort(objects, function(a, b)
    return a.text:lower() < b.text:lower()
  end)

  -- Build the index list
  local items = {}
  for _, obj in ipairs(objects) do
    local link = pandoc.Link(obj.text, "#" .. obj.id)
    table.insert(items, {pandoc.Plain({link})})
  end

  -- Append as a new top-level section
  table.insert(doc.blocks, pandoc.Header(1, "Object Index", pandoc.Attr("object-index")))
  table.insert(doc.blocks, pandoc.BulletList(items))

  return doc
end
