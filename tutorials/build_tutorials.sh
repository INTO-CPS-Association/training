#!/bin/bash

find . -name "Tutorial*.tex" -d 2 -execdir sh -c 'latexmk `basename $1`' {} \;
