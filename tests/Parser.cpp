#include <gtest/gtest.h>

#include <parser.h>

using namespace marklar;


TEST(ParserTest, BasicFunction) {
	const auto testProgram =
		"i32 main() {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, SkipperStart) {
	// Has a newline before the function
	const auto testProgram = R"mrk(
		i32 main() {
			return 0;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, BasicComment) {
	const auto testProgram =
		"// This is a comment\n"
		"i32 main() {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, ComplexComment) {
	const auto testProgram =
		"//// Blah //a//a//a This is a comment /\n"
		"i32 main() {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, InvalidComment) {
	const auto testProgram =
		"a // This is a comment\n"
		"i32 main() {"
		"}";

	EXPECT_FALSE(parse(testProgram));
}

TEST(ParserTest, FunctionSingleDecl) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionSingleDecl_NameCheck1) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i09za_ = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionSingleDecl_NameCheck2) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i' = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionMultiDecl) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  i32 j = 0;"
		"  i32 k = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionDeclAssign) {
	const auto testProgram =
		"i32 main() {"
		"  i32 r = 1 + 2;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionMultiDeclAssign) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionReturn) {
	const auto testProgram =
		"i32 main() {"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionReturnComplex) {
	const auto testProgram =
		"i32 main() {"
		"  return a + b + c + 0 + 1 + d;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, MultipleFunction) {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, MultipleComplexFunction) {
	const auto testProgram =
		"i32 bar() {"
		"  i32 a = 0;"
		"  i32 b = 2;"
		"  return a + b + 0;"
		"}"
		"i32 foo() {"
		"  return a + 0;"
		"}"
		"i32 main() {"
		"  return 0 + 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionArgs) {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCall) {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {"
		"  foo();"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCallArgs) {
	const auto testProgram =
		"i32 foo(i32 a) {}"
		"i32 main() {"
		"  foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionCallArgsComplex) {
	const auto testProgram =
		"i32 bar(i32 a, i32 b) {}"
		"i32 foo(i32 a) {"
		"  return bar(a, 5);"
		"}"
		"i32 main() {"
		"  return foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionIfStmt) {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FunctionIfElseStmt) {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"  } else {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, Assignment) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  a = a + 1;"
		"  return a;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, WhileStmt) {
	const auto testProgram =
		"i32 main() {"
		"  while (i < 4) {"
		"    if (i > 5) {"
		"    } else {"
		"    }"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, LogicalOR) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 0;"
		"  if (a || b) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, Division) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 / 3;"
		"  return i;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, Subtraction) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 - 3;"
		"  return i;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, RightShift) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 257 >> 8;"
		"  return i;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, Multiplication) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 8 * 4;"
		"  return i;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, ComplexEulerProblem1) {
	const auto testProgram =
		"i32 main() {"
		"  i32 sum = 0;"
		"  i32 i = 0;"
		"  while (i < 1000) {"
		"    if (((i % 5) == 0) || ((i % 3) == 0)) {"
		"      sum = sum + i;"
		"    }"
		"    i = i + 1;"
		"  }"
		"  return sum;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, FuncCallInIfStmt) {
	const auto testProgram =
		"i32 func1(i32 a) {"
		"  return 0;"
		"}"
		"i32 main() {"
		"  if (func1(45) > 0) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, LogicalAnd) {
	const auto testProgram =
		"i32 main() {"
		"  while ((0 != 1) && (0 != 1)) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, String) {
	const auto testProgram =
		"i32 main() {"
		"  printf(\"string test\");"
		"  return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, PrimitiveType_i64) {
	const auto testProgram = R"mrk(
		i64 main() {
		  return 0;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, UserDefinedType_Basic) {
	const auto testProgram = R"mrk(
		type MyType {
			i64 a;
			i64 b;
		}

		i64 main() {
		  return 0;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, UserDefinedType_UseBasic) {
	const auto testProgram = R"mrk(
		type MyType {
			i64 a;
			i64 b;
		}

		i64 main() {
			MyType t;

			return 0;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ParserTest, UserDefinedType_UseComplex) {
	const auto testProgram = R"mrk(
		type MyType {
			i64 a;
			i64 b;
		}

		i64 main() {
			MyType t;
			t.a = 1;
			t.b = 3;

			return t.b;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram));
}

