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

	// We expect verifyModule to fail (returns true), since we don't have a return
	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
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

	// We expect verifyModule to fail (returns true), since we don't have a return
	auto module = codegenTest(root);
	EXPECT_TRUE(verifyModule(*module, &errorOut)) << errorInfo;
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

	module->dump();
}

