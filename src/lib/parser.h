#pragma once

#include <boost/variant/recursive_variant.hpp>

#include <string>

namespace parser {

	struct base_expr;
	struct func_expr;
	struct decl_expr;
	struct operator_expr;
	struct call_expr;
	struct return_expr;
	struct if_expr;

	typedef boost::variant<
		boost::recursive_wrapper<base_expr>,
		boost::recursive_wrapper<func_expr>,
		boost::recursive_wrapper<decl_expr>,
		boost::recursive_wrapper<operator_expr>,
		boost::recursive_wrapper<call_expr>,
		boost::recursive_wrapper<return_expr>,
		boost::recursive_wrapper<if_expr>,
		std::string
	> base_expr_node;

	struct base_expr {
		std::vector<base_expr_node> children;
	};

	struct func_expr {
		std::string functionName;
		std::vector<std::string> args;
		std::vector<base_expr_node> declarations;
		std::vector<base_expr_node> expressions;
		base_expr_node retExpr;
	};

	struct decl_expr {
		std::string declName;
		base_expr_node val;
	};

	struct operator_expr {
		std::string valLHS;
		base_expr_node valRHS;
	};

	struct return_expr {
		base_expr_node ret;
	};
	
	struct call_expr {
		std::string funcName;
		std::vector<std::string> values;
	};

	struct if_expr {
		operator_expr condition;
		std::vector<base_expr_node> thenBranch;
		std::vector<base_expr_node> elseBranch;
	};

}


namespace marklar {

	bool parse(const std::string& str);

	bool parse(const std::string& str, parser::base_expr_node& root);

}

