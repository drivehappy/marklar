#pragma once

#include "parser.h"

namespace marklar {

	struct ast_codegen {
		void operator()(const parser::base_expr* expr) const;
		void operator()(const parser::func_expr* expr) const;

	};

}

