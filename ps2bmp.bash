#!/usr/bin/bash
## #############################################################
## Convertion of postscript 
## script for converting ps files to other format.
##
## Modification of Paul Pettersson's eps2bmp script
##
## NB: frozen versions are
##     ps2bmp-2.0
##     ps2bmp-2.3
##     ps2bmp-2.4
##
## Synopsis:
##  Getting around nasty ps bugs.
## #############################################################
## @FILE:    ps2bmp
## @PLACE:   BRICS AArhus; host:harald
## @FORMAT:  bash script
## @AUTHOR:  M. Oliver M'o'ller     <omoeller@verify-it.de>
## @BEGUN:   Fri Sep 15 11:45:42 2000
## @VERSION: V2.5                       Thu Jan 10 17:15:12 2002
## #############################################################
## 
SHELL=/usr/local/bin/bash
## -- elementaries --------------------------------------------------------
## 
## Shell settings
INFILE=""
OUTFILE=""
FORMAT="bmp256"
SCALE="1";
QUIET=false;
VERBOSE=false;
LANDSCAPE_PAPER=false;
TO_STDOUT=false;
CLIPPING_IMAGE=false;
FATTEN_FACTOR="";
FATTEN_GRANULARITY="0.9";
ONLY_VERTICAL=false;
WORKING_DIR="/tmp";
TMP_NAME="tmp.bs2bmp.37876.x.$$";
PRESERVE_TMPFILES=false;
BACKSLASH="\\"  
VERSION_NUMBER="V2.5 ";
VERSION_DATE="Thu Jan 10 17:15:12 2002" ;
VERSION="${VERSION_NUMBER}${VERSION_DATE}";
## -- WORKAROUNDS ----------------------------------------------------------
if [ "${LOCATION}" == "uu" ]; then
  BACKSLASH="\\\\";
fi;
## --------------------
function clip_ps_file { ## -- compute bounding box -------------------------
  if [ "${LOCATION}" == "acer" ]; then
    ps2epsi $1 $2;
  else
    epstool -c -b  -o$2 $1;
  fi;
}
## -- other functions ------------------------------------------------------
function print_usage()
{
  echo "ps2bmp  ($VERSION)";
  echo "";
  echo "usage:  ps2bmp [hvclp] [-f FORMAT] [-s SCALE] [-F FACTOR] [-G GRANULARITY] [-w DIR] INFILE [OUTFILE]"
  echo "        -h             : print help"
  echo "        -v             : verbose on"
  echo "        -c             : channel output to stdout"
  echo "        -l             : assume landscape paper"
  echo "        -p             : preserve temporary files"
  echo "        -f FORMAT      : set format; default: ${FORMAT}"
  echo "        -s SCALE       : set scale to factor";
  echo "        -C             : clip image (i.e. reduce to bounding box)";
  echo "        -F FATTEN      : fatten up the picture FATTEN=1,2,3,4,5";
  echo "        -G GRANULARITY : pixel granularity for fattening (default: $FATTEN_GRANULARITY)";
  echo "        -V             : only vertical fattening";
  echo "        -q             : process quietly";
  echo "        -W DIR         : working DIR for temporary files (default: $WORKING_DIR)";
  echo "        --version      : print version"
}
function print_help()
{
  print_usage
  echo "";
  echo "description:";
  echo " Translates postscript files into other formats.";
  echo " (see gs --help to find out more about possible formats)";
  echo " If -f ps is set, the input is untouched (except fattening).";
  echo " If scaling is used, points are taken from the lower right edge,";
  echo " ignoring the bounding box (since gs cannot handle this).";
  echo " If -l is set, the width and height of the output are swapped.";
  echo " Fattening is useful, if a picture should be translated to bitmap formats";
  echo " later, where the lines should be bigger in order not to vanish when";
  echo " the bitmap is scaled down. However, this is slow and creates very big";
  echo " postscript files as intermediate results. Try to keep -F small and play";
  echo " with -G instead.";
  echo " If fattening is used and format is not ps, then the image is clipped";
  echo " automatically.";
  echo " If -V is set, fattening works only vertically.";
  echo " (thanks to Donald Arseneau for a tex hint here.)";
  echo "";
  echo "relies on:  pstops psnup gs bc grep sed "
  echo "            latex [geometry epsfig fancybox]"
  echo "            ps2epsi OR epstool (for clipping)"
  echo "";
  echo "author:     M. Oliver M\"oller  <omoeller@verify-it.de>";
  exit 1;
}
## --
function set_in_out_files() 
{
if [ -z "$INFILE" ]; then 
  INFILE="$switch"; 
else 
  if [ -z "$OUTFILE" ]; then 
    OUTFILE=$switch; 
  else
    print_help; 
  fi;  
fi;
}
function abort { ## -- clear the temp files --
  echo "ps2bmp: ERROR: $1" 1>&2;
  echo "** ps2bmp aborted." 1>&2;
  echo "** consider:  rm ${WORKING_DIR}/${TMP_NAME}.*" 1>&2;
  exit 1;
}
function gettheending { ## $1: name $2: desired ending
  LENGTH=${#1}
  POS=$(($LENGTH-${#2}));
  if [ $POS -lt 0 ]; then
    POS=0
  fi;
  RESULT="*error*";
  NOTDONE=true;
  while($NOTDONE)do
    RESULT="${1:0:$POS}$2";
    if [ "${RESULT:0:$LENGTH}" == "$1" ]; then
      NOTDONE=false;
    fi;
    POS=$(($POS+1));
  done;  
  echo $RESULT
}
function local_stripoffsuffix() { ## remove last suffix (if present)
  if [ -z "$(echo ${1} |grep  "\\.")" ]; then
    echo $1;
  else
    POS=${#1};
    while [ "${1:${POS}:1}" != "." ]; do
      POS=$((${POS}-1));
    done;
    echo ${1:0:${POS}};
  fi;
}
function compute { ## scales
  echo "0.0+($1)*${SCALE}" | bc -l | cut -d'.' -f 1;
}
function create_latex_file {
  echo -n "" > ${WORKING_DIR}/${TEX_FILE}
  echo "\\documentclass{article}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "%\\usepackage{fullpage}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\usepackage[a4paper,hmargin=2cm,vmargin=2cm]{geometry} % -- fixed margins" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "%\\usepackage{graphicx}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\usepackage{epsfig}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\usepackage{fancybox}" >> ${WORKING_DIR}/${TEX_FILE};
  echo "%%\\usepackage{pstricks}" >> ${WORKING_DIR}/${TEX_FILE};
  echo "\\pagestyle{empty}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\setlength{\\parindent}{0mm}" >> ${WORKING_DIR}/${TEX_FILE};
  echo "\\listfiles" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}begin{document}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\def${BACKSLASH}fatten#1{%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\setbox0\\hbox{#1}% ELSE" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\leavevmode ${BACKSLASH}copy0\\kern-\\wd0 " >> ${WORKING_DIR}/${TEX_FILE} ;
  if $ONLY_VERTICAL; then ## -- only up/down movements ---------------------
    echo "%% only vertical shifts "  >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "${BACKSLASH}raise${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0 " >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "${BACKSLASH}lower${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0 " >> ${WORKING_DIR}/${TEX_FILE} ;
##    echo "\\kern.3\\pix ${BACKSLASH}box0 }" >> ${WORKING_DIR}/${TEX_FILE} ;
    echo " ${BACKSLASH}box0 }" >> ${WORKING_DIR}/${TEX_FILE} ;
  else ## -- move to the corners -------------------------------------------
    echo "%% horizontal and vertical shifts "  >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "\\kern.3\\pix ${BACKSLASH}raise${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0 " >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "\\lower${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0 " >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "\\kern.4\\pix ${BACKSLASH}raise${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0" >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "\\lower${FATTEN_GRANULARITY}\\pix ${BACKSLASH}copy0\\kern-${BACKSLASH}wd0" >> ${WORKING_DIR}/${TEX_FILE} ;
    echo "\\kern.3\\pix ${BACKSLASH}box0 }" >> ${WORKING_DIR}/${TEX_FILE} ;
  fi; ## -------------------------------------------------------------------
  echo "${BACKSLASH}newdimen\\pix \\pix=.01in" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "%%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}begin{center}" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}vspace*{110mm}%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}boxput{%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "\\hbox{%" >> ${WORKING_DIR}/${TEX_FILE} ;
  I=1;
  while [ $I -le ${FATTEN_FACTOR} ]; do
    echo -n "${BACKSLASH}fatten{" >> ${WORKING_DIR}/${TEX_FILE} ;
    I=$((${I}+1));
  done;
  echo -n "${BACKSLASH}epsfig{file=${FATTEN_INPUT}" >> ${WORKING_DIR}/${TEX_FILE} ;
  I=1;
  while [ $I -le ${FATTEN_FACTOR} ]; do
    echo -n "}" >> ${WORKING_DIR}/${TEX_FILE} ;
    I=$((${I}+1));
  done;
  echo "}}{}}{}%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}end{center}%" >> ${WORKING_DIR}/${TEX_FILE} ;
  echo "${BACKSLASH}end{document}" >> ${WORKING_DIR}/${TEX_FILE} ;
}
## -- reading options --------------------
AWAIT="";
for switch in $@; do
  case $AWAIT in
	    -f ) AWAIT=""; FORMAT=$switch;;
	    -s ) AWAIT=""; SCALE=$switch;;
	    -F ) AWAIT=""; FATTEN_FACTOR=$switch;;
	    -G ) AWAIT=""; FATTEN_GRANULARITY=$switch;;
	    *  ) case $switch in
	     --help ) print_help;;
	      -help ) print_help;;
	         -h ) print_help;;
	         -f ) AWAIT="-f";;
	         -s ) AWAIT="-s";;
	         -F ) AWAIT="-F";;
	         -G ) AWAIT="-G";;
	         -v ) VERBOSE=true;;
	         -l ) LANDSCAPE_PAPER=true;;
	         -p ) PRESERVE_TMPFILES=true;;
	         -c ) TO_STDOUT=true;;
	         -C ) CLIPPING_IMAGE=true;;
	         -V ) ONLY_VERTICAL=true;;
	         -q ) QUIET=true;;
          --version ) echo "Version: $VERSION";exit 1;;
		 -* ) echo "Unknown switch: $switch";print_usage;exit 1;;
                 *  ) set_in_out_files;; 
	         esac
  esac
done;
## -- propagate -------------------------
if ${TO_STDOUT}; then
  VERBOSE=false;
  QUIET=true;
fi;
if [ -z "$INFILE" ]; then
  echo "missing INFILE.";
  print_usage;
  exit 1;
fi
ORIG_INFILE=$INFILE;
## try to find .ps or .eps endings: -----
if [ ! -e $INFILE ]; then
  INFILE=$(gettheending $ORIG_INFILE ".ps");
fi
if [ ! -e $INFILE ]; then
  INFILE=$(gettheending $ORIG_INFILE ".eps");
fi
if [ ! -e $INFILE ]; then
  abort "Cannot find input file $ORIG_INFILE[.ps,.eps]";
fi
## --------------------------------------
if $TO_STDOUT; then
  if ${VERBOSE}; then
    echo "** channeling to   stdout";
  fi;
else
  if [ -z "$OUTFILE" ]; then
    OUTFILE=$(local_stripoffsuffix "${INFILE}");
    OUTFILE=${OUTFILE}.bmp;
    if ${VERBOSE}; then
      echo "** using outfile ${OUTFILE}";
    fi;
  fi
fi
TMP_FILE=${TMP_NAME}.tmp-file-for-ps2bmp.$$.ps;
TMP_FILE_2=${TMP_NAME}.tmp-file-for-ps2bmp.$$.to_clip.ps;
if [ -e ${WORKING_DIR}/${TMP_FILE} ]; then
  abort "Sorry, temp file ${WORKING_DIR}/${TMP_FILE} exists.";
fi;
if [ -e ${WORKING_DIR}/${TMP_FILE_2} ]; then
  abort "temporary file ${WORKING_DIR}/${TMP_FILE_2} already exits. Sorry.";
else
  echo "" > ${WORKING_DIR}/${TMP_FILE_2};
fi;
## -- GET BOUNDING BOX -----------------------------------------------------

## -- START PROCESSING -----------------------------------------------------

if [ -z "${FATTEN_FACTOR}" ]; then ## -- NO FATTENING ----------------------
  if ${CLIPPING_IMAGE}; then
    clip_ps_file ${INFILE} ${WORKING_DIR}/${TMP_FILE_2};
  else
    cp ${INFILE} ${WORKING_DIR}/${TMP_FILE_2};
  fi;
  SIZE_X=$(grep "^%%BoundingBox: " ${WORKING_DIR}/${TMP_FILE_2} | cut -d' ' -f 2 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_X; do SIZE_X=$onlyfirst;break;
  done;
  SIZE_Y=$(grep "^%%BoundingBox: " ${WORKING_DIR}/${TMP_FILE_2} | cut -d' ' -f 3 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_Y; do SIZE_Y=$onlyfirst;break;
  done;
  SIZE_XX=$(grep "^%%BoundingBox: " ${WORKING_DIR}/${TMP_FILE_2} | cut -d' ' -f 4 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_XX; do SIZE_XX=$onlyfirst;break;
  done;
  SIZE_YY=$(grep "^%%BoundingBox: " ${WORKING_DIR}/${TMP_FILE_2} | cut -d' ' -f 5 | sed -e "s/[^0123456789]//g" );
  for onlyfirst in $SIZE_YY; do SIZE_YY=$onlyfirst;break;
  done;

  HEIGHT=$((${SIZE_YY}-${SIZE_Y}));
  WIDTH=$((${SIZE_XX}-${SIZE_X}));

  cat ${WORKING_DIR}/${TMP_FILE_2} |grep -v "^%%BoundingBox: " | pstops "0(-${SIZE_X},-${SIZE_Y})" > ${WORKING_DIR}/${TMP_FILE}
else ## -- APPLY FATTENING: OTHER TMP_FILE ---------------------------------
  if ! ${QUIET}; then
    echo "** using fattening.";
  fi;
  FATTEN_INPUT="$TMP_NAME.input.ps";
  if cat ${INFILE} | sed "s/^%%BoundingBox:.*$/%%BoundingBox: 0 0 596 842/g" > ${WORKING_DIR}/${FATTEN_INPUT}; then
    :
  else
    abort "copy failed";
  fi;
  TEX_FILE="$TMP_NAME.tex";
  create_latex_file;
  ## -- compile and modify ----------------------------
  pushd ${WORKING_DIR} >/dev/null;
  if ${QUIET}; then
    if ! latex $TEX_FILE >/dev/null; then
      abort "latex failed; missing package?";
    fi;
    if ! dvips -o${TMP_NAME}.ps  ${TMP_NAME}.dvi >/dev/null 2>/dev/null; then
      abort "dvips failed.";
    fi;
  else
    if ! latex $TEX_FILE; then
      abort "latex failed; missing package?";
    fi;
    if ! dvips -o${TMP_NAME}.ps  ${TMP_NAME}.dvi; then
      abort "dvips failed.";
    fi;
  fi;
  ## -- exception: for ps, don't clip ---------------------------
  if [ "$FORMAT" == "ps" ]; then
    cp ${TMP_NAME}.ps ${TMP_NAME}.2.ps;
  else
    clip_ps_file ${TMP_NAME}.ps ${TMP_NAME}.2.ps;
  fi;
  ## ------------------------------------------------------------
  SIZE_X=$(grep "^%%BoundingBox: " ${TMP_NAME}.2.ps | cut -d' ' -f 2 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_X; do SIZE_X=$onlyfirst;break;
  done;
  SIZE_Y=$(grep "^%%BoundingBox: " ${TMP_NAME}.2.ps | cut -d' ' -f 3 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_Y; do SIZE_Y=$onlyfirst;break;
  done;
  SIZE_XX=$(grep "^%%BoundingBox: " ${TMP_NAME}.2.ps | cut -d' ' -f 4 | sed -e "s/[^0123456789]//g");
  for onlyfirst in $SIZE_XX; do SIZE_XX=$onlyfirst;break;
  done;
  SIZE_YY=$(grep "^%%BoundingBox: " ${TMP_NAME}.2.ps | cut -d' ' -f 5 | sed -e "s/[^0123456789]//g" );
  for onlyfirst in $SIZE_YY; do SIZE_YY=$onlyfirst;break;
  done;
  ## -- allow some slight aberration: unneccessary ---
  #  SIZE_X=$((${SIZE_X}-2));  	  
  #  SIZE_Y=$((${SIZE_Y}-2));  	  
  #  SIZE_XX=$((${SIZE_XX}+2));  	  
  #  SIZE_YY=$((${SIZE_YY}+2));
  ## --
  HEIGHT=$((${SIZE_YY}-${SIZE_Y}));
  WIDTH=$((${SIZE_XX}-${SIZE_X}));
  ##---
  cat ${TMP_NAME}.2.ps| grep -v "^%%BoundingBox: " | pstops "0(-${SIZE_X},-${SIZE_Y})" > ${WORKING_DIR}/${TMP_FILE}

  ## --  remove temp files ----------------------------
  popd >/dev/null;
fi;
## ----------------------------------------------------
## --spam ------------------------------------------------------------------
if $VERBOSE; then
  echo "INFILE:             $INFILE"
  echo "OUTFILE:            $OUTFILE"
  echo "FORMAT:             $FORMAT";
  echo "TO_STDOUT:          $TO_STDOUT"
  echo "LANDSCAPE_PAPER:    $LANDSCAPE_PAPER";
  echo "TMP_FILE:           $TMP_FILE"
  echo "SIZE_X:             $SIZE_X"
  echo "SIZE_Y:             $SIZE_Y"
  echo "SIZE_XX:            $SIZE_XX"
  echo "SIZE_YY:            $SIZE_YY"
  echo "HEIGHT:             $HEIGHT"
  echo "WIDTH:              $WIDTH";
  echo "SCALE:              $SCALE";
  echo "FATTEN_FACTOR:      $FATTEN_FACTOR";
  echo "FATTEN_GRANULARITY: $FATTEN_GRANULARITY";
  echo "ONLY_VERTICAL:      $ONLY_VERTICAL";
  echo "WORKING_DIR:        $WORKING_DIR";
  echo "VERSION:            $VERSION"
fi;
## -- Sanity Checks --------------------------------------------------------
if [ "$SIZE_Y" == "" ]; then
  abort "No bounding box found in ${INFILE}.";
fi
## -------------------------------------------------------------------------
## -- special case: format ps ----------------------------------------------
## -------------------------------------------------------------------------
if [ "$FORMAT" == "ps" ]; then
  if $VERBOSE; then
    echo "** SPECIAL CASE: ps output";
  fi;
  if [ -z "$SCALE" ]; then
    SCALE="1";
  fi;
  LANDSCAPE_SWITCH="";
  if $LANDSCAPE_PAPER; then
    LANDSCAPE_SWITCH="-l";
  fi;
  if ${TO_STDOUT}; then
    COMMAND_ARGS="-s${SCALE} ${LANDSCAPE_SWITCH} ${WORKING_DIR}/${TMP_FILE} ";
  else
    COMMAND_ARGS="-s${SCALE} ${LANDSCAPE_SWITCH} ${WORKING_DIR}/${TMP_FILE} ${OUTFILE}" ;
  fi;
  if $VERBOSE; then
    echo "** psnup ARGS   : ${COMMAND_ARGS}";
  fi;
  if psnup ${COMMAND_ARGS};  then
    :
  else
    abort "psnup failed";
  fi;
  ## -----------------------------------------------------------------------
else
  ## -- ORDINARY FORMAT ----------------------------------------------------
  ## -- compute scale ------------------------------------------------------
  DPI=72;
  RESOLUTION_ARG="";
  if ! [ -z "$SCALE" ]; then
    BASE_DPI=72;
    WIDTH=$(compute ${WIDTH});
    HEIGHT=$(compute ${HEIGHT});
    DPI=$(compute 72);
    RESOLUTION_ARG="-r${DPI}x${DPI}";
  fi;
  if ${VERBOSE}; then
    echo "** using output size ${WIDTH}x${HEIGHT}, dpi: ${DPI}"
  fi
  if  [ -z ${WIDTH} ] || [ -z ${HEIGHT} ] || [ -z ${DPI} ] || \
   [ ${WIDTH} -le 0 ] || [ ${HEIGHT} -le 0 ] || [ ${DPI} -le 0 ]; then
    abort "illegal dimensions: ${WIDTH}x${HEIGHT}, dpi: ${DPI}";
  fi;
  
  ## -- landscape-flip ----------------------------------
  if $LANDSCAPE_PAPER; then
    TMP=$HEIGHT;
    HEIGHT=$WIDTH;
    WIDTH=$TMP;
  fi;
  ## ----------------------------------------------------
  
  if ${TO_STDOUT}; then
    if echo "quit" | gs -q -dNOPAUSE -dQUIET -dGraphicsAlphaBits=4 -dTextAlphaBits=4 -sDEVICE=${FORMAT}  -sOutputFile=- \
     -g${WIDTH}x${HEIGHT} ${RESOLUTION_ARG} ${WORKING_DIR}/${TMP_FILE}  2>/dev/null; then
      :
    else
      abort "gs failed.";
    fi;
  else
    if echo "quit" | gs -q -dNOPAUSE -dQUIET -dGraphicsAlphaBits=4 -dTextAlphaBits=4 -sDEVICE=${FORMAT} -sOutputFile=${OUTFILE} \
     -g${WIDTH}x${HEIGHT} ${RESOLUTION_ARG} ${WORKING_DIR}/${TMP_FILE}  2>/dev/null; then
      :
    else
      abort "gs failed.";
    fi;
  fi;
fi; ## -- end of special case/ordinary case --------------------------------
## -------------------------------------------------------------------------
if ${VERBOSE}; then
  echo "** constructed:";
  ls -l ${OUTFILE};
fi
if ${PRESERVE_TMPFILES}; then
  echo "** consider:  rm ${WORKING_DIR}/${TMP_NAME}.*" 1>&2;
else
  rm ${WORKING_DIR}/${TMP_NAME}.*
fi;
echo "** ps2bmp done."
exit 0; ## --- OK ------------------------------


### Local Variables: ***
### mode: lisp ***
### eval: (defun update-global-date () (let ((pos (point-marker))) (goto-char (point-min)) (if (search-forward-regexp "^VERSION_DATE=" (point-max) t) (progn (kill-line) (insert (format "\"%s\" ;" (current-time-string))) (basic-save-buffer) (message "** Version Date Updated."))) (goto-char pos)))  ***
### eval: (defun new-global-hh-insert-disclaimer () (interactive) (insert-disclaimer) (update-global-date) (ksh-mode)(font-lock-mode) (local-set-key [f4] #'new-global-hh-insert-disclaimer)) ***
### eval: (progn (ksh-mode)(font-lock-mode) (local-set-key [f4] #'new-global-hh-insert-disclaimer)) ***
### comment-column:0 ***
### comment-start: "### "  ***
### comment-end: "***" ***
### End: ***


