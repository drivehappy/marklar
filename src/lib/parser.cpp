#include "parser.h"

#define BOOST_SPIRIT_DEBUG 

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <regex>
#include <string>
#include <vector>

// Debug
#include <iostream>


using namespace std;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;


namespace parser {

	struct base_expr;
	struct func_expr;
	struct decl_expr;
	struct operator_expr;
	struct call_expr;
	struct return_expr;

	typedef boost::variant<
		boost::recursive_wrapper<base_expr>,
		boost::recursive_wrapper<func_expr>,
		boost::recursive_wrapper<decl_expr>,
		boost::recursive_wrapper<operator_expr>,
		boost::recursive_wrapper<call_expr>,
		boost::recursive_wrapper<return_expr>,
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
	
}


BOOST_FUSION_ADAPT_STRUCT(
	parser::base_expr,
	(std::vector<parser::base_expr_node>, children)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::decl_expr,
	(std::string, declName)
	(parser::base_expr_node, val)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::func_expr,

	(std::string, functionName)
	(std::vector<std::string>, args)
	(std::vector<parser::base_expr_node>, declarations)
	(std::vector<parser::base_expr_node>, expressions)
	(parser::base_expr_node, retExpr)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::operator_expr,
	(std::string, valLHS)
	(parser::base_expr_node, valRHS)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::return_expr,
	(parser::base_expr_node, ret)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::call_expr,
	(std::string, funcName)
	(std::vector<std::string>, values)
)

namespace parser {

	template <typename Iterator>
	struct marklar_grammar : qi::grammar<Iterator, base_expr_node(), qi::space_type>
	{
		marklar_grammar() : marklar_grammar::base_type(start)
		{
			start %= rootNode;

			rootNode %= qi::eps >> +funcExpr;

			funcExpr %=
				  "int"
				>> varName
				>> '(' >> *(varDef % ',') >> ')'
				>> '{'
				>> *varDecl
				>> *baseExpr
				>> -returnExpr
				>> '}'
				;

			varDef %=
				  "int"
				>> varName
				;

			varDecl %=
				   varDef
				>> -('=' >> (op_expr | value))
				>> ';'
				;

			op_expr %= value >> +(op >> value);

			baseExpr %= intLiteral | callExpr;

			callExpr %=
				   varName
				>> '(' >> *(value % ',') >> ')'
				>> ';'
				;

			returnExpr %=
				   "return"
				>> (op_expr | value)
				>> ';'
				;

			varName %= qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			intLiteral %= +qi::char_("0-9");
			value %= (varName | intLiteral);
			op %= '+';

			// Debugging
			/*
			BOOST_SPIRIT_DEBUG_NODE(funcExpr);
			BOOST_SPIRIT_DEBUG_NODE(varDecl);
			BOOST_SPIRIT_DEBUG_NODE(baseExpr);
			BOOST_SPIRIT_DEBUG_NODE(returnExpr);
			*/
		}

		qi::rule<Iterator, base_expr_node(), qi::space_type> start;
		qi::rule<Iterator, base_expr(), qi::space_type> rootNode;

		qi::rule<Iterator, func_expr(), qi::space_type> funcExpr;
		qi::rule<Iterator, decl_expr(), qi::space_type> varDecl;
		qi::rule<Iterator, string(), qi::space_type> varDef;
		qi::rule<Iterator, operator_expr(), qi::space_type> op_expr;
		qi::rule<Iterator, base_expr_node(), qi::space_type> baseExpr;
		qi::rule<Iterator, return_expr(), qi::space_type> returnExpr;
		qi::rule<Iterator, call_expr(), qi::space_type> callExpr;

		qi::rule<Iterator, std::string(), qi::space_type> varName;
		qi::rule<Iterator, std::string(), qi::space_type> intLiteral;
		qi::rule<Iterator, std::string(), qi::space_type> value;
		qi::rule<Iterator, std::string(), qi::space_type> op;
	};

}

namespace marklar {

	bool parse(const std::string& str) {
		parser::base_expr_node root;
		parser::marklar_grammar<std::string::const_iterator> p;
		const bool r = qi::phrase_parse(str.begin(), str.end(), p, qi::space, root);

		return r;
	}

}

