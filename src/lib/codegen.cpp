#include "codegen.h"

#include <map>
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
using namespace std;


namespace {
	bool is_number(const string& s) {
		return !s.empty() && find_if(s.begin(),
			s.end(), [](char c) { return !isdigit(c); }) == s.end();
	}
}


Value* ast_codegen::operator()(const string& val) {
	//cerr << "Generating code for string \"" << val << "\"" << endl;

	Value *retVal = nullptr;

	map<string, Value*>::iterator itr = m_symbolTable.find(val);
	if (itr != m_symbolTable.end()) {
		Value *localVar = itr->second;
		retVal = m_builder.CreateLoad(localVar);

		//return m_builder.CreateRet(retVal);
	} else if (is_number(val)) {
		APInt vInt(32, stoi(val));
		retVal = ConstantInt::get(getGlobalContext(), vInt);
		
		//return m_builder.CreateRet(v);
	} else {
		cerr << "ERROR: Could not find symbol: \"" << val << "\"" << endl;
		cerr << "  SymbolTable size: " << m_symbolTable.size() << endl;

		for (const auto& kv : m_symbolTable) {
			cerr  << "    Key: " << kv.first << ", Value: " << kv.second << endl;
		}

		cerr << endl;
	}

	return retVal;
}

Value* ast_codegen::operator()(const parser::base_expr& expr) {
	//cerr << "Generating code for base_expr, children size = \"" << expr.children.size() << "\"" << endl;

	//assert(expr);

	/*
	for (base_expr_node& itr : expr->children) {
		boost::apply_visitor(ast_codegen(m_module, m_builder), &itr));
	}
	*/

	return nullptr;
}

Value* ast_codegen::operator()(const parser::func_expr& func) {
	//cerr << "Generating code for Function \"" << func.functionName << "\"" << endl;

	Function *F = nullptr;
	vector<Type*> args(func.args.size(), Type::getInt32Ty(getGlobalContext()));

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
	for (auto& argStr : func.args) {
		argItr->setName(argStr);

		m_symbolTable[argStr] = argItr;
	}

	// Visit declarations inside the function node
	for (auto& itrDecl : func.declarations) {
		boost::apply_visitor(*this, itrDecl);
	}

	// Visit expressions inside the function node
	for (auto& itrExpr : func.expressions) {
		boost::apply_visitor(*this, itrExpr);
	}

	// LLVM sanity check
	verifyFunction(*F);

	return nullptr;
}

Value* ast_codegen::operator()(const parser::decl_expr& decl) {
	//cerr << "Generating code for declaration \"" << decl.declName << "\"" << endl;

	Value* var = nullptr;

	map<string, Value*>::const_iterator itr = m_symbolTable.find(decl.declName);
	if (itr == m_symbolTable.end()) {
		//cerr << "  Variable referenced for first time: " << decl.declName << endl;

		BasicBlock *bb = m_builder.GetInsertBlock();
		AllocaInst *Alloca = nullptr;

		// If there is no basic block it indicates it might be at the global-level
		if (bb) {
			Function *TheFunction = bb->getParent();
			IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
			Alloca = TmpB.CreateAlloca(Type::getInt32Ty(getGlobalContext()), nullptr, decl.declName.c_str());
			//Alloca->getType()->dump();

			m_symbolTable[decl.declName] = Alloca;
		} else {
			// Check if the function was already declared, if not then build it
			auto itr = m_symbolTable.find(decl.declName);
			if (itr == m_symbolTable.end()) {
				// Assume for now this has no arguments
				vector<Type*> args(0, Type::getInt32Ty(getGlobalContext()));

				FunctionType *FT = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
				Function *F = Function::Create(FT, Function::ExternalLinkage, decl.declName, m_module);

				// Add this function to the symbol table
				m_symbolTable[decl.declName] = F;
			}
		}

		var = Alloca;
	} else {
		// Use the variable itself
		var = itr->second;
	}

	Value* const exprRhs = boost::apply_visitor(*this, decl.val);
	if (exprRhs) {
		// Don't just obtain the variable from codegen, since that produces a load...
		// instead just look it up directly
		const auto itr = m_symbolTable.find(decl.declName);
		if (itr != m_symbolTable.end()) {
			if (exprRhs->getType()->isPointerTy()) {
				Value *varLhs = m_builder.CreateLoad(exprRhs);
				m_builder.CreateStore(varLhs, itr->second);
			} else {
				m_builder.CreateStore(exprRhs, itr->second);
			}
		} else {
			cerr << "ERROR: Could not find variable: " << decl.declName << endl;
			return nullptr;
		}
	}

	return var;
}

Value* ast_codegen::operator()(const parser::operator_expr& expr) {
	//cerr << "Generating code for operator" << endl;

	return nullptr;
}

Value* ast_codegen::operator()(const parser::return_expr& exprRet) {
	//cerr << "Generating code for return:" << endl;
	
	Value* v = boost::apply_visitor(*this, exprRet.ret);

	return m_builder.CreateRet(v);

	/*
	string* retVar = boost::get<string>(&exprRet.ret);
	assert(retVar);
	const string returnVar = *retVar;

	// Build the return here
	map<string, Value*>::iterator itr = m_symbolTable.find(returnVar);
	if (itr != m_symbolTable.end()) {
		Value *localVar = itr->second;
		Value *retVal = builder.CreateLoad(localVar);
		return builder.CreateRet(retVal);
	} else if (is_number(returnVar)) {
		// If string is a constant:
		APInt vInt(32, atoi(getData().c_str()));
		Value *v = ConstantInt::get(getGlobalContext(), vInt);
		
		return builder.CreateRet(v);
	} else {
		printf("ERROR: Could not find symbol: %s\n", mReturnExpr->data.c_str());
		printf("  SymbolTable size: %u\n", ASTHelper::symbolTable.size());

		for (const auto kv : ASTHelper::symbolTable) {
			printf("    Key: %s, Value: %p\n", kv.first.c_str(), kv.second);
		}
	}
	*/

	return nullptr;
}

Value* ast_codegen::operator()(const parser::call_expr& expr) {
	return nullptr;
}

Value* ast_codegen::operator()(const parser::if_expr& expr) {
	return nullptr;
}

Value* ast_codegen::operator()(const parser::binary_op& op) {
	cerr << "Generating code for binary_op:" << endl;

	Value* const varLhs = boost::apply_visitor(*this, op.lhs);
	assert(varLhs);

	for (auto& itr : op.operation) {
		Value* const varRhs = boost::apply_visitor(*this, itr.rhs);
		assert(varRhs);

		if (itr.op == "+") {
			return m_builder.CreateAdd(varLhs, varRhs);
		}
	}

	return varLhs;
}

