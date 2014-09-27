#include "parser.h"

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
	struct basic_expr;

	typedef boost::variant<
		boost::recursive_wrapper<base_expr>,
		boost::recursive_wrapper<func_expr>,
		boost::recursive_wrapper<decl_expr>,
		boost::recursive_wrapper<operator_expr>,
		boost::recursive_wrapper<basic_expr>,
		std::string
	> base_expr_node;

	struct base_expr {
		std::vector<base_expr_node> children;
	};

	struct func_expr {
		std::string functionName;
		std::vector<base_expr_node> declarations;
		std::vector<base_expr_node> expressions;
	};

	struct decl_expr {
		std::string declName;
		base_expr_node val;
	};

	struct operator_expr {
		std::string valLHS;
		std::string valRHS;
		std::string op;
	};

	struct basic_expr {
		base_expr_node expressionRHS;
	};

	struct return_expr {
		base_expr_node ret;
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
	(std::vector<parser::base_expr_node>, declarations)
	(std::vector<parser::base_expr_node>, expressions)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::operator_expr,
	(std::string, valLHS)
	(std::string, valRHS)
	(std::string, op)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::basic_expr,
	(parser::base_expr_node, expressionRHS)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::return_expr,
	(parser::base_expr_node, ret)
)


namespace parser {

	template <typename Iterator>
	struct marklar_grammar : qi::grammar<Iterator, base_expr_node(), qi::space_type>
	{
		marklar_grammar() : marklar_grammar::base_type(start)
		{
			start %= funcExpr;

			baseNode %= funcExpr;

			funcExpr %=
				  "int"
				>> varName
				>> '(' >> ')'
				>> '{'
				>> *decl
				>> *baseNode
				>> -returnExpr
				>> '}'
				;

			decl %=
				  "int"
				>> varName
				>> -('=' >> (op_expr | value))
				>> ';'
				;

			op_expr %= value >> op >> value;

			basicExpr %= intLiteral | op_expr;

			returnExpr %=
				   "return"
				>> (op_expr | value)
				>> ';'
				;

			varName %= qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			intLiteral %= +qi::char_("0-9");
			value %= (varName | intLiteral);
			op %= '+';
		}

		qi::rule<Iterator, base_expr_node(), qi::space_type> start;
		qi::rule<Iterator, base_expr_node(), qi::space_type> baseNode;

		qi::rule<Iterator, func_expr(), qi::space_type> funcExpr;
		qi::rule<Iterator, decl_expr(), qi::space_type> decl;
		qi::rule<Iterator, operator_expr(), qi::space_type> op_expr;
		qi::rule<Iterator, basic_expr(), qi::space_type> basicExpr;
		qi::rule<Iterator, return_expr(), qi::space_type> returnExpr;

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

