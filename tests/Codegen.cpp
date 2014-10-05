#include <gtest/gtest.h>

#include <parser.h>
#include <codegen.h>

#include <boost/variant/get.hpp>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace marklar;
using namespace parser;


TEST(Codegen, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	// Begin the code generation using LLVM and our AST
	LLVMContext &context = getGlobalContext();
	Module * module = new Module("Marklar LLVM Test", context);
	IRBuilder<> builder(getGlobalContext());

	ast_codegen codeGenerator(module, builder);

	module->dump();

	// Codegen for each expression we've found in the root AST
	base_expr* expr = boost::get<base_expr>(&root);
	for (auto& itr : expr->children) {
		boost::apply_visitor(codeGenerator, itr);
	}

	module->dump();
}
 
