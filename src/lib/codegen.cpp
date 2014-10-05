#include "codegen.h"

#include <string>
#include <vector>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

// Debugging
#include <iostream>


using namespace marklar;
using namespace parser;

using namespace llvm;


void ast_codegen::operator()(const std::string& expr) {

}

void ast_codegen::operator()(const parser::base_expr& expr) {
	//assert(expr);

	/*
	for (base_expr_node& itr : expr->children) {
		boost::apply_visitor(ast_codegen(m_module, m_builder), &itr));
	}
	*/
}

void ast_codegen::operator()(const parser::func_expr& func) {
	//std::cerr << "Codegen: func_expr" << std::endl;

	Function *F = nullptr;
	std::vector<Type*> args(func.args.size(), Type::getInt32Ty(getGlobalContext()));

	// Determine if this function name has been defined yet
	auto itr = m_symbolTable.find(func.functionName);
	if (itr == m_symbolTable.end()) {
		// Could not find existing function with this name, build it
		FunctionType *FT = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
		F = Function::Create(FT, Function::ExternalLinkage, func.functionName, m_module);

		// Add it to the symbol table so we can refer to it later
		m_symbolTable[func.functionName] = F;
	} else {
		F = dynamic_cast<Function*>(itr->second);
	}

	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	m_builder.SetInsertPoint(BB);

	// Add function arguments
	Function::arg_iterator argItr = F->arg_begin();
	
}

void ast_codegen::operator()(const parser::decl_expr& expr) {

}

void ast_codegen::operator()(const parser::operator_expr& expr) {

}

void ast_codegen::operator()(const parser::return_expr& expr) {

}

void ast_codegen::operator()(const parser::call_expr& expr) {

}

void ast_codegen::operator()(const parser::if_expr& expr) {

}

void ast_codegen::operator()(const parser::binary_op& expr) {

}

