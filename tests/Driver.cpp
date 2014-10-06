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

namespace {

	const string g_outputBitCode = "output.bc";
	const string g_outputExe = "a.out";

	vector<string> g_cleanupFiles = {
		g_outputExe,
		g_outputBitCode,
		"output_opt.bc",
		"output.o",
	};

}


TEST(DriverTest, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"  return 3;"
		"}";

	// Cleanup generated intermediate and executable files
	BOOST_SCOPE_EXIT(void) {
		for (auto& itr : g_cleanupFiles) {
			boost::filesystem::remove(itr);
		}
	} BOOST_SCOPE_EXIT_END

	EXPECT_TRUE(driver::generateOutput(testProgram, g_outputBitCode ));
	EXPECT_TRUE(driver::optimizeAndLink(g_outputBitCode, g_outputExe));

	const int r = system(("./" + g_outputExe).c_str());
	EXPECT_NE(-1, r);
	EXPECT_EQ(3, WEXITSTATUS(r));
}

