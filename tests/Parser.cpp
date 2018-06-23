#include "catch.hpp"

#include <parser.h>

using namespace marklar;


TEST_CASE("ParserTest_BasicFunction") {
	const auto testProgram =
		"i32 main() {"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_SkipperStart") {
	// Has a newline before the function
	const auto testProgram = R"mrk(
		i32 main() {
			return 0;
		}
	)mrk";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_BasicComment") {
	const auto testProgram =
		"// This is a comment\n"
		"i32 main() {"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_ComplexComment") {
	const auto testProgram =
		"//// Blah //a//a//a This is a comment /\n"
		"i32 main() {"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_InvalidComment") {
	const auto testProgram =
		"a // This is a comment\n"
		"i32 main() {"
		"}";

	REQUIRE_FALSE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionSingleDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionSingleDecl_NameCheck1") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i09za_ = 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionSingleDecl_NameCheck2") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i' = 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionMultiDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  i32 j = 0;"
		"  i32 k = 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionDeclAssign") {
	const auto testProgram =
		"i32 main() {"
		"  i32 r = 1 + 2;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionMultiDeclAssign") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionReturn") {
	const auto testProgram =
		"i32 main() {"
		"  return 1;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionReturnComplex") {
	const auto testProgram =
		"i32 main() {"
		"  return a + b + c + 0 + 1 + d;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_MultipleFunction") {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_MultipleComplexFunction") {
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

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionArgs") {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionCall") {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {"
		"  foo();"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionCallArgs") {
	const auto testProgram =
		"i32 foo(i32 a) {}"
		"i32 main() {"
		"  foo(45);"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionCallArgsComplex") {
	const auto testProgram =
		"i32 bar(i32 a, i32 b) {}"
		"i32 foo(i32 a) {"
		"  return bar(a, 5);"
		"}"
		"i32 main() {"
		"  return foo(45);"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionIfStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FunctionIfElseStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"  } else {"
		"  }"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_Assignment") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  a = a + 1;"
		"  return a;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_WhileStmt") {
	const auto testProgram =
		"i32 main() {"
		"  while (i < 4) {"
		"    if (i > 5) {"
		"    } else {"
		"    }"
		"  }"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_LogicalOR") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 0;"
		"  if (a || b) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_Division") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 / 3;"
		"  return i;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_Subtraction") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 5 - 3;"
		"  return i;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_RightShift") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 257 >> 8;"
		"  return i;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_Multiplication") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 8 * 4;"
		"  return i;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_ComplexEulerProblem1") {
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

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_FuncCallInIfStmt") {
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

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_LogicalAnd") {
	const auto testProgram =
		"i32 main() {"
		"  while ((0 != 1) && (0 != 1)) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_String") {
	const auto testProgram =
		"i32 main() {"
		"  printf(\"string test\");"
		"  return 0;"
		"}";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_PrimitiveType_i64") {
	const auto testProgram = R"mrk(
		i64 main() {
		  return 0;
		}
	)mrk";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_UserDefinedType_Basic") {
	const auto testProgram = R"mrk(
		type MyType {
			i64 a;
			i64 b;
		}

		i64 main() {
		  return 0;
		}
	)mrk";

	REQUIRE(parse(testProgram));
}

TEST_CASE("ParserTest_UserDefinedType_UseBasic") {
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

	REQUIRE(parse(testProgram));
}
