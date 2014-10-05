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

TEST(Codegen, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	// Begin the code generation using LLVM and our AST
	auto module = codegenTest(root);

	EXPECT_TRUE(verifyModule(*module));
}
 
