# Tests
include_directories ("~/dev/spirit/include")
include_directories ("~/dev/llvm/include")
include_directories ("~/dev/llvm-build/include")

link_directories ("~/dev/llvm-build/lib")

include_directories ("${PROJECT_SOURCE_DIR}/src/libmarklarc")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

file(GLOB libmarklarc_TESTS
	"*.cpp"
)
add_executable (marklarctests ${libmarklarc_TESTS})
target_link_libraries (marklarctests libmarklarc -lboost_program_options -lboost_system -lboost_filesystem)
