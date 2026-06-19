function Table(el)
  el.attr.classes:insert("table")
  el.attr.classes:insert("table-bordered")
  el.attr.classes:insert("table-striped")
  el.attr.classes:insert("table-sm")
  return el
end
