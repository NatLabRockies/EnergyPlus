pip install -r requirements.txt
sphinx-build -M html $(pwd) out
sphinx-build -b pdf  $(pwd) out
