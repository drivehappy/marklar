#include <gtest/gtest.h>

#include <sys/wait.h>

#include <algorithm>
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

	vector<string> g_cleanupFiles = {
		g_outputExe,
		g_outputBitCode,
		"output_opt.bc",
		"output.o",
	};

	void cleanupFiles() {
		for (auto& itr : g_cleanupFiles) {
			boost::filesystem::remove(itr);
		}
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
		const int r = system(("./" + exe).c_str());
		if (r == -1) {
			return -1;
		}

		return WEXITSTATUS(r);
	}

}

TEST(DriverTest, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"  return 3;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(3, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionSingleDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 2;"
		"  return i;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(2, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionMultiDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 2;"
		"  int j = 5;"
		"  return j;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(5, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionMultiDeclSum) {
	const auto testProgram =
		"int main() {"
		"  int i = 2;"
		"  int j = 5;"
		"  return i + j;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(7, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionMultiDeclSumComplex) {
	const auto testProgram =
		"int main() {"
		"  int i = 2;"
		"  int j = 5;"
		"  return i + j + 6;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(13, runExecutable(g_outputExe));
}

// Tests scoping rules of multiple functions with identical variable names
TEST(DriverTest, MultipleFunction) {
	const auto testProgram =
		"int bar() {"
		"  int a = 5;"
		"  return a;"
		"}"
		"int foo() {"
		"  int a = 4;"
		"  return a;"
		"}"
		"int main() {"
		"  int a = 3;"
		"  return a;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(3, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionUseArgs) {
	const auto testProgram =
		"int main(int a) {"
		"  return a;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
	EXPECT_EQ(2, runExecutable(g_outputExe + " arg1"));
	EXPECT_EQ(3, runExecutable(g_outputExe + " arg1 arg2"));
}

TEST(DriverTest, FunctionCall) {
	const auto testProgram =
		"int foo(int a) {"
		"  return a + 1;"
		"}"
		"int main(int a) {"
		"  return foo(a);"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(2, runExecutable(g_outputExe));
	EXPECT_EQ(3, runExecutable(g_outputExe + " arg1"));
	EXPECT_EQ(4, runExecutable(g_outputExe + " arg1 arg2"));
}

TEST(DriverTest, FunctionIfStmtReturnSimple) {
	const auto testProgram =
		"int main() {"
		"  if (3 < 4) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionIfStmtReturn) {
	const auto testProgram =
		"int main() {"
		"  int a = 3;"
		"  int b = 4;"
		"  if (a < b) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
}

TEST(DriverTest, FunctionIfElseStmtReturn) {
	const auto testProgram =
		"int main() {"
		"  int a = 3;"
		"  int b = 4;"
		"  if (a > b) {"
		"    return 1;"
		"  } else {"
		"    return 0;"
		"  }"
		"  return 2;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(0, runExecutable(g_outputExe));
}

TEST(DriverTest, OperatorLessThan) {
	const auto testProgram =
		"int main() {"
		"  if (3 < 4) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
}

TEST(DriverTest, OperatorGreaterThan) {
	const auto testProgram =
		"int main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(2, runExecutable(g_outputExe));
}

TEST(DriverTest, OperatorEqual) {
	const auto testProgram =
		"int main() {"
		"  if (4 == 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
}

TEST(DriverTest, OperatorModulo) {
	const auto testProgram =
		"int main() {"
		"  int a = 5 % 3;"
		"  if (a == 2) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		cleanupFiles();
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(createExe(testProgram));

	EXPECT_EQ(1, runExecutable(g_outputExe));
}

