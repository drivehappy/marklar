#include "catch.hpp"

#include <sys/wait.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>

#include <driver.h>


using namespace marklar;
using namespace std;

// Unit test helpers
namespace {

	const string g_outputBitCode = "output.bc";
	const string g_outputExe = "a.out";
	const string g_outputStdout = "testStdout.txt";

	vector<string> g_cleanupFiles = {
		g_outputExe,
		g_outputBitCode,
		g_outputStdout,
		"output_opt.bc",
		"output.o",
	};

	void cleanupFiles() {
		for (auto& itr : g_cleanupFiles) {
			boost::filesystem::remove(itr);
		}
	}

	string loadFileContents(const string& filename) {
		ifstream ifs(filename.c_str());
		stringstream buffer;
		buffer << ifs.rdbuf();
		return buffer.str();
	}

	string stdoutContents() {
		return loadFileContents(g_outputStdout);
	}

	bool createExe(const string& testProgram) {
		if (!driver::generateOutput(testProgram, g_outputBitCode)) {
			return false;
		}
		if (!driver::optimizeAndLink(g_outputBitCode, g_outputExe)) {
			return false;
		}
		return true;
	}

	int runExecutable(const string& exe) {
		const int r = system(("./" + exe + " > " + g_outputStdout).c_str());
		if (r == -1) {
			return -1;
		}

		return WEXITSTATUS(r);
	}


	class DriverTestFixture {
	public:
		void TearDown() {
			cleanupFiles();
		}
	};

}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_BasicFunction") {
	const auto testProgram =
		"i32 main() {"
		"  return 3;"
		"}";

	/*
	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END
	*/

	REQUIRE(createExe(testProgram));

	CHECK(3 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionSingleDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  return i;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionMultiDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return j;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(5 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionMultiDeclSum") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return i + j;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(7 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionMultiDeclSumComplex") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return i + j + 6;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(13 == runExecutable(g_outputExe));
}

// Tests scoping rules of multiple functions with identical variable names
TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_MultipleFunction") {
	const auto testProgram =
		"i32 bar() {"
		"  i32 a = 5;"
		"  return a;"
		"}"
		"i32 foo() {"
		"  i32 a = 4;"
		"  return a;"
		"}"
		"i32 main() {"
		"  i32 a = 3;"
		"  return a;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(3 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionUseArgs") {
	const auto testProgram =
		"i32 main(i32 a) {"
		"  return a;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
	CHECK(2 == runExecutable(g_outputExe + " arg1"));
	CHECK(3 == runExecutable(g_outputExe + " arg1 arg2"));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionCall") {
	const auto testProgram =
		"i32 foo(i32 a) {"
		"  return a + 1;"
		"}"
		"i32 main(i32 a) {"
		"  return foo(a);"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
	CHECK(3 == runExecutable(g_outputExe + " arg1"));
	CHECK(4 == runExecutable(g_outputExe + " arg1 arg2"));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionIfStmtReturnSimple") {
	const auto testProgram =
		"i32 main() {"
		"  if (3 < 4) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionIfStmtReturn") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  i32 b = 4;"
		"  if (a < b) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FunctionIfElseStmtReturn") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  i32 b = 4;"
		"  if (a > b) {"
		"    return 1;"
		"  } else {"
		"    return 0;"
		"  }"
		"  return 2;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(0 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_OperatorLessThan") {
	const auto testProgram =
		"i32 main() {"
		"  if (3 < 4) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_OperatorGreaterThan") {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_OperatorEqual") {
	const auto testProgram =
		"i32 main() {"
		"  if (4 == 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_OperatorModulo") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 5 % 3;"
		"  if (a == 2) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_WhileStmt") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 2;"
		"  i32 b = 6;"
		"  while (a < b) {"
		"    a = a + 1;"
		"  }"
		"  return a;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(6 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_LogicalOR") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 4;"
		"  if ((a == 0) || (b == 0)) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_LogicalAND") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 4;"
		"  if ((a == 0) && (b == 0)) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_Division") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 / 3;"
		"  return i;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_Subtraction") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 - 3;"
		"  return i;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_Multiplication") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 * 3;"
		"  return i;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(15 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_MultiMethods") {
	const auto testProgram =
		"i32 a() {"
		"  return 1;"
		"}"
		"i32 main() {"
		"  i32 i = 5 * 3;"
		"  return i;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(15 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FuncCallInIfStmt") {
	const auto testProgram =
		"i32 func1(i32 a) {"
		"	return a + 5;"
		"}"
		"i32 main() {"
		"	if (func1(10) < 15) {"
		"		return 1;"
		"	}"
		"   return func1(10);"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(15 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_IfWith2ReturnStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (1 == 1) {"
		"    return 1;"
		"    return 2;"    
		"  } else {"
		"    return 0;"
		"    return 5;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_WhileWithReturnStmt") {
	const auto testProgram =
		"i32 main() {"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(1 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FuncWithEarlyReturnStmt") {
	const auto testProgram =
		"i32 main() {"
		"	return 2;"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(2 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_MultipleChainedFunctionCall") {
	const auto testProgram =
		"i32 unaryFunc(i32 n) {"
		"  return n + 1;"
		"}"
		"i32 binaryFunc(i32 a, i32 b) {"
		"  return unaryFunc((a * a) + (b * b));"
		"}"
		"i32 main() {"
		"  return binaryFunc(1, 5);"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(27 == runExecutable(g_outputExe));
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_FuncWithPrintf") {
	const auto testProgram =
		"i32 main() {"
		"   printf(\"test\");"
		"   return 0;"
		"}";

	REQUIRE(createExe(testProgram));

	CHECK(0 == runExecutable(g_outputExe));

	CHECK("test" == stdoutContents());
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_PrintfEscapeChars") {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test\n");
		   return 0;
		}
		)mrk";

	REQUIRE(createExe(testProgram));

	CHECK(0 == runExecutable(g_outputExe));

	CHECK("test\n" == stdoutContents());
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_PrintfMultiArg") {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test: %s %d\n", "hello world", 32);
		   return 0;
		}
		)mrk";

	REQUIRE(createExe(testProgram));

	CHECK(0 == runExecutable(g_outputExe));

	CHECK("test: hello world 32\n" == stdoutContents());
}

TEST_CASE_METHOD(DriverTestFixture, "DriverTestFixture_PrintfMultiFunc") {
	const auto testProgram = R"mrk(
		i32 work() {
		   printf("%d\n", 1);
		   return 0;
		}
		i32 main() {
		   printf("test: %s %d\n", "hello world", 32);
		   return 0;
		}
		)mrk";

	REQUIRE(createExe(testProgram));

	CHECK(0 == runExecutable(g_outputExe));

	CHECK("test: hello world 32\n" == stdoutContents());
}

