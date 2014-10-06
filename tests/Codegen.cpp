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
		"int main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}
 
TEST(CodegenTest, FunctionSingleDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}
 
TEST(CodegenTest, FunctionSingleDeclReturn) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
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
		"int main() {"
		"  int i = 1 + 2;"
		"  int j = i + 2;"
		"  int k = i + j;"
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
		"int main() {"
		"  int i = 2;"
		"  int j = 5;"
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
		"int bar() {"
		"  int a = 0;"
		"  int b = 2;"
		"  return a + b + 0;"
		"}"
		"int foo() {"
		"  int a = 4;"
		"  return a + 0;"
		"}"
		"int main() {"
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
		"int main(int a, int b) {"
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
		"int bar(int a) {"
		"  int b = 2;"
		"  return 1 + b + 0;"
		"}"
		"int main(int a, int b) {"
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
		"int main(int a) {"
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
		"int foo(int a) {"
		"  return a + 1;"
		"}"
		"int main(int a) {"
		"  return foo(a);"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	auto module = codegenTest(root);
	EXPECT_FALSE(verifyModule(*module, &errorOut)) << errorInfo;
}




