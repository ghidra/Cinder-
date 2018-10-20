#! /bin/bash

# needs 1 argument
# argument 1: Where we want the project folder to be made

make_folder(){
  #$1 FOLDER
  if [ ! -e $1 ];then
    mkdir $1
    echo "          -"$1" directory CREATED"
  else
    echo "          -"$1" directory ALREADY EXISTS"
  fi
}

#PROJECTPATH=`cd "$1"; pwd`

if [ $# -eq 0 ];then
	echo "***********************************"
	echo "no arguments given, please provide:"
	echo "     -desired project path"
	echo "***********************************"
else
  if [ ! -d $1 ];then
  	echo "***********************************"
  	echo "     -Making Project Directory"
  	echo "***********************************"
  	make_folder $1
  	make_folder $1"/proj"
  	make_folder $1"/proj/cmake"

  	touch $1"/proj/cmake/CMakeLists.txt"
text="cmake_minimum_required( VERSION 2.8 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( APPNAMEApp )

get_filename_component( CINDER_PATH \"\${CMAKE_CURRENT_SOURCE_DIR}/../../../../Cinder\" ABSOLUTE )
get_filename_component( APP_PATH \"\${CMAKE_CURRENT_SOURCE_DIR}/../../\" ABSOLUTE )

include( \"\${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake\" )

set( SRC_FILES
	\${APP_PATH}/src/App.cpp
)
ci_make_app(
	SOURCES     \${SRC_FILES}
	INCLUDES    \${APP_PATH}/include
	CINDER_PATH \${CINDER_PATH}
	BLOCKS		\${CINDER_PATH}/blocks/Cinder-
)"
    printf "%b" "$text" > $1"/proj/cmake/CMakeLists.txt"
    echo "     -CMakeLists.txt created"
  else
  	echo "***********************************"
  	echo "     -Directory Already Exists"
  	echo "***********************************"
  fi

fi
