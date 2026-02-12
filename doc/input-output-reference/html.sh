# Single page
pandoc \
  --to=html5 \
  --mathml \
  --standalone=true \
  --table-of-contents=true \
  --output=input-output-reference-bootstrap.html \
  --template template.html \
  --css=style.css \
  --include-in-header header.html \
  --include-before-body navbar.html \
  --include-after-body footer.html \
  --lua-filter=bootstrap-tables.lua \
  input-output-reference.tex

# Multi page
rm -Rf chunked_default/

pandoc \
  --to=chunkedhtml \
  --mathml \
  --standalone=true \
  --table-of-contents=true \
  --split-level=2 \
  --output=chunked_default \
  --number-sections=false \
  input-output-reference.tex


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
  --include-after-body footer.html \
  --lua-filter=bootstrap-tables.lua \
  input-output-reference.tex

# Copy assets that pandoc doesn't copy for chunked output
cp style.css chunked/
cp media/ep_nobg.png chunked/media/
