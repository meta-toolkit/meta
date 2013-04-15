# MeTA: ModErn Text Analysis

## Project setup

 - You will probably want to enable SVM classifier functionality through
   liblinear. Download the source
   [here](http://www.csie.ntu.edu.tw/~cjlin/liblinear/),
   and add the path to liblinear in config.ini (replace the existing path).
   Make sure you compile it.

 - You will also probably want to enable supervised latent Dirichlet allocation
   through slda. Download the source
   [here](http://www.cs.cmu.edu/~chongw/slda/). Add
    ```C++
    #include <cstddef>
    ```
   at the top of corpus.h, and compile. Again, add the path to slda to
   config.ini.

 - To compile initially, run the following commands
    ```bash
    mkdir build
    cd build
    CXX=clang++ LDFLAGS="-lcxxrt -ldl" cmake ../ -DCMAKE_BUILD_TYPE=Debug
    make
    ```

 - There are rules for clean, tidy, and doc. (Also, once you run the cmake
   command once, you should be able to just run make like usual as you're
   developing---it'll detect when the CMakeLists.txt file has changed and rebuild
   Makefiles if it needs to.)
