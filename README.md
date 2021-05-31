## About
This is a personal project and toy compiler used as opportunity to learn Boost Spirit (Qi and X3), as well as LLVM.

The name comes from my own name and references South Park's Marklar species that refer to nouns, verbs, etc. simply as "Marklar". The compilar jokingly had only ambiguous "marklar" types (in truth there was no type system and the underlying type used was just an i32).

## Getting Started
Currently this only builds under Linux, other operating systems should work though.


### Prerequistes
This assumes a distro with apt:

```
apt install cmake build-essential llvm-9-dev libboost1.74-all-dev
```

### Building
1. Clone the repo
   ```
   git clone git clone https://github.com/drivehappy/marklar
   ```

2. Generate makefile via cmake:
   ```
   mkdir marklar_build
   cd marklar_build
   cmake ../marklar
   ```

3. Build (optionally supplying -j#, where # is the number of CPUs used):
   ```
   make
   ```

