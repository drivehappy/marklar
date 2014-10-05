#include "codegen.h"

#include <string>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>

using namespace marklar;
using namespace parser;

using namespace llvm;


void ast_codegen::operator()(const std::string& expr) const {

}

void ast_codegen::operator()(const parser::base_expr& expr) const {
	//assert(expr);

	/*
	for (base_expr_node& itr : expr->children) {
		boost::apply_visitor(ast_codegen(m_module, m_builder), &itr));
	}
	*/
}

void ast_codegen::operator()(const parser::func_expr& expr) const {
	
}

void ast_codegen::operator()(const parser::decl_expr& expr) const {

}

void ast_codegen::operator()(const parser::operator_expr& expr) const {

}

void ast_codegen::operator()(const parser::return_expr& expr) const {

}

void ast_codegen::operator()(const parser::call_expr& expr) const {

}

void ast_codegen::operator()(const parser::if_expr& expr) const {

}

void ast_codegen::operator()(const parser::binary_op& expr) const {

}

