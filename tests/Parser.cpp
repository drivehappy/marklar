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

TEST(ParserTest, FunctionReturnComplex) {
	const auto testProgram =
		"int main() {"
		"  return a + b + c + 0 + 1 + d;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, MultipleFunction) {
	const auto testProgram =
		"int foo() {}"
		"int main() {}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, MultipleComplexFunction) {
	const auto testProgram =
		"int bar() {"
		"  int a = 0;"
		"  int b = 2;"
		"  return a + b + 0;"
		"}"
		"int foo() {"
		"  return a + 0;"
		"}"
		"int main() {"
		"  return 0 + 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionArgs) {
	const auto testProgram =
		"int main(int a, int b) {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCall) {
	const auto testProgram =
		"int foo() {}"
		"int main() {"
		"  foo();"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCallArgs) {
	const auto testProgram =
		"int foo(int a) {}"
		"int main() {"
		"  foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCallArgsComplex) {
	const auto testProgram =
		"int bar(int a, int b) {}"
		"int foo(int a) {"
		"  return bar(a, 5);"
		"}"
		"int main() {"
		"  return foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionIfStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionIfElseStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"  } else {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, Assignment) {
	const auto testProgram =
		"int main() {"
		"  int a = 0;"
		"  a = a + 1;"
		"  return a;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, WhileStmt) {
	const auto testProgram =
		"int main() {"
		"  while (i < 4) {"
		"    if (i > 5) {"
		"    } else {"
		"    }"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

