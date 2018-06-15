#include <gtest/gtest.h>

#include <parser.h>
#include <codegen.h>

#include <memory>
#include <string>

#include <boost/variant/get.hpp>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using namespace marklar;
using namespace parser;

using namespace llvm;
using namespace std;


namespace {

	LLVMContext MyGlobalContext;

	unique_ptr<Module> codegenTest(const base_expr_node& root) {
		LLVMContext &context = MyGlobalContext;
		unique_ptr<Module> module(new Module("", context));
		IRBuilder<> builder(MyGlobalContext);

		ast_codegen codeGenerator(module.get(), builder);

		// Codegen for each expression we've found in the root AST
		const base_expr* expr = boost::get<base_expr>(&root);
		for (auto& itr : expr->children) {
			boost::apply_visitor(codeGenerator, itr);
		}

		return module;
	}


	class CodegenTest : public ::testing::Test {
	public:
		CodegenTest()
		:	m_errorOut(m_errorInfo)
		{}

		base_expr_node m_root;
		string m_errorInfo;
		raw_string_ostream m_errorOut;
	};

}

TEST_F(CodegenTest, BasicFunction) {
	const auto testProgram =
		"i32 main() {"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	// Expect this to fail since we require a 'return'
	auto module = codegenTest(m_root);
	EXPECT_TRUE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}
 
TEST_F(CodegenTest, FunctionSingleDecl) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	// Expect this to fail since we require a 'return'
	auto module = codegenTest(m_root);
	EXPECT_TRUE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}
 
TEST_F(CodegenTest, FunctionSingleDeclReturn) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  return i;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionMultiDeclAssign) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"  return k;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionMultiDeclSum) {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return i + j;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, MultipleFunction) {
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

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionArgs) {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionMultipleArgs) {
	const auto testProgram =
		"i32 bar(i32 a) {"
		"  i32 b = 2;"
		"  return 1 + b + 0;"
		"}"
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionUseArgs) {
	const auto testProgram =
		"i32 main(i32 a) {"
		"  return a;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionCall) {
	const auto testProgram =
		"i32 foo(i32 a) {"
		"  return a + 1;"
		"}"
		"i32 main(i32 a) {"
		"  return foo(a);"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, IfElseStmt) {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  } else {"
		"    return 2;"
		"  }"
		"  return 3;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, OperatorGreaterThan) {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, OperatorEqual) {
	const auto testProgram =
		"i32 main() {"
		"  if (4 == 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, OperatorModulo) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 5 % 3;"
		"  if (a == 2) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, Assignment) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  a = a + 1;"
		"  return a;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, WhileStmt) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 2;"
		"  i32 b = 6;"
		"  while (a < b) {"
		"    a = a + 1;"
		"  }"
		"  return a;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, LogicalOR) {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 0;"
		"  if ((a == 0) || (b == 0)) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, SameVariableNameDiffFunctions) {
	const auto testProgram =
		"i32 fib(i32 a) {"
		"  return a;"
		"}"
		"i32 main() {"
		"  i32 a = 10;"
		"  return fib(a);"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FuncCallInIfStmt) {
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

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, WhileWithReturnStmt) {
	const auto testProgram =
		"i32 main() {"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FuncWithEarlyReturnStmt) {
	const auto testProgram =
		"i32 main() {"
		"   return 2;"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FuncWithPrintf) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test");
		   return 0;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FuncWithPrintfArgs) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   return 0;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FuncWithPrintfArgsDifferTypes) {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   printf("another call, different function type");
		   return 0;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, DuplicateDefinition) {
	// Failure expected, but a error should be generated
	const auto testProgram = R"mrk(
		i32 main() {
			i32 a;
			i32 a;
			return 0;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_TRUE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, DuplicateDefinitionInFunctionArgs) {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a, i32 a) {
			return 0;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_TRUE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, DuplicateDefinitionVarAndArg) {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a) {
			i32 a = 2;
			return a;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, Primitive_i64) {
	const auto testProgram = R"mrk(
		i64 main(i64 a) {
			i64 b = 2;
			return b;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, Operator_i32_i64) {
	const auto testProgram = R"mrk(
		i64 main(i32 a) {
			i64 b = a + 1;
			return b;
		}
		)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, CastIntegers) {
	const auto testProgram = R"mrk(
		i64 main(i64 n) {
			if (n != 2) {
				return 0;
			}

			return 1;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionCallParameterType) {
	const auto testProgram = R"mrk(
		i64 isPrime(i64 n) {
			return 1;
		}

		i64 main() {
			return isPrime(1);
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, FunctionOperatorCast) {
	const auto testProgram = R"mrk(
		i64 main() {
			i64 a = 1 << 30;
			i64 b = 2 * a;
			i64 c = a * 2;

			return a;
		}
	)mrk";

	EXPECT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}

TEST_F(CodegenTest, InvalidType) {
	const auto testProgram = R"mrk(
		marklar main() {
			marklar a = 1 << 30;

			return a;
		}
	)mrk";

	ASSERT_TRUE(parse(testProgram, m_root));

	auto module = codegenTest(m_root);
	EXPECT_FALSE(verifyModule(*module, &m_errorOut)) << m_errorInfo;
}
