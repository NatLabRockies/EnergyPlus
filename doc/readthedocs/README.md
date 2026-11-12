# Read the Docs development guide

This README covers building, contributing to, and publishing the EnergyPlus documentation site.
For the reader-facing documentation, visit the
[EnergyPlus documentation home](https://energyplus.readthedocs.io/en/latest/).

The site combines Sphinx documentation, Doxygen-generated C API documentation, and LaTeX user
guides converted to HTML with Pandoc.

## Building the Doxygen (C) Documentation
The C documentation is built with Doxygen, which is available on Debian with `apt install doxygen`.
To build the docs, you can just run `doxygen` in the `doc/readthedocs/doxygen/` folder
Then you can browse the built docs at: `doc/readthedocs/sphinx/static/c_prebuilt/index.html`.

## Building the Sphinx (Python) Documentation
The Sphinx documentation is essentially the main documentation engine, since it is what gets executed by ReadTheDocs.
Install the pinned Read the Docs dependency stack first (`python3 -m pip install -r doc/readthedocs/requirements.txt`).
Also, this process will build the Doxygen based C documentation, so you'll need that installed as well.
Move into the Python API doc folder: `cd doc/readthedocs/sphinx/`.
Run the Sphinx built Makefile: `make html`.
Browse the built docs at: `doc/readthedocs/sphinx/_build/html/index.html`

To build the complete local Read the Docs site, including the LaTeX guides converted via Pandoc,
run `make -j $(nproc) guides`. The target builds Sphinx first so the guide landing pages are copied last.

### Previewing the site locally

From the repository root, build the complete site and start a local HTTP server:

```bash
make -C doc/readthedocs/sphinx -j 23 guides
python3 -m http.server 8000 --directory doc/readthedocs/sphinx/_build/html
```

Open <http://127.0.0.1:8000/> in a browser. Keep the terminal running while viewing the site and
press `Ctrl+C` to stop the server. After changing the documentation, rerun the `make` command and
refresh the browser.

## Adding Documentation
The Python and C API documentation are generated directly from the API sources (.py files and .h files) in the api source directory: `src/EnergyPlus/api`.
- To edit the documentation, just edit the code there and rebuild the documentation.
- To edit the C API documentation configuration, just edit the `doc/readthedocs/doxygen/Doxyfile` configuration file and rebuild.
- To edit the Python API documentation configuration, just edit the `doc/readthedocs/sphinx/conf.py` configuration file and rebuild.
- To add more content to the documentation that is hosted on ReadTheDocs, just add more `rst` sources or content in the `doc/readthedocs/sphinx` directory and make sure to update the root `doc/readthedocs/sphinx/index.rst` file, then rebuild.

## Updating ReadTheDocs
ReadTheDocs will build "latest" and "stable" versions of EnergyPlus commits by default.
[`latest`](https://energyplus.readthedocs.io/en/latest/) always points to the latest commit made to the `develop` branch, almost always due to a pull request merge.
[`stable`](https://energyplus.readthedocs.io/en/stable/) always points to the last release tag made to EnergyPlus.

All major release versions of EnergyPlus should be added as well.
To accomplish this, simply go into ReadTheDocs [version configuration](https://readthedocs.org/projects/energyplus/versions/) and "Activate a Version".
This will trigger a build of that tag.  Make sure that the build is labeled as "active" and not "hidden", to make sure it is included in the main list of official versions.

In a similar way, if a branch is specifically modifying the documentation, and it is desired to get ReadTheDocs building commits to that branch, just activate that branch but keep it hidden.
The developer will still be able to see the build results, but they won't be advertised in the main list of official release documentation versions.
