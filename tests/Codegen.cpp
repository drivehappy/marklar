#include "catch.hpp"

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

	unique_ptr<Module> codegenTest(LLVMContext& context, const base_expr_node& root) {
		unique_ptr<Module> module(new Module("", context));
		IRBuilder<> builder(context);

		ast_codegen codeGenerator(&context, module.get(), builder);

		// Codegen for each expression we've found in the root AST
		const base_expr* expr = boost::get<base_expr>(&root);
		for (auto& itr : expr->children) {
			boost::apply_visitor(codeGenerator, itr);
		}

		return module;
	}


	class CodegenTestFixture {
	public:
		CodegenTestFixture()
		:	m_errorOut(m_errorInfo)
		{}

		base_expr_node m_root;
		string m_errorInfo;
		raw_string_ostream m_errorOut;
	};

}

TEST_CASE_METHOD(CodegenTestFixture, "BasicFunction") {
	const auto testProgram =
		"i32 main() {"
		"}";

	CHECK(parse(testProgram, m_root));

	// Expect this to fail since we require a 'return'
	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK(verifyModule(*module, &m_errorOut));
}
 
TEST_CASE_METHOD(CodegenTestFixture, "FunctionSingleDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	CHECK(parse(testProgram, m_root));

	// Expect this to fail since we require a 'return'
	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK(verifyModule(*module, &m_errorOut));
}
 
TEST_CASE_METHOD(CodegenTestFixture, "FunctionSingleDeclReturn") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  return i;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionMultiDeclAssign") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"  return k;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionMultiDeclSum") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 2;"
		"  i32 j = 5;"
		"  return i + j;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "MultipleFunction") {
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

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionArgs") {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionMultipleArgs") {
	const auto testProgram =
		"i32 bar(i32 a) {"
		"  i32 b = 2;"
		"  return 1 + b + 0;"
		"}"
		"i32 main(i32 a, i32 b) {"
		"  return 1;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionUseArgs") {
	const auto testProgram =
		"i32 main(i32 a) {"
		"  return a;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionCall") {
	const auto testProgram =
		"i32 foo(i32 a) {"
		"  return a + 1;"
		"}"
		"i32 main(i32 a) {"
		"  return foo(a);"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "IfElseStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  } else {"
		"    return 2;"
		"  }"
		"  return 3;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "OperatorGreaterThan") {
	const auto testProgram =
		"i32 main() {"
		"  if (3 > 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "OperatorEqual") {
	const auto testProgram =
		"i32 main() {"
		"  if (4 == 4) {"
		"    return 1;"
		"  }"
		"  return 2;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "OperatorModulo") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 5 % 3;"
		"  if (a == 2) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "Assignment") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  a = a + 1;"
		"  return a;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "WhileStmt") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 2;"
		"  i32 b = 6;"
		"  while (a < b) {"
		"    a = a + 1;"
		"  }"
		"  return a;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "LogicalOR") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 0;"
		"  i32 b = 0;"
		"  if ((a == 0) || (b == 0)) {"
		"    return 2;"
		"  }"
		"  return 1;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "SameVariableNameDiffFunctions") {
	const auto testProgram =
		"i32 fib(i32 a) {"
		"  return a;"
		"}"
		"i32 main() {"
		"  i32 a = 10;"
		"  return fib(a);"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FuncCallInIfStmt") {
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

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "WhileWithReturnStmt") {
	const auto testProgram =
		"i32 main() {"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FuncWithEarlyReturnStmt") {
	const auto testProgram =
		"i32 main() {"
		"   return 2;"
		"	while (1 == 1) {"
		"		return 1;"
		"	}"
		"   return 0;"
		"}";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FuncWithPrintf") {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test");
		   return 0;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FuncWithPrintfArgs") {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   return 0;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FuncWithPrintfArgsDifferTypes") {
	const auto testProgram = R"mrk(
		i32 main() {
		   printf("test %d\n", 23);
		   printf("another call, different function type");
		   return 0;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "DuplicateDefinition") {
	// Failure expected, but a error should be generated
	const auto testProgram = R"mrk(
		i32 main() {
			i32 a;
			i32 a;
			return 0;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "DuplicateDefinitionInFunctionArgs") {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a, i32 a) {
			return 0;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "DuplicateDefinitionVarAndArg") {
	// Failure expected, a error should be generated
	const auto testProgram = R"mrk(
		i32 main(i32 a) {
			i32 a = 2;
			return a;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "Primitive_i64") {
	const auto testProgram = R"mrk(
		i64 main(i64 a) {
			i64 b = 2;
			return b;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "Operator_i32_i64") {
	const auto testProgram = R"mrk(
		i64 main(i32 a) {
			i64 b = a + 1;
			return b;
		}
		)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "CastIntegers") {
	const auto testProgram = R"mrk(
		i64 main(i64 n) {
			if (n != 2) {
				return 0;
			}

			return 1;
		}
	)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionCallParameterType") {
	const auto testProgram = R"mrk(
		i64 isPrime(i64 n) {
			return 1;
		}

		i64 main() {
			return isPrime(1);
		}
	)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "FunctionOperatorCast") {
	const auto testProgram = R"mrk(
		i64 main() {
			i64 a = 1 << 30;
			i64 b = 2 * a;
			i64 c = a * 2;

			return a;
		}
	)mrk";

	CHECK(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}

TEST_CASE_METHOD(CodegenTestFixture, "InvalidType") {
	const auto testProgram = R"mrk(
		marklar main() {
			marklar a = 1 << 30;

			return a;
		}
	)mrk";

	REQUIRE(parse(testProgram, m_root));

	LLVMContext context;
	auto module = codegenTest(context, m_root);
	INFO(m_errorInfo);
	CHECK_FALSE(verifyModule(*module, &m_errorOut));
}
