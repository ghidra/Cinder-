#! /bin/bash
make_folder(){
  #$1 FOLDER
  if [ ! -e $1 ];then
    mkdir $1
    echo "          -"$1" directory CREATED"
  else
    echo "          -"$1" directory ALREADY EXISTS"
  fi
}

cd ../..
make_folder "build"
cd build && cmake .. -DCINDER_BOOST_USE_SYSTEM=1 && make
cmake .. -DCMAKE_BUILD_TYPE=Release -DCINDER_BOOST_USE_SYSTEM=1 && make.

