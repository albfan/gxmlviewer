#!/bin/sh

showHelp() {
   echo "rebuild-rpm -s specfile [-M][-m][-r]"
   echo "  -s specfile : rpm spec file. Must be in root of project dir."
   echo "  -M          : increment major number"
   echo "  -m          : increment minor number"
   echo "  -r          : increment release number"
   exit 1
}

# Check Command Line Parameters
while getopts s:Mmr\? c
do
   case $c in
      s)   SPECFILE=$OPTARG;;
      M)   NEW_MAJOR="Y";;
      m)   NEW_MINOR="Y";;
      r)   NEW_RELEASE="Y";;
      \?)  showHelp;;
   esac
done
shift `expr $OPTIND - 1`

if [ "$SPECFILE" = "" ] ; then
   showHelp
fi

if [ ! -f $SPECFILE ] ; then
   echo "Spec file not found: $SPECFILE"
   exit 2
fi

specfile=$SPECFILE
name=`cat $specfile | grep Name | awk '{ print $2; }'`
version=`cat $specfile | grep Version | awk '{ print $2; }'`
majorversion=`echo $version | awk -F'.' ' { print $1; }'`
minorversion=`echo $version | awk -F'.' ' { print $2; }'`
release=`cat $specfile | grep Release | awk '{ print $2; }'`

if [  "$NEW_RELEASE" = "Y" ] ; then
   release=`expr $release + 1`
fi

if [  "$NEW_MINOR" = "Y" ] ; then
   minorversion=`expr $minorversion + 1`
   release="0"
fi

if [  "$NEW_MAJOR" = "Y" ] ; then
   majorversion=`expr $majorversion + 1`
   minorversion="0"
   release="0"
fi

source=${name}-${majorversion}.${minorversion}-${release}.tar.gz
cat $SPECFILE | sed "s%Source0.*%Source0: ${source}%g" | sed "s%Version:.*%Version: ${majorversion}.${minorversion}%g" | sed "s%Release:.*%Release: ${release}%g"> ${SPECFILE}.new
cp -f ${SPECFILE}.new $SPECFILE
rm -f ${SPECFILE}.new

spec=`basename $SPECFILE`
prjdir=`echo $SPECFILE | sed "s%$spec%%g"`
if [ "$prjdir" = "" ] ; then
   prjdir="."
fi
cd $prjdir
make clean distclean
cd ..
tar czvf $source $name/*
echo "New build: $source"

