#!/bin/bash

find . -name "Tutorial*.tex" -maxdepth 2 -execdir sh -c 'latexmk `basename $1`' {} \;
