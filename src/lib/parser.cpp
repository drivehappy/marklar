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
		qi::unused_type dummy;
	};

	struct func_expr {
		std::string functionName;
//		qi::unused_type dummy;
		std::vector<base_expr_node> declarations;
		std::vector<base_expr_node> expressions;
	};

	struct decl_expr {
		std::string declName;
		std::string val;
	};

	struct operator_expr {
		std::string valLHS;
		std::string valRHS;
		std::string op;
	};

	struct basic_expr {
		base_expr_node expressionRHS;
		qi::unused_type dummy;
	};
	
}


BOOST_FUSION_ADAPT_STRUCT(
	parser::base_expr,
	(std::vector<parser::base_expr_node>, children)
	(qi::unused_type, dummy)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::decl_expr,
	(std::string, declName)
	(std::string, val)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::func_expr,

	(std::string, functionName)
//	(qi::unused_type, dummy)

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
	(qi::unused_type, dummy)
)

namespace parser {

	template <typename Iterator>
	struct marklar_grammar : qi::grammar<Iterator, base_expr_node(), qi::space_type>
	{
		marklar_grammar() : marklar_grammar::base_type(start)
		{

			start %=
				op_expr
				;

			baseNode %= funcExpr;

			funcExpr %=
				  "int"
				> varName
				> '(' > ')'
				> '{'
				> *baseNode
				> *baseNode
				> '}'
				;

			decl %=
				  "int"
				> varName
				> -('=' > value)
				> ';'
				;

			op_expr %= value > op > value;

			basicExpr %= intLiteral || op_expr;

			varName %= qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			intLiteral %= +qi::char_("0-9");
			value %= (varName | intLiteral);
			op %= qi::lit('+');
		}

		qi::rule<Iterator, base_expr_node(), qi::space_type> start;
		qi::rule<Iterator, base_expr_node(), qi::space_type> baseNode;

		qi::rule<Iterator, func_expr(), qi::space_type> funcExpr;
		qi::rule<Iterator, decl_expr(), qi::space_type> decl;
		qi::rule<Iterator, operator_expr(), qi::space_type> op_expr;
		qi::rule<Iterator, basic_expr(), qi::space_type> basicExpr;

		qi::rule<Iterator, std::string(), qi::space_type> varName;
		qi::rule<Iterator, std::string(), qi::space_type> intLiteral;
		qi::rule<Iterator, std::string(), qi::space_type> value;
		qi::rule<Iterator, std::string(), qi::space_type> op;

	};

}

namespace marklar {

	void parse(const std::string& str) {
		/*
			int main() {
				int i = 0;
				int j = 0;
				int r = i + j;

				return r;
			}
		*/

		cout << "Parsing: " << endl << str << endl;

		parser::base_expr_node root;
		parser::marklar_grammar<std::string::const_iterator> p;
		const bool r = qi::phrase_parse(str.begin(), str.end(), p, qi::space, root);
		//const bool r = qi::parse(str.begin(), str.end(), p, root);
		if (!r) {
			cout << "Parsing failed." << endl;
			return;
		}

		cout << "Parse success" << endl;
	}

	double parseTest(const std::string& str) {
		double result = 0.0;
		/*
		const auto r = parse_numbers(str.begin(), str.end(), result);
		cout << "Parse numbers: " << r << endl;
		*/
		
		return result;
	}
}
