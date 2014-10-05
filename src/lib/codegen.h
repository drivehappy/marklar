#pragma once

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>

#include "parser.h"


namespace marklar {

	struct ast_codegen {
		void operator()(llvm::Module* module, llvm::IRBuilder<>& builder, const parser::base_expr* expr) const;
		void operator()(llvm::Module* module, llvm::IRBuilder<>& builder, const parser::func_expr* expr) const;

	};

}

