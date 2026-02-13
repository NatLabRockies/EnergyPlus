# Single page
# pandoc \
#   --to=html5 \
#   --mathml \
#   --standalone=true \
#   --table-of-contents=true \
#   --output=input-output-reference-bootstrap.html \
#   --template template.html \
#   --css=style.css \
#   --include-in-header header.html \
#   --include-before-body navbar.html \
#   --include-after-body footer.html \
#   --lua-filter=bootstrap-tables.lua \
#   --lua-filter=object-index.lua \
#   input-output-reference.tex
#
# Multi page
# rm -Rf chunked_default/
#
# pandoc \
#   --to=chunkedhtml \
#   --mathml \
#   --standalone=true \
#   --table-of-contents=true \
#   --split-level=2 \
#   --output=chunked_default \
#   --number-sections=false \
#   input-output-reference.tex

rm -Rf chunked/

# header/footer/navbar inlined in the template already
pandoc --to=chunkedhtml \
  --mathml \
  --standalone=true \
  --table-of-contents=true \
  --split-level=2 \
  --output=chunked \
  --template template_chunked.html \
  --css=style.css \
  --number-sections=false \
  --include-in-header header.html \
  --include-after-body footer.html \
  --lua-filter=bootstrap-tables.lua \
  --lua-filter=object-index.lua \
  input-output-reference.tex

# Build search index from sitemap (levels 2 & 3 only)
python3 -c "
import json, pathlib
def collect(node, out):
    s = node['section']
    if int(s['level']) in (2, 3):
        out.append({'t': s['title'], 'p': s['path']})
    for sub in node.get('subsections', []):
        collect(sub, out)
idx = []
collect(json.loads(pathlib.Path('chunked/sitemap.json').read_text()), idx)
pathlib.Path('chunked/search-index.json').write_text(json.dumps(idx))
"

# Copy assets that pandoc doesn't copy for chunked output
cp style.css chunked/
cp ../../release/ep_nobg.png chunked/media/
