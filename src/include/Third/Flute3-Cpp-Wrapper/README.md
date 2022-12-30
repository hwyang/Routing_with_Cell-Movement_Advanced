# Flute3-Cpp-Wrapper
C++ Wrapper of flute-3.1 from FastRoute 4.1


## 1. How to Build
**Step 1:** Download the source code. For example,
~~~
$ git clone --recursive https://github.com/jacky860226/Flute3-Cpp-Wrapper.git
~~~

or

~~~
$ git clone https://github.com/jacky860226/Flute3-Cpp-Wrapper.git
$ cd Flute3-Cpp-Wrapper
$ git submodule init
$ git submodule update --recursive
$ cd ..
~~~

**Step 2:** Go to the project root and build by
~~~
$ cd Flute3-Cpp-Wrapper
$ make
~~~

### 1.1. Dependencies

* [GCC](https://gcc.gnu.org/) (version >= 7.5.0) or other working c++ compliers

## 2. How to run

1. Include header `Flute3Wrapper.hpp`
2. Link `libflute3wrapper.a` while compiling.

For details, please refer to `test.cpp`

## 3. How to test
~~~
$ make test
$ ./test
~~~

The output may be:
```
FLUTE wirelength only = 140
FLUTE wirelength = 140
FLUTE degree = 6
FLUTE printtree
 0:  x=40  y=20  e=6
 1:  x=80  y=20  e=6
 2:  x=20  y=30  e=7
 3:  x=50  y=40  e=8
 4:  x=30  y=50  e=9
 5:  x=60  y=60  e=9
s6:  x=50  y=20  e=7
s7:  x=50  y=30  e=8
s8:  x=50  y=40  e=9
s9:  x=50  y=50  e=9

FLUTE plottree
40 20
50 20

80 20
50 20

20 30
50 30

50 40
50 40

30 50
50 50

60 60
50 50

50 20
50 30

50 30
50 40

50 40
50 50

50 50
50 50

FLUTE writeSVG: test.svg
```
