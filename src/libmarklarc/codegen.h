#pragma once

#include <map>
#include <string>
#include <tuple>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "parser.h"


namespace marklar {

	/* This is a visitor that walks the AST and for each node will generate the LLVM IR.
	 */
	class ast_codegen : public boost::static_visitor<llvm::Value*> {
	public:
		using symbolValue_t = std::map<std::string, llvm::Value*>;

		ast_codegen(llvm::LLVMContext* ctx, llvm::Module* m, llvm::IRBuilder<>& b)
		: m_context(ctx), m_module(m), m_builder(b) {}

		ast_codegen(const ast_codegen& rhs)
		: m_context(rhs.m_context), m_module(rhs.m_module), m_builder(rhs.m_builder), m_symbolTable(rhs.m_symbolTable) {}

		bool addSymbol(const std::string& name, llvm::Value* val) {
			const bool exists = (m_symbolTable.find(name) != m_symbolTable.end());

			if (!exists) {
				m_symbolTable[name] = val;
			}

			return !exists;
		}


		llvm::Value* operator()(const parser::base_expr& expr);
		llvm::Value* operator()(const std::string& expr);
		llvm::Value* operator()(const parser::func_expr& expr);
		llvm::Value* operator()(const parser::def_expr& expr);
		llvm::Value* operator()(const parser::decl_expr& expr);
		llvm::Value* operator()(const parser::operator_expr& expr);
		llvm::Value* operator()(const parser::return_expr& expr);
		llvm::Value* operator()(const parser::call_expr& expr);
		llvm::Value* operator()(const parser::if_expr& expr);
		llvm::Value* operator()(const parser::binary_op& expr);
		llvm::Value* operator()(const parser::while_loop& expr);
		llvm::Value* operator()(const parser::var_assign& expr);
		llvm::Value* operator()(const parser::udf_type& expr);

	private:
		llvm::LLVMContext* m_context;
		llvm::Module* m_module;
		llvm::IRBuilder<>& m_builder;

		symbolValue_t m_symbolTable;
	};

}

