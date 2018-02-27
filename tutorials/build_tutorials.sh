#!/bin/bash

set -e



find . -name "Tutorial*.tex" -maxdepth 2 -execdir bash -c 'latexmk -pdf -bibtex -f "$0"' {} \;
