#include "parser.h"

#include <regex>
#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

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
		string
	> base_expr_node;

	struct base_expr {
		vector<base_expr_node> children;
	};

	struct func_expr {
		string functionName;
		vector<decl_expr> declarations;
		//vector<base_expr_node> expressions;
	};

	struct decl_expr {
		string declName;
		string val;
	};

	struct operator_expr {
		string valLHS;
		string valRHS;
		string op;
	};

	struct basic_expr {
		base_expr_node expressionRHS;
	};

	
}

BOOST_FUSION_ADAPT_STRUCT(
	parser::base_expr,
	(vector<parser::base_expr_node>, children)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::func_expr,
	(string, functionName)
	//(vector<parser::decl_expr>, declarations)
	//(vector<parser::base_expr_node>, expressions)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::decl_expr,
	(string, declName)
	(string, val)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::operator_expr,
	(string, valLHS)
	(string, valRHS)
	(string, op)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::basic_expr,
	(parser::base_expr_node, expressionRHS)
)

namespace parser {

	template <typename Iterator>
	struct marklar_grammar : qi::grammar<Iterator, base_expr_node()>
	{
		marklar_grammar() : marklar_grammar::base_type(start)
		{

			start %= qi::eps
				>> baseNode
				;

			baseNode %= funcExpr;

			funcExpr %=
				   "int "
				>> variable
				>> "(){"
				//>> *decl
	//			>> *basicExpr
				>> "}"
				;

			decl %=
				   "int "
				>> variable
				>> -(" = " >> value)
				>> ";"
				;

			op_expr %= value >> op >> value;


			basicExpr %= intLiteral || op_expr;

			variable %= qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			intLiteral %= +qi::char_("0-9");
			value %= (variable | intLiteral);
			op %= qi::lit('+');
		}

		qi::rule<Iterator, base_expr_node()> start;
		qi::rule<Iterator, base_expr_node()> baseNode;

		qi::rule<Iterator, func_expr()> funcExpr;
		qi::rule<Iterator, decl_expr()> decl;
		qi::rule<Iterator, operator_expr()> op_expr;
		qi::rule<Iterator, basic_expr()> basicExpr;

		qi::rule<Iterator, string()> variable;
		qi::rule<Iterator, string()> intLiteral;
		qi::rule<Iterator, string()> value;
		qi::rule<Iterator, string()> op;

	};

}

namespace marklar {

	void parse(const string& str) {
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
		parser::marklar_grammar<string::const_iterator> p;
		const bool r = qi::parse(str.begin(), str.end(), p, root);
		if (!r) {
			cout << "Parsing failed." << endl;
			return;
		}

		cout << "Parse success" << endl;
	}

	double parseTest(const string& str) {
		double result = 0.0;
		/*
		const auto r = parse_numbers(str.begin(), str.end(), result);
		cout << "Parse numbers: " << r << endl;
		*/
		
		return result;
	}
}
