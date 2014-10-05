#include "codegen.h"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>

using namespace marklar;
using namespace llvm;


void ast_codegen::operator()(Module* module, IRBuilder<>& builder, const parser::base_expr* expr) const {
	
}

void ast_codegen::operator()(Module* module, IRBuilder<>& builder, const parser::func_expr* expr) const {
	
}

