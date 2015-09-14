#include <gtest/gtest.h>

#include <parser.h>
#include <codegen.h>

#include <memory>
#include <string>

#include <boost/variant/get.hpp>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using namespace marklar;
using namespace parser;

using namespace llvm;
using namespace std;


namespace {

	unique_ptr<Module> codegenTest(const base_expr_node& root) {
		LLVMContext &context = getGlobalContext();
		unique_ptr<Module> module(new Module("", context));
		IRBuilder<> builder(getGlobalContext());

		ast_codegen codeGenerator(module.get(), builder);

		// Codegen for each expression we've found in the root AST
		const base_expr* expr = boost::get<base_expr>(&root);
		for (auto& itr : expr->children) {
			boost::apply_visitor(codeGenerator, itr);
		}

		return module;
	}

}

TEST(CodegenTest, BasicFunction) {
	const auto testProgram =
		"i32 main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	// Expect this to fail since we require a 'return'
	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
}
 
TEST(CodegenTest, FunctionSingleDecl) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	// Expect this to fail since we require a 'return'
	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
}
 
TEST(CodegenTest, FunctionSingleDeclReturn) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  return i;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionMultiDeclAssign) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"  return k;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionMultiDeclSum) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return i + j;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, MultipleFunction) {
	const auto testProgram =
		"i32 bar() {"
		"  i32 a = 0;"
		"  i32 b = 2;"
		"  return a + b + 0;"
		"}"
		"i32 foo() {"
		"  i32 a = 4;"
		"  return a + 0;"
		"}"
		"i32 main() {"
		"  return 0 + 1;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionArgs) {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionMultipleArgs) {
	const auto testProgram =
		"i32 bar(i32 a) {"
		"  i32 b = 2;"
		"  return 1 + b + 0;"
		"}"
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionUseArgs) {
	const auto testProgram =
		"i32 main(i32 a) {"
		"  return a;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionCall) {
	const auto testProgram =
		"i32 foo(i32 a) {"
		"  return a + 1;"
		"}"
		"i32 main(i32 a) {"
		"  return foo(a);"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, IfElseStmt) {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  } else {"
		"    return 2;"
		"  }"
		"  return 3;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	auto module = codegenTest(root);

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	module->print(errorOut, nullptr);
	if (verifyModule(*module, &errorOut)) {
		cerr << "Error: " << errorInfo << endl;
	}

	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, OperatorGreaterThan) {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, OperatorEqual) {
	const auto testProgram =
		"i32 main() {"
		"  if (4 == 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, OperatorModulo) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 5 % 3;"
		"  if (a == 2) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, Assignment) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  a = a + 1;"
		"  return a;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, WhileStmt) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 2;"
		"  i32 b = 6;"
		"  while (a < b) {"
		"    a = a + 1;"
		"  }"
		"  return a;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, LogicalOR) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 0;"
		"  if ((a == 0) || (b == 0)) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, SameVariableNameDiffFunctions) {
	const auto testProgram =
		"i32 fib(i32 a) {"
		"  return a;"
		"}"
		"i32 main() {"
		"  i32 a = 10;"
		"  return fib(a);"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FuncCallInIfStmt) {
	const auto testProgram =
		"i32 func1(i32 a) {"
		"	return 0;"
		"}"
		"i32 main() {"
		"	if (func1(1) > 0) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, WhileWithReturnStmt) {
	const auto testProgram =
		"i32 main() {"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FuncWithEarlyReturnStmt) {
	const auto testProgram =
		"i32 main() {"
		"   return 2;"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FuncWithPrintf) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test");
		   return 0;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FuncWithPrintfArgs) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   return 0;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FuncWithPrintfArgsDifferTypes) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   printf("another call, different function type");
		   return 0;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, DuplicateDefinition) {
	// Failure expected, but a error should be generated
	const auto testProgram = R"mrk(
		i32 main() {
			i32 a;
			i32 a;
			return 0;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, DuplicateDefinitionInFunctionArgs) {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a, i32 a) {
			return 0;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, DuplicateDefinitionVarAndArg) {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a) {
			i32 a = 2;
			return a;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, Primitive_i64) {
	const auto testProgram = R"mrk(
		i64 main(i64 a) {
			i64 b = 2;
			return b;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, Operator_i32_i64) {
	const auto testProgram = R"mrk(
		i64 main(i32 a) {
			i64 b = a + 1;
			return b;
		}
		)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, CastIntegers) {
	const auto testProgram = R"mrk(
		i64 main(i64 n) {
			if (n != 2) {
				return 0;
			}

			return 1;
		}
	)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

TEST(CodegenTest, FunctionCallParameterType) {
	const auto testProgram = R"mrk(
		i64 isPrime(i64 n) {
			return 1;
		}

		i64 main() {
			return isPrime(1);
		}
	)mrk";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}

