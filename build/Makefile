CXX				:= g++
#CXX				:= clang

#LLVM_INCLUDE	:= /usr/include/llvm-3.5/
#LLVM_DIR		:= /home/mharmer/dev/llvm-3.6/
LLVM_DIR		:= /home/mharmer/dev/llvm/
LLVM_BUILD_DIR		:= /home/mharmer/dev/llvm-build/
LLVM_INCLUDE		:= $(LLVM_DIR)/include/
LLVM_BUILD_INCLUDE	:= $(LLVM_BUILD_DIR)/include/
LLVM_LIB		:= $(LLVM_DIR)/lib
LLVM_BUILD_LIB		:= $(LLVM_BUILD_DIR)/Release+Asserts/lib/
LLVM_BIN		:= $(LLVM_DIR)/bin
LLVM_BUILD_BIN		:= $(LLVM_BUILD_DIR)/Release+Asserts/bin
#GTEST_DIR		:= /home/mharmer/dev/gtest-1.7.0/

#CC_FLAGS 		:= -Wall -Werror -O0 -g -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -I../src/ -I../src/lib/ -I/usr/include/llvm-3.5/ -I/usr/include/llvm-c-3.5/ -L/usr/lib/x86_64-linux-gnu -L/usr/lib/llvm-3.5/lib 
CC_FLAGS 		:= -Wall -Werror -O2 -std=c++14 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -I../src/ -I../src/lib/ -I$(LLVM_INCLUDE) -I$(LLVM_BUILD_INCLUDE) -L/usr/lib/x86_64-linux-gnu -L/usr/lib -L$(LLVM_LIB) -L$(LLVM_BUILD_LIB)

#-L$(GTEST_DIR)/lib/
#-I$(GTEST_DIR)/include

#LD_FLAGS 		:=  `$(LLVM_BIN)/llvm-config --libs all` `$(LLVM_BIN)/llvm-config --ldflags --system-libs` -lboost_program_options -lboost_system -lboost_filesystem
LD_FLAGS 		:=  `$(LLVM_BUILD_BIN)/llvm-config --libs all` `$(LLVM_BUILD_BIN)/llvm-config --ldflags --system-libs` -lboost_program_options -lboost_system -lboost_filesystem
LD_FLAGS_TESTS	:= $(LD_FLAGS) -lgtest_main -lgtest -lpthread
CPP_FILES		:= $(wildcard ../src/*.cpp)
CPP_FILES_LIB	:= $(wildcard ../src/lib/*.cpp)
CPP_FILES_TESTS	:= $(wildcard ../tests/*.cpp)
OBJ_FILES_LIB	:= $(patsubst ../src/lib/%.cpp,../src/lib/%.o,$(CPP_FILES_LIB))
OBJ_FILES		:= $(patsubst ../src/%.cpp,../src/%.o,$(CPP_FILES))
OBJ_FILES_TESTS	:= $(patsubst ../tests/%.cpp,../tests/%.o,$(CPP_FILES_TESTS))

#
all: build-all

build-all: main tests

../src/%.o: ../src/%.cpp
	$(CXX) $(CC_FLAGS) -c -o $@ $<

../src/lib/%.o: ../src/lib/%.cpp
	$(CXX) $(CC_FLAGS) -c -o $@ $<

../tests/%.o: ../tests/%.cpp 
	$(CXX) $(CC_FLAGS) -c -o $@ $<

main: $(OBJ_FILES) $(OBJ_FILES_LIB)
	$(CXX) $(CC_FLAGS) -o marklarc $(OBJ_FILES) $(OBJ_FILES_LIB) $(LD_FLAGS)

tests: $(OBJ_FILES_TESTS) $(OBJ_FILES_LIB)
	$(CXX) $(CC_FLAGS) -o tests $(OBJ_FILES_TESTS) $(OBJ_FILES_LIB) $(LD_FLAGS_TESTS)
 
output: output.o wrapper.o
	gcc output.o -o output
	g++ -std=c++14 wrapper.cpp -o wrapper

output.o: output.asm 
	as --32 -o output.o output.asm 


#.PHONY: build-dirs
#.IGNORE: build-dirs
#build-dirs:
	#mkdir -p out gen

.PHONY: clean
.IGNORE: clean
clean: 
	rm -rf ../src/*.o ../src/lib/*.o ../tests/*.o marklarc tests

