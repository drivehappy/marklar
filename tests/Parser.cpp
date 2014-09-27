#include <gtest/gtest.h>

#include <parser.h>

using namespace marklar;


TEST(ParserTest, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionSingleDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionMultiDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
		"  int j = 0;"
		"  int k = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionDeclAssign) {
	const auto testProgram =
		"int main() {"
		"  int r = 1 + 2;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionMultiDeclAssign) {
	const auto testProgram =
		"int main() {"
		"  int i = 1 + 2;"
		"  int j = i + 2;"
		"  int k = i + j;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionReturn) {
	const auto testProgram =
		"int main() {"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

