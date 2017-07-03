# Container

[![Build Status](https://travis-ci.org/matheuspf/Container.svg?branch=master)](https://travis-ci.org/matheuspf/Container) [![Coverage Status](https://coveralls.io/repos/github/matheuspf/Container/badge.svg?branch=master)](https://coveralls.io/github/matheuspf/Container?branch=master)

This is a C++14 implementation of a multidimensional container. 

Provides a simple and very generic interface, while aiming for performance.


<br>

### Google Test


There are a number of tests using [Google Test](https://github.com/google/googletest).

If you want to run the tests:

```
cd test
mkdir build
cd build

cmake ..
cmake --build .

./ContainerTests
```

Google Test will be downloaded automatically from the repository.


<br>

### Documentation

If you want to generate the documentation, install [Doxygen](http://www.stack.nl/~dimitri/doxygen/) and run:

```
cd doc
doxygen Doxyfile
```