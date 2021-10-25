#!/bin/bash

# get arguments
CMAKE_SOURCE_DIR=$1
MIRRORMESH_BINARY_DIR=$2
HEADER_FILE="git_log_mirrormesh.h"

# test if $HEADER_FILE is present
if [ ! -f "$MIRRORMESH_BINARY_DIR/$HEADER_FILE" ]; then
   \touch $MIRRORMESH_BINARY_DIR/$HEADER_FILE
   echo "#ifndef _GIT_LOG_MIRRORMESH_H" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   echo "#define _GIT_LOG_MIRRORMESH_H" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   echo "#define MIRRORMESH_GIT_BRANCH \"No git branch found\"" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   echo "#define MIRRORMESH_GIT_COMMIT \"No git commit found\"" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   echo "#define MIRRORMESH_GIT_DATE   \"No git commit found\"" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   echo "#endif" >> "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
fi

# parameter
git_is_present=1

# test if .git is present
if [ ! -d $CMAKE_SOURCE_DIR/.git ]; then
   git_is_present=0
fi

git_commit=`git rev-parse HEAD 2> /dev/null`
if [ "$git_commit" == "" ]; then
   git_is_present=0
fi

if [ $git_is_present -eq 1 ]; then
   git_branch=`git rev-parse --abbrev-ref HEAD 2> /dev/null`
   echo "   > Found a git branch: $git_branch"
   echo "   > Found a git commit: $git_commit"
   git_date=`git show -s --format="%ci" $git_commit`
   echo "   > Found a git date: $git_date"

   # erase any previous version file
   \rm -f $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   \touch $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#ifndef _GIT_LOG_MIRRORMESH_H" >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#define _GIT_LOG_MIRRORMESH_H" >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#define MIRRORMESH_GIT_BRANCH \"$git_branch\"" >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#define MIRRORMESH_GIT_COMMIT \"$git_commit\"" >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#define MIRRORMESH_GIT_DATE   \"$git_date\""   >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
   echo "#endif" >> $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"

   # diff
   diff=`diff $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp" "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"`
   if [ "$diff" != "" ]; then
      \cp $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp" "$MIRRORMESH_BINARY_DIR/$HEADER_FILE"
   fi

   # clean
   \rm -f $MIRRORMESH_BINARY_DIR/$HEADER_FILE"_tmp"
fi
