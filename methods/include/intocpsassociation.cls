%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% intocpsassociation.cls
% Template for INTO-CPS Report (e.g. deliverable, tech note)
% Peter Gorm Larsen, 2014
% Adapted From intocps Report Template, Richard Payne, 2011
% Doc History page from DESTECS stylem, 2011
%
% USAGE:
% \documentclass[a4paper, 12pt, twoside]{intocpsassociation}
% \reporttitle{Report on xxxx}
% \status{Public Document}
% \submissiondate{31 January 2015}
% \contributors{
%    John Fitzgerald & NCL \\
%    Peter Gorm Larson & AU
% }
% \editors{
%    John Fitzgerald & NCL \\
%    Peter Gorm Larson & AU
% }
% \reviewers{John Fitzgerald & NCL \\
%    Peter Gorm Larson & AU}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\NeedsTeXFormat{LaTeX2e}\relax
\ProvidesClass{include/intocpsreport}[2018/02/26 INTO-CPS Association Report Template]



%%Add [final] option to the report class to switch between draft and final version of the report
\newif\iffinalversion						%Define new if
\finalversionfalse							%Default value is false
\DeclareOption{final}{\finalversiontrue}	%Set to true if final option is given
% Add [lineno] option to the report class to add line numbers to each text line
\newif\iflineno								%Define new if
\linenotrue                                  %Default value is true
\DeclareOption{lineno}{\linenotrue}			%Set to true if option is given
% Add [narrowmargin] option to the report class to use narrow margins
\newif\ifnarrowmargin								%Define new if
\narrowmarginfalse                                 %Default value is true
\DeclareOption{narrowmargin}{\narrowmargintrue}			%Set to true if option is given


%%%% Load class article and redefine \maketitle after
%%%% declaring all the options
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions\relax
\LoadClass{article}

%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Requiring Packages%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%


%\RequirePackage[T1]{fontenc}
\RequirePackage{ifthen}

%%%% Require package for display URL; use {hyperref} instead of {url} %%%%
\RequirePackage{hyperref}

%%%% Require graphicx for various images %%%%
\RequirePackage{graphicx}

%%%% Require color %%%%
\RequirePackage[RGB]{xcolor}

%%%% Require fancy header %%%%
\RequirePackage{fancyhdr}

%%%% Require table colour package for version history table %%%%
\RequirePackage{colortbl}

\RequirePackage{tabularx}	% Adds possibility to maximize the width of columns (and enables word wrap)


\renewcommand{\sfdefault}{phv}

%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Conditional package includes dependent on the class options
%%%%%%%%%%%%%%%%%%%%%%%%%%%
\iffinalversion
	% Process final version stuff
\else
	% Process options for the draft mode
	\iflineno
		\RequirePackage{lineno}	% Show line numbers for reviewing purposes (Ubuntu: texlive-humanities package)
		\linenumbers[1]
	\fi % lineno
\fi % finalversion
\ifnarrowmargin
	% Process narrow margins
\usepackage[left=1.0in, right=1.0in, top=1.0in, bottom=1.0in]{geometry}
\fi % narrowmargins


%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Defining variables%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%% Various variables %%%%
%\newcommand{\intocps@reportnumber}{Document Number}
%\newcommand{\reportnumber}[1]{\renewcommand{\intocps@reportnumber}{#1}}
\newcommand{\intocps@reporttitle}{Document Title}
\newcommand{\reporttitle}[1]{\renewcommand{\intocps@reporttitle}{#1}}
\newcommand{\intocps@shortreporttitle}{Short Document Title}
\newcommand{\shortreporttitle}[1]{\renewcommand{\intocps@shortreporttitle}{#1}}
\newcommand{\intocps@reportstatus}{Document Status}
\newcommand{\reportstatus}[1]{\renewcommand{\intocps@reportstatus}{#1}}
\newcommand{\intocps@submissiondate}{Submission date}
\newcommand{\submissiondate}[1]{\renewcommand{\intocps@submissiondate}{#1}}

%%%%% Optional variables %%%%%
\newcommand{\intocps@editors}{Document Editors}
\newcommand{\editors}[1]{\renewcommand{\intocps@editors}{#1}}
\newcommand{\intocps@contributors}{Document Contributors}
\newcommand{\contributors}[1]{\renewcommand{\intocps@contributors}{#1}}
%\newcommand{\intocps@reviewers}{Document Reviewers}
%\newcommand{\reviewers}[1]{\renewcommand{\intocps@reviewers}{#1}}

%%%% Determine whether deliverable or technote %%%%
%%%% (from KGP Destecs addition, 2010/10/06) %%%%
\newif\iftechnote
% default to original behaviour
\technotefalse
%\newcommand{\doctype}{\iftechnote{Technical Note}\else{Deliverable}\fi}


%%%%%%%%%%%%%%%%%%%%%%
%%%% Style things %%%%
%%%%%%%%%%%%%%%%%%%%%%


%%%% Make paragraphs start on new line, with no indentation %%%%
\RequirePackage[parfill]{parskip}

%%%% Set depth of table of contents %%%%
\setcounter{tocdepth}{2}


%%%% Set change log details %%%%
\def\@changelog{\relax}
% #1: version
% #2: date
% #3: author
% #4: description
\def\addversion#1#2#3#4{
\expandafter\def\expandafter\@changelog\expandafter{\@changelog
#1 & #2 & #3 & #4 \\\hline
}
\def\intocps@docversion{#1}
}



%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Defining Header %%%%
%%%%%%%%%%%%%%%%%%%%%%%%%

 \pagestyle{fancy}
 %Left side of header - include doc number, short title and report status
 \lhead{\sffamily \intocps@shortreporttitle\ (\intocps@reportstatus)}
 \rhead{\includegraphics[height=0.4cm]{include/intocps-logo-horiz}} %intocps logo
 \setlength{\headheight}{15pt}
 \renewcommand{\headrulewidth}{0.5pt} %solid line

 \clearpage


%%%%%%%%%%%%%%%%%%%%%%%
%%%% Document Info %%%%
%%%%%%%%%%%%%%%%%%%%%%%

%% Change 'titlepage' into a page containing the document information
\def\maketitle{
\createdocinfo
}

\def\createdocinfo{
 %%%% Title page%%%%
 \thispagestyle{empty}
 \begin{center}
   \sffamily
  % \includegraphics[width=0.8\textwidth]{include/horizon2020-logo}\\
  %   \vfill
   %{\large Project: INTO-CPS}\\[2ex]
%   {\large Grant Agreement: 644047}\\[2ex]
   {\large INtegrated TOol chain for model-based design of CPSs}\\
     \vfill
   \includegraphics[width=.5\textwidth]{include/intocps-logo}
     \vfill
   {\textbf{\Large \intocps@reporttitle}}\\[4ex]
%   {\large \doctype\ Number: \intocps@reportnumber}\\[2ex]
   {\large Version: \intocps@docversion}\\[2ex]
   {\large Date: \intocps@submissiondate}%\\[4ex]
%   {\large \intocps@reportstatus\ Document}
     \vfill
   The INTO-CPS Association\\[2ex]
   {http://into-cps.org}
 \end{center}
 \clearpage


 %%%% Contributors etc
 \ifx\intocps@contributors\@empty
 \else
   \textbf{\large Contributors:}

   \bigskip
     \intocps@contributors
 \fi

 \ifx\intocps@editors\@empty
 \else
 \bigskip
 \bigskip
   \textbf{\large Editors:}

   \bigskip
     \intocps@editors
 \fi

% \ifx\intocps@reviewers\@empty
% \else
% \bigskip
%   \textbf{\large Reviewers:}
%
%   \bigskip
%     \intocps@reviewers
% \fi

\vfill
\copyright The INTO-CPS Association
%\textbf{\large Consortium:}
%\begin{table*}[!h]
%\centering
%\begin{tabular}{| l | l | l | l |}
%\hline
%Aarhus University & AU & Newcastle University & UNEW \\
%\hline
%University of York & UY & Link\"{o}ping University & LIU \\
%\hline
%Verified Systems International GmbH & VSI & Controllab Products & CLP \\
%\hline
%ClearSy & CLE & TWT GmbH & TWT \\
%\hline
%Agro Intelligence & AI & United Technologies & UTRC \\
%\hline
%Softeam & ST &  & \\
%\hline
%\end{tabular}
%\end{table*}%
 \clearpage


%%%% Document history page %%%%
 \vskip0.5cm
 \if\@changelog\relax\else

 \section*{Document History}
 \begin{tabularx}{\linewidth}[h!]{| l | l | l | X |}
 \rowcolor[gray]{.8}\hline
 \textbf{Ver} & \textbf{Date} & \textbf{Author} & \textbf{Description} \\\hline
 \@changelog
 \end{tabularx}
 \vskip0.5cm
 \clearpage
 \fi
}
