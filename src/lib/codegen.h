#pragma once

#include <map>
#include <string>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "parser.h"


namespace marklar {

	class ast_codegen : public boost::static_visitor<> {
	public:
		ast_codegen(llvm::Module* m, llvm::IRBuilder<>& b)
		: m_module(m), m_builder(b)
		{}

		void operator()(const parser::base_expr& expr);
		void operator()(const std::string& expr);
		void operator()(const parser::func_expr& expr);
		void operator()(const parser::decl_expr& expr);
		void operator()(const parser::operator_expr& expr);
		void operator()(const parser::return_expr& expr);
		void operator()(const parser::call_expr& expr);
		void operator()(const parser::if_expr& expr);
		void operator()(const parser::binary_op& expr);

	private:
		llvm::Module* m_module;
		llvm::IRBuilder<>& m_builder;

		std::map<std::string, llvm::Value*> m_symbolTable;
	};

}

