# pthsafe_linked_list
thread safe linked list with test code

## Install

You need to have **autotools** installed - or at least it is advised to be existing before install.

In that case if you've got **autotools** installed, then call ***autoreconf -imf*** in the root directory of the package - where you can find **configure.ac** file.

```bash
    % autoreconf -imf
```

If no errors occur, then you need to run ***configure*** script as follows:

```bash
    % ./configure
```

This script will create ***src/Makfile*** for you, which then should be used to build the test program.

The contents of the package directory will look like this (*see below*):

```bash
    ├── aclocal.m4
    ├── autom4te.cache
    │   ├── output.0
    │   ├── requests
    │   └── traces.0
    ├── compile
    ├── config.guess
    ├── config.h
    ├── config.h.in
    ├── config.log
    ├── config.status
    ├── config.sub
    ├── configure
    ├── configure.ac
    ├── configure~
    ├── depcomp
    ├── install-sh
    ├── missing
    ├── README.md
    ├── src
    │   ├── ll.c
    │   ├── ll.h
    │   ├── main.c
    │   ├── Makefile
    │   ├── Makefile.am
    │   ├── Makefile.in
    │   ├── sf.c
    │   └── sf.h
    ├── stamp-h1
    └── VERSION
```

From now it's quite simpe, you just go into ***src*** subdirectory, then call **make**.

If you have got **valgrind** tool, then you can check for the memory leaks, and inconsistencies.

```bash
(cd src && make) && valgrind -s --tool=drd --trace-rwlock=yes ./src/tsllist_demo
```
