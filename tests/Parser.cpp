#include <gtest/gtest.h>

#include <parser.h>

using namespace marklar;


TEST(ParserTest, BasicFunction) {
	const auto testProgram =
		"int main(){"
		"}";

	parse(testProgram);
}

TEST(ParserTest, Dummy) {
	//EXPECT_EQ(0, 1);

	const auto testProgram =
		"int main() {\n"
		"  int i = 0;\n"
		"  int j = 0;\n"
		"  int r = i + j;\n"
		"  return r;\n"
		"}";

	parse(testProgram);
}

TEST(ParserTest, Boost) {
	//EXPECT_EQ(123.41, parseTest("123.4"));
}

