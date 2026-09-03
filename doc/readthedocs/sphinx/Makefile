# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build

# Root of the doc/ tree (source dir for the pandoc-based guides build), and where CMake
# configures/builds it -- matches the paths .readthedocs.yml's post_build steps use (doc, doc/build).
DOC_ROOT      = ../..
DOC_BUILD_DIR = $(DOC_ROOT)/build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Build the pandoc-based chunked HTML guides (Engineering Reference, I/O Reference, etc.) and
# copy them into $(BUILDDIR)/html/guides, the same layout .readthedocs.yml's post_build produces.
# Run `make html guides` for full local parity with what gets published to Read the Docs.
guides:
	cmake -S "$(DOC_ROOT)" -B "$(DOC_BUILD_DIR)" -DBUILD_HTML_DOCS:BOOL=ON
	cmake --build "$(DOC_BUILD_DIR)" --target html_docs -j 4
	mkdir -p "$(BUILDDIR)/html/guides"
	cp -r "$(DOC_BUILD_DIR)/html/." "$(BUILDDIR)/html/guides/"

.PHONY: help guides Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
