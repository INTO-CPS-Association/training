# Usage:
# 	$ make 					- compile pdf
# 	$ make open				- compile and open pdf
# 	$ make clean 			- clean intermediate files 
# 	$ make edit 			- open all *.tex files with vimsrv servername
# 	$ make edit FILE=f.tex	- open only one file with vimsrv servername
#
# Makefile for compiling pdf file from tex file
# 
OS := $(shell uname -s)
PWD := $(shell pwd)
BNAME := D3.6_Examples_Compendium_3
ADD_TEXS := fcu/fcu.tex linefollower/linefollowrobot.tex uavswarm/uavswarm.tex ether/ether.tex
PDF := $(strip $(BNAME)).pdf #$(patsubst %.tex,%.pdf, $(wildcard *.tex))

ifeq ($(OS), Darwin)
OS := OSX 
TEXS := $(foreach f, $(wildcard *.tex), $(PWD)/$(f))
TEXS += $(ADD_TEXS)
#EDIT := /Applications/MacVim.app/Contents/MacOS/MacVim --servername vimsvr -g -f -p --remote-silent $(TEXS)
### open multi tabs and each tab for each file
EDIT := mvim -p --servername vimsvr -g -f $(TEXS)
### open one tab for all files
#EDIT := mvim --servername vimsvr -g -f $(TEXS)
OPEN := open
endif

ifeq ($(OS), Linux)
	OS := Linux 
endif

ifneq (,$(findstring CYGWIN_NT, $(OS)))
# CYGWIN_NT
# Convert cygwin path to Windows path, and replace one \ with two \\ 
WIN_PWD := $(shell cygpath -w ${PWD} | sed -e 's/\\/\\\\/g')
TEXS := $(foreach f, $(wildcard *.tex), $(WIN_PWD)\\$(f))
TEXS += $(ADD_TEXS)
PYTHON27 := C:\\python27
GVIM := c:\\MyPrograms\\gvim73\\vim73\\gvim.exe
OS := CYGWIN_NT
OPEN := cygstart.exe
endif

ifneq (,$(findstring MINGW64_NT, $(OS)))
#WIN_PWD := $(shell cygpath -w ${PWD} | sed -e 's_\\_\\\\_')
WIN_PWD := $(shell cygpath -w ${PWD})
TEXS := $(foreach f, $(wildcard *.tex), $(WIN_PWD)\\$(f))
TEXS += $(foreach f, $(wildcard */*.tex), $(WIN_PWD)\\$(f))
TEXS += $(ADD_TEXS)
PYTHON27 := C:\\python27
GVIM := c:\\MyPrograms\\gvim73\\vim73\\gvim.exe
OS := MINGW64_NT
OPEN := start
endif

ifeq (,$(OS))
$(error OS can not be determined)
endif

.PHONY: all show clean edit open 
all: clean $(PDF) 

show:
	@echo $(OS)

# clean 
clean: 
	-@rm -f *.aux *.lof *.log *.lot *.out *.toc *.bbl *.blg *.pdf *.nav *.snm *.vrb

edit:
ifneq (,$(findstring CYGWIN_NT, $(OS)))
	/cygdrive/c/Windows/System32/cmd.exe /C start /D $(PYTHON27) $(GVIM) -g -f -p --servername vimsvr --remote-silent $(TEXS)
endif
ifeq ($(OS), OSX)
	@echo "Edit $(EDIT) $(TEXS) &"
	@$(EDIT) &
endif
	@echo "Edit $(EDIT) $(TEXS) &"
	@$(EDIT) &

open: $(PDF)
	@$(OPEN) $< &

%.pdf: %.tex *.tex $(TEXS)
	@echo "Compiling pdf file $(basename $<).pdf from $<"
	pdflatex $< # if the exit code of this command is not 0, then exit
	-bibtex $(basename $<)  # even this command fails, continue
	-bibtex $(basename $<)
	-bibtex $(basename $<)
	-pdflatex $<
	-pdflatex $<
	-pdflatex $<

fast: *.tex $(TEXS)
	@echo "Compiling pdf file $(BNAME).pdf from $(BNAME).tex"
	pdflatex $(BNAME).tex
	@$(OPEN) $(BNAME).pdf &

pic: pic/*.png

pic/%.png: pic/%.gv
	@echo "dot -Tpng $< -o $@"
	dot -Tpng $< -o "$(basename $<).png"
