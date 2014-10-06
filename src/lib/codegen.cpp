#include "codegen.h"

#include <map>
#include <string>
#include <vector>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

// Debugging
#include <iostream>


using namespace marklar;
using namespace parser;

using namespace llvm;
using namespace std;
using namespace std::placeholders;


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
		Value* const localVar = itr->second;

		// Only create a load if this is a pointer type, this avoids
		// problems with function arguments that aren't created through Alloca
		if (localVar->getType()->isPointerTy()) {
			retVal = m_builder.CreateLoad(localVar);
		} else {
			retVal = localVar;
		}
	} else if (is_number(val)) {
		APInt vInt(32, stoi(val));
		retVal = ConstantInt::get(getGlobalContext(), vInt);
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

	// Determine if this function name has been defined yet
	auto itr = m_symbolTable.find(func.functionName);
	if (itr == m_symbolTable.end()) {
		// Could not find existing function with this name, build it
		vector<Type*> args(func.args.size(), Type::getInt32Ty(getGlobalContext()));
		FunctionType *FT = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
		F = Function::Create(FT, Function::ExternalLinkage, func.functionName, m_module);

		// Add it to the symbol table so we can refer to it later
		m_symbolTable[func.functionName] = F;
	} else {
		F = dynamic_cast<Function*>(itr->second);
	}

	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), func.functionName.c_str(), F);
	m_builder.SetInsertPoint(BB);

	// Build a return value in place
	IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
	AllocaInst* const Alloca = TmpB.CreateAlloca(Type::getInt32Ty(getGlobalContext()), nullptr, "__retval__");
	assert(Alloca);
	m_symbolTable["__retval__"] = Alloca;

	APInt vInt(32, 0);
	m_builder.CreateStore(ConstantInt::get(getGlobalContext(), vInt), Alloca);

	BasicBlock *ReturnBB = BasicBlock::Create(getGlobalContext(), "return");
	m_symbolTable["__retval__BB"] = ReturnBB;


	// Add function arguments
	Function::arg_iterator argItr = F->arg_begin();
	for (auto& argStr : func.args) {
		argItr->setName(argStr);
		m_symbolTable[argStr] = argItr;
	}

	// Created a new visitor, this allows function-level scoping so our symbol table
	// isn't re-used across other functions
	ast_codegen symbolVisitor(*this);

	// Visit declarations inside the function node
	for (auto& itrDecl : func.declarations) {
		boost::apply_visitor(symbolVisitor, itrDecl);
	}

	// Visit expressions inside the function node
	for (auto& itrExpr : func.expressions) {
		boost::apply_visitor(symbolVisitor, itrExpr);
	}

	// Build the return inst
	F->getBasicBlockList().push_back(ReturnBB);
	m_builder.SetInsertPoint(ReturnBB);

	Value* const loadRetVal = m_builder.CreateLoad(m_symbolTable["__retval__"]);
	assert(loadRetVal);
	Value* const retVal = m_builder.CreateRet(loadRetVal);
	assert(retVal);

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
	
	// We can't generate a CreateRet in-place here since it might be
	// within an if-else, LLVM doesn't allow terminators in the branches
	// Therefore, just reference the return value on the stack we setup
	// when the function was created - we should only be writing to this once
	Value* const retVal = m_symbolTable["__retval__"];
	assert(retVal);

#if 0	
	if (m_returnGenerated) {
		/*
		APInt vInt(32, stoi(val));
		defaultRet = ConstantInt::get(getGlobalContext(), vInt);

		m_builder.CreateStore(defaultRet, retVal);
		*/
		return retVal;
	}
#endif

	Value* const v = boost::apply_visitor(*this, exprRet.ret);
	assert(v);

	Value* const n = m_builder.CreateStore(v, retVal);
	assert(n);

	Value* const ReturnBB = m_symbolTable["__retval__BB"];
	Value* const r = m_builder.CreateBr(dynamic_cast<BasicBlock*>(ReturnBB));
	assert(r);

	// This should be a pointer type, the function would have created this
	//assert(retVal->getType()->isPointerTy());

	//m_returnGenerated = true;

	return r;
}

Value* ast_codegen::operator()(const parser::call_expr& expr) {
	//cerr << "Generating code for call_expr:" << endl;

	const string& callFuncName = expr.funcName;
	Function *calleeF = m_module->getFunction(callFuncName);
	if (calleeF == nullptr) {
		cerr << "Error: Could not find function definition for \"" << callFuncName << "\"" << endl;
		return nullptr;
	}

	// Check that the number of arguments match with what we expect on the function
	if (calleeF->arg_size() != expr.values.size()) {
		cerr << "Error: Function call expected " << calleeF->arg_size() << " arguments, but got " << expr.values.size() << endl;
		return nullptr;
	}

	// Build the arguments
	std::vector<Value*> ArgsV;
	cerr << "  Building call arguments: " << expr.values.size() << endl;
	for (auto& exprArg : expr.values) {
		Value* const v = boost::apply_visitor(*this, exprArg);
		ArgsV.push_back(v);
	}

	CallInst *callInst = m_builder.CreateCall(calleeF, ArgsV, callFuncName);

	// Pass the call up so the value can be stored
	return callInst;
}

Value* ast_codegen::operator()(const parser::if_expr& expr) {
	//cerr << "Generating code for ifExpr:" << endl;

	Function *TheFunction = m_builder.GetInsertBlock()->getParent();
	assert(TheFunction);

	// Call the visitor directory for the binary_op
	Value* const CondV = (*this)(expr.condition);
	assert(CondV);

	// Create blocks for the then and else cases, insert the 'then' block at the
	// end of the function
	BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "if.then", TheFunction);
	BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "if.else");
	BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "if.end");

	m_builder.CreateCondBr(CondV, ThenBB, ElseBB);

	Value* ThenV = nullptr;
	Value* ElseV = nullptr;

	// Build the 'then' branch code
	{
		m_builder.SetInsertPoint(ThenBB);

		// Created a new visitor, this allows scoping so our symbol table
		// isn't re-used across other functions
		ast_codegen symbolVisitor(*this);

		for (const auto& itrThen : expr.thenBranch) {
			ThenV = boost::apply_visitor(symbolVisitor, itrThen);
			assert(ThenV);
		}

		// Small hack: Update our return flag if the 'then' branch created one
		//m_returnGenerated |= symbolVisitor.m_returnGenerated;

		if (!isa<BranchInst>(ThenV)) {
			m_builder.CreateBr(MergeBB);
		}

		ThenBB = m_builder.GetInsertBlock();
	}

	// Build the 'else' branch code
	{
		TheFunction->getBasicBlockList().push_back(ElseBB);
		m_builder.SetInsertPoint(ElseBB);

		// Created a new visitor, this allows scoping so our symbol table
		// isn't re-used across other functions
		ast_codegen symbolVisitor(*this);

		for (const auto& itrElse : expr.elseBranch) {
			ElseV = boost::apply_visitor(symbolVisitor, itrElse);
			assert(ElseV);
		}

		// Small hack: Update our return flag if the 'else' branch created one
		//m_returnGenerated |= symbolVisitor.m_returnGenerated;

		if (!ElseV || (ElseV && !isa<BranchInst>(ElseV))) {
			m_builder.CreateBr(MergeBB);
		}

		ElseBB = m_builder.GetInsertBlock();
	}

	// Finalize the condition, sets the insertion point so code after this is attached
	// to the "if.end" block
	TheFunction->getBasicBlockList().push_back(MergeBB);
	m_builder.SetInsertPoint(MergeBB);

	return MergeBB;
}

Value* ast_codegen::operator()(const parser::binary_op& op) {
	//cerr << "Generating code for binary_op:" << endl;

	// Mapping of operator to LLVM creation calls
	const map<string, std::function<Value*(Value*, Value*)>> ops = {
		{ "+",  bind(&IRBuilder<>::CreateAdd,     m_builder, _1, _2, "add", false, false) },
		{ "<",  bind(&IRBuilder<>::CreateICmpSLT, m_builder, _1, _2, "cmp") },
		{ ">",  bind(&IRBuilder<>::CreateICmpSGT, m_builder, _1, _2, "cmp") },
		{ "==", bind(&IRBuilder<>::CreateICmpEQ,  m_builder, _1, _2, "cmp") },
	};

	Value* varLhs = boost::apply_visitor(*this, op.lhs);
	assert(varLhs);

	// This acts a chain, e.g.: "1 + 3 + i + k", varLhs is built up for each
	for (auto& itr : op.operation) {
		Value* const varRhs = boost::apply_visitor(*this, itr.rhs);
		assert(varRhs);

		const auto& itr2 = ops.find(itr.op);
		if (itr2 == ops.end()) {
			cerr << "Unknown operator: \"" << itr.op << "\"" << endl;
			assert(false && "Unsupported operator");
			return nullptr;
		}

		// Call the mapped operator type to create the appropriate one
		varLhs = itr2->second(varLhs, varRhs);
	}

	return varLhs;
}

