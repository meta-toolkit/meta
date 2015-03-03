---
title: Setup Guide
layout: default
---

# Setup Guide

## Outline
- [Mac OS X](#mac-os-x-build-guide)
- [Ubuntu](#ubuntu-build-guide)
- [Arch Linux](#arch-linux-build-guide)
- [EWS/EngrIT](#ewsengrit-build-guide) (this is UIUC-specific)
- [Generic Setup Notes](#generic-setup-notes)

## Mac OS X Build Guide
Mac OS X 10.6 or higher is required. You may have success with 10.5, but
this is not tested.

You will need to have [homebrew][homebrew] installed, as well as the
Command Line Tools for Xcode (homebrew requires these as well, and it will
prompt for them during install, or you can install them with `xcode-select
--install` on recent versions of OS X).

Once you have homebrew installed, run the following commands to get the
dependencies for MeTA:

{% highlight bash %}
brew update
brew install cmake
brew install icu4c
{% endhighlight %}

To get started, run the following commands:

{% highlight bash %}
# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Release -DICU_ROOT=/usr/local/opt/icu4c
make
{% endhighlight %}

You can now test the system by running the following command:

{% highlight bash %}
ctest --output-on-failure
{% endhighlight %}

If everything passes, congratulations! MeTA seems to be working on your
system.

## Ubuntu Build Guide

The directions here depend greatly on your installed version of Ubuntu. To
check what version you are on, run the following command:

{% highlight bash %}
cat /etc/issue
{% endhighlight %}

If it reads "Ubuntu 12.04 LTS" or something of that nature, see the
[Ubuntu 12.04 LTS Build Guide](#ubuntu-12.04-lts-build-guide). If it reads
"Ubuntu 14.04 LTS" (or 14.10), see the
[Ubuntu 14.04 LTS Build Guide](#ubuntu-14.04-lts-build-guide). If your
version is less than 12.04 LTS, your operating system is not supported
(even by your vendor) and you should upgrade to at least 12.04 LTS (or
14.04 LTS, if possible).

### Ubuntu 12.04 LTS Build Guide
Building on Ubuntu 12.04 LTS requires more work than its more up-to-date
14.04 sister, but it can be done relatively easily. You will, however, need
to install a newer C++ compiler from a ppa, and switch to it in order to
build meta. We will also need to install a newer CMake version than is
natively available.

Start by running the following commands to get the dependencies that we
will need for building MeTA.

{% highlight bash %}
# this might take a while
sudo apt-get update
sudo apt-get install python-software-properties

# add the ppa that contains an updated g++
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update

# this will probably take a while
sudo apt-get install g++ g++-4.8 libicu-dev git make wget

wget http://www.cmake.org/files/v3.1/cmake-3.1.1-Linux-x86_64.sh
sudo sh cmake-3.1.1-Linux-x86_64.sh --prefix=/usr/local
{% endhighlight %}

During CMake installation, you should agree to the license and then say "n"
to including the subdirectory. You should be able to run the following
commands and see the following output:

{% highlight bash %}
g++-4.8 --version
{% endhighlight %}

should print

    g++-4.8 (Ubuntu 4.8.1-2ubuntu1~12.04) 4.8.1
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

and

{% highlight bash %}
/usr/local/bin/cmake --version
{% endhighlight %}

should print

    cmake version 3.1.1

    CMake suite maintained and supported by Kitware (kitware.com/cmake).

Once the dependencies are all installed, you should be ready to build. Run
the following commands to get started:

{% highlight bash %}
# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
CXX=g++-4.8 /usr/local/bin/cmake ../ -DCMAKE_BUILD_TYPE=Release
make
{% endhighlight %}

You can now test the system by running the following command:

{% highlight bash %}
/usr/local/bin/ctest --output-on-failure
{% endhighlight %}

If everything passes, congratulations! MeTA seems to be working on your
system.

### Ubuntu 14.04 LTS Build Guide
Ubuntu 14.04 has a recent enough GCC for building MeTA, but we'll need to
add a ppa for a more recent version of CMake.

Start by running the following commands to install the dependencies for
MeTA.

{% highlight bash %}
# this might take a while
sudo apt-get update
sudo apt-get install software-properties-common

# add the ppa for cmake
sudo add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update

# install dependencies
sudo apt-get install cmake libicu-dev git
{% endhighlight %}

Once the dependencies are all installed, you should double check your
versions by running the following commands.

{% highlight bash %}
g++ --version
{% endhighlight %}

should output

    g++ (Ubuntu 4.8.2-19ubuntu1) 4.8.2
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

and

{% highlight bash %}
cmake --version
{% endhighlight %}

should output

    cmake version 3.1.1

    CMake suite maintained and supported by Kitware (kitware.com/cmake).

Once the dependencies are all installed, you should be ready to build. Run
the following commands to get started:

{% highlight bash %}
# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
cmake ../ -DCMAKE_BUILD_TYPE=Release
make
{% endhighlight %}

You can now test the system by running the following command:

{% highlight bash %}
ctest --output-on-failure
{% endhighlight %}

If everything passes, congratulations! MeTA seems to be working on your
system.

## Arch Linux Build Guide
Arch Linux consistently has the most up to date packages due to its rolling
release setup, so it's often the easiest platform to get set up on.

To install the dependencies, run the following commands.

{% highlight bash %}
sudo pacman -Sy
sudo pacman -S clang cmake git icu libc++ make
{% endhighlight %}

Once the dependencies are all installed, you should be ready to build. Run
the following commands to get started:

{% highlight bash %}
# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Release
make
{% endhighlight %}

You can now test the system by running the following command:

{% highlight bash %}
ctest --output-on-failure
{% endhighlight %}

If everything passes, congratulations! MeTA seems to be working on your
system.

## EWS/EngrIT Build Guide
If you are on a machine managed by Engineering IT at UIUC, you should
follow this guide. These systems have software that is much too old for
building MeTA, but EngrIT has been kind enough to package updated versions
of research software as modules. The modules provided for GCC and CMake are
recent enough to build MeTA, so it is actually mostly straightforward.

To set up your dependencies (**you will need to do this every time you log
back in to the system**), run the following commands:

{% highlight bash %}
module load gcc
module load cmake
{% endhighlight %}

Once you have done this, double check your versions by running the
following commands.

{% highlight bash %}
g++ --version
{% endhighlight %}

should output

    g++ (GCC) 4.8.2
    Copyright (C) 2013 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

and

{% highlight bash %}
cmake --version
{% endhighlight %}

should output

    cmake version 3.1.1

    CMake suite maintained and supported by Kitware (kitware.com/cmake).

If your versions are correct, you should be ready to build. To get started,
run the following commands:

{% highlight bash %}
# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
CXX="/software/gcc-4.8.2/bin/g++" cmake ../ -DCMAKE_BUILD_TYPE=Release
make
{% endhighlight %}

You can now test the system by running the following command:

{% highlight bash %}
ctest --output-on-failure
{% endhighlight %}

If everything passes, congratulations! MeTA seems to be working on your
system.

## Generic Setup Notes

 - There are rules for clean, tidy, and doc. **After you run the cmake command
   once, you will be able to just run `make` as usual** when you're
   developing---it'll detect when the CMakeLists.txt file has changed and
   rebuild Makefiles if it needs to.

 - To compile in debug mode, just replace `Release` with `Debug` in the
   appropriate `cmake` command for your OS above and rebuild using `make`
   after.

----

[homebrew]: http://brew.sh
