# MPI deadlock GCC plugin

Static analyses to find the origin of MPI collective errors

## Install GCC 12.2.0

Download GCC 12.2.0 and unpack it:

```
wget http://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
tar -xvzf gcc-12.2.0.tar.gz
```

Check if you meet all GCC dependencies:

```
./gcc-12.2.0/contrib/download_prerequisites
```

Create a build directory:

```
mkdir gcc-build
cd gcc-build
```

Configure the source to enable plugins:

```
../gcc-12.2.0/configure --prefix=<INSTALLDIR> --enable-languages=c,c++,fortran --enable-plugin --disable-bootstrap --disable-multilib
```

Build and install GCC:

```
make -j$(getconf _NPROCESSORS_ONLN) && make install
```

Make sure plugins are available:

```
$ <INSTALLDIR>/bin/gcc -print-file-name=plugin
<INSTALLDIR>/lib/gcc/x86_64-pc-linux-gnu/12.2.0/plugin
```

## GCC plugins

While GCC is installing, you can take a look at these resources:
- [GCC plugins documentation](https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gccint/Plugins.html#Plugins)
- [Experiments with the GCC plugin mechanism](https://github.com/rofirrim/gcc-plugins)

## Set up the Makefile

The Makefile may need some adjustment depending on your GCC installation:

```Makefile
# -------------------------------- Compilers --------------------------------- #
CC     = gcc_1220
MPICC  = mpicc # export MPICH_CC="gcc_1220"
CFLAGS = #-Wall -Wextra -Wstrict-prototypes -Wunreachable-code -Werror -O3 -g

CXX = g++_1220

PLUGIN_FLAGS = -I`$(CC) -print-file-name=plugin`/include -I$(INCLUDEDIR) \
               -Wall -fPIC -fno-rtti -g -shared
```

In my case, I use aliases to call GCC 12.2.0:

```bash
alias gcc_1220="<INSTALLDIR>/bin/gcc"
alias g++_1220="<INSTALLDIR>/bin/g++"
```

You can also add the executable directly in your `PATH` but make sur to rename
it before to avoid conflict with an other version of GCC already installed on
your computer. For example:

```
$ ls $HOME/.local/bin
g++_1220         gcc_1220
$ export PATH="$HOME/.local/bin:$PATH"
```

Here `$HOME/.local` is the `<INSTALLDIR>`.
