set -x
mkdir $HOME/lib
export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH
mkdir $HOME/bin
export PATH=$HOME/bin:$PATH
mkdir $HOME/include
export CPLUS_INCLUDE_PATH=$HOME/include:$CPLUS_INCLUDE_PATH
wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.sh
sh cmake-3.2.2-Linux-x86_64.sh --prefix=$HOME --exclude-subdir
if [ "`echo $CXX`" == "g++" ]; then
      export CXX=g++-4.8;
fi
if [ "`echo $CXX`" == "clang++" ]; then
      export CXX=clang++-3.6 && travis/install_libcxx.sh
fi
