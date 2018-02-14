\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{tutorial}[2016/10/26 Class for preparing INTO-CPS tutorials]

%\DeclareOption{myoption}{%
%    do something
%}

% based on the article class
\PassOptionsToClass{a4paper,fleqn}{article}

% pass unknown options to the article class
\DeclareOption*{%
    \PassOptionsToClass{\CurrentOption}{article}%
}

% process any options
\ProcessOptions\relax

% load the article class
\LoadClass{article}

\RequirePackage{graphicx}
\RequirePackage[top=2.5cm,left=2.5cm,right=2.5cm,head=1cm,headsep=0.2cm]{geometry}
\RequirePackage{times}
\RequirePackage[scaled]{helvet}
\RequirePackage{xcolor}
\RequirePackage[pagestyles]{titlesec}
\RequirePackage{titling}
\RequirePackage{lastpage}
\RequirePackage{etoolbox}
\RequirePackage{textcomp}
\RequirePackage{xspace}
\RequirePackage{enumitem}
\RequirePackage{xcolor}
\RequirePackage{tikz}
\usetikzlibrary{decorations.markings}

\AtBeginDocument{\maketitle}

\newcommand{\logos}{%
\raisebox{0.24cm}{\resizebox{!}{0.68cm}{\includegraphics{../logo}}}%
%\hspace{1.5em}%
%\resizebox{!}{0.94cm}{\includegraphics{ncl}}%
}

\newpagestyle{main}{
  \sethead{}{}{}
  \setfoot{}{\thepage~of \pageref{LastPage}}{}
}


\newpagestyle{title}{
  \sethead{}{}{\logos}
  \setfoot{\theauthor, \thedate}{\thepage~of \pageref{LastPage}}{}
}

\titleformat{\section}
  {\sffamily\large\bfseries}
  {\thesection}{1em}{}

\setlength{\parindent}{0pt}

\makeatletter
\renewcommand{\@maketitle}{%
\newpage%
\pagestyle{main}%
\thispagestyle{title}%
\null%
\let \footnote \thanks%
{\vspace{2ex}\noindent\sffamily\huge \@title \par\vspace{-0.5ex}}%
\noindent\rule{\textwidth}{.6pt}\par%
}
\makeatother
\let\oldmaketitle\maketitle
\renewcommand{\maketitle}{\oldmaketitle\thispagestyle{title}\pagestyle{main}}

\newlist{instructions}{enumerate}{1}
\setlist[instructions]{label=Step~\arabic*.,leftmargin=*,align=left,resume}

\newenvironment{nscenter}
 {\parskip=0pt\par\nopagebreak\centering}
 {\par\noindent\ignorespacesafterend}

\newenvironment{annotation}[2][]{%
\vspace{1.5\parskip}
\begin{tikzpicture}[]%
\node[anchor=south west,inner sep=0] (image) at (0,0) {\includegraphics[#1]{#2}};%
\begin{scope}[x={(image.south east)},y={(image.north west)},style={font=\vphantom{Ag}}]%
}{%
\end{scope}%
\end{tikzpicture}%
}

\def\AnnoSep{0.02}	% Marge entre les annotations et l'image
\newcommand{\BckImg}[2][red]{%
  \node[anchor=south west,inner sep=0] (image) at (0,0) {\includegraphics[#1]{#2}};%
}%

\newcommand{\lpoint}[4][red]{%
  \draw[->,thick,#1] (0,#2)++(-\AnnoSep,0) node[anchor=east]{#3} -- (#4);%
}%
\newcommand{\rpoint}[4][red]{%
  \draw[->,thick,#1] (0,#2)++(+\AnnoSep+1,0) node[anchor=west]{#3} -- (#4);%
}%
\newcommand{\upoint}[4][red]{%
  \draw[->,thick,#1] (#2,0)++(0,\AnnoSep+1) node[anchor=south]{#3} -- (#4);%
}%
\newcommand{\dpoint}[4][red]{%
  \draw[->,thick,#1] (#2,0)++(0,-\AnnoSep) node[anchor=north]{#3} -- (#4);%
}%

\tikzset{
    o/.style={
        shorten >=#1,
        decoration={
            markings,
            mark={
                at position 1
                with {
                    \draw circle [radius=#1];
                }
            }
        },
        postaction=decorate
    },
    o/.default=2pt
}
\newcommand{\lcircle}[5][red]{%
  \draw[o={#2},thick,#1] (0,#3)++(-\AnnoSep,0) node[anchor=east]{#4} -- (#5);%
  %\node[circle,draw,thick,#1,radius=#2] (a) at (#5) {};%
  %\draw[-,thick,#1] (0,#3)++(-\AnnoSep,0) node[anchor=east]{#4} -- (a.west);%
}%
\newcommand{\rcircle}[5][red]{%
  \draw[o={#2},thick,#1] (0,#3)++(+\AnnoSep+1,0) node[anchor=west]{#4} -- (#5);%
  %\node[circle,draw,thick,#1,radius=#2] (a) at (#5) {};%
  %\draw[-,thick,#1] (0,#3)++(+\AnnoSep+1,0) node[anchor=west]{#4} -- (a.east);%
}%
\newcommand{\ucircle}[5][red]{%
  \draw[o={#2},thick,#1] (#3,0)++(0,\AnnoSep+1) node[anchor=south]{#4} -- (#5);%
  %\node[circle,draw,thick,#1,radius=#2] (a) at (#5) {};%
  %\draw[-,thick,#1] (#3,0)++(0,\AnnoSep+1) node[anchor=south]{#4} -- (a.north);%
}%
\newcommand{\dcircle}[5][red]{%
  \draw[o={#2},thick,#1] (#3,0)++(0,-\AnnoSep) node[anchor=north]{#4} -- (#5);%
  %\node[circle,draw,thick,#1,radius=#2] (a) at (#5) {};%
  %\draw[-,thick,#1] (#3,0)++(0,-\AnnoSep) node[anchor=north]{#4} -- (a.south);%
}%

\newcommand{\lsquare}[6][red]{%
  \node[draw,thick,#1,minimum width=#2,minimum height=#3] (a) at (#6) {};%
  \draw[-,thick,#1] (0,#4)++(-\AnnoSep,0) node[anchor=east]{#5} -- (a.west);%
}%
\newcommand{\rsquare}[6][red]{%
  \node[draw,thick,#1,minimum width=#2,minimum height=#3] (a) at (#6) {};%
  \draw[-,thick,#1] (0,#4)++(+\AnnoSep+1,0) node[anchor=west]{#5} -- (a.east);%
}%
\newcommand{\usquare}[6][red]{%
  \node[draw,thick,#1,minimum width=#2,minimum height=#3] (a) at (#6) {};%
  \draw[-,thick,#1] (#4,0)++(0,\AnnoSep+1) node[anchor=south]{#5} -- (a.north);%
}%
\newcommand{\dsquare}[6][red]{%
  \node[draw,thick,#1,minimum width=#2,minimum height=#3] (a) at (#6) {};%
  \draw[-,thick,#1] (#4,0)++(0,-\AnnoSep) node[anchor=north]{#5} -- (a.south);%
}

\newcommand{\helpergrid}{%
\draw[opacity=0.2,thin,xstep=0.01,ystep=0.01] (0,0) grid (1,1);%
\draw[opacity=0.4,thick,xstep=0.1,ystep=0.1] (0,0) grid (1,1);%
\foreach \x in {0,0.1,...,1.1}%
  \node[anchor=north,opacity=0.4] at (\x,0) {\pgfmathprintnumber{\x}};%
\foreach \y in {0,0.1,...,1}%
  \node[anchor=east,opacity=0.4] at (0,\y) {\pgfmathprintnumber{\y}};%
}%

\newcommand{\pathsep}{\hspace{0.2ex}/\hspace{0.2ex}}
\newcommand{\menusep}{\textgreater\xspace}
