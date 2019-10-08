set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

file(GLOB libmarklarc_SRC
	"*.h"
	"*.cpp"
)

add_library(libmarklarc ${libmarklarc_SRC})

# '/usr/lib/llvm-9/lib' is the output of 'llvm-config --ldflags', find a way to run this automatically
include_directories (/usr/include/llvm-9/ /usr/include/llvm-c-9/)
target_link_libraries (libmarklarc -L/usr/lib/llvm-9/lib -lLLVM-9)

