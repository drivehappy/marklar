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


using namespace std;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;


BOOST_FUSION_ADAPT_STRUCT(
	parser::operation,
	(std::string, op)
	(parser::base_expr_node, rhs)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::binary_op,
	(parser::base_expr_node, lhs)
	(std::vector<parser::operation>, operation)
)

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
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::operator_expr,
	(std::string, valLHS)
	(std::vector<std::string>, op_and_valRHS)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::return_expr,
	(parser::base_expr_node, ret)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::call_expr,
	(std::string, funcName)
	(std::vector<parser::base_expr_node>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::if_expr,
	(parser::binary_op, condition)
	(std::vector<parser::base_expr_node>, thenBranch)
	(std::vector<parser::base_expr_node>, elseBranch)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::while_loop,
	(parser::binary_op, condition)
	(std::vector<parser::base_expr_node>, loopBody)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::var_assign,
	(std::string, varName)
	(parser::base_expr_node, varRhs)
)


namespace parser {

	template <typename Iterator>
	struct marklar_grammar : qi::grammar<Iterator, base_expr_node(), qi::space_type>
	{
		marklar_grammar() : marklar_grammar::base_type(start)
		{
			start %= rootNode;

			rootNode %= qi::eps >> +funcExpr >> qi::eoi;

			funcExpr %=
				  "marklar"
				>> varName
				>> '(' >> *(varDef % ',') >> ')'
				>> '{'
				>> *varDecl
				>> *baseExpr
				>> -returnExpr
				>> '}'
				;

			varDef %=
				  "marklar"
				>> varName
				;

			varDecl %=
				   varDef
				>> -('=' >> (op_expr | value))
				>> ';'
				;

			op_expr %=
				   factor
				>> *(op >> factor);

			factor %=
				  qi::lit('(') >> op_expr >> ')'
				| callExpr
				| value;

			baseExpr %= intLiteral | returnExpr | (callExpr >> ';') | ifExpr | varDecl | varAssign | whileLoop;

			// Small hack to only allow op_expr, but allow boost::fusion to use
			// the base_node_expr type still (if we didn't, then baseExpr would
			// be used, and it would parse odd things)
			callBaseExpr %= op_expr;

			callExpr %=
				   varName
				>> '(' >> *(callBaseExpr % ',') >> ')'
				;

			returnExpr %=
				   "return"
				>> (callExpr | op_expr | value)
				>> ';'
				;

			ifExpr %=
				   qi::lit("if")
				>> '('
				>> op_expr
				>> ')' >> '{'
				>> *baseExpr
				>> '}'
				>> -(qi::lit("else") >> '{' >> *baseExpr >> '}')
				;

			whileLoop %=
				   qi::lit("while")
				>> '('
				>> op_expr
				>> ')' >> '{'
				>> *baseExpr
				>> '}'
				;

			varAssign %=
				   varName
				>> ('=' >> (op_expr | value))
				>> ';'
				;

			varName %= qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			intLiteral %= +qi::char_("0-9");
			value %= (varName | intLiteral);

			// '>>' before the next '>' or else it will be matched as greater-than
			op %=
				  ascii::string(">>")
				| ascii::string("<<")
				| ascii::string(">=")
				| ascii::string("<=")
				| ascii::string("!=")
				| ascii::string("==")
				| ascii::string("||")
				| ascii::string("&&")
				| qi::char_("+<>%/*&")
				| qi::char_("\\-")
				;

			// Debugging
			/*
			BOOST_SPIRIT_DEBUG_NODE(funcExpr);
			BOOST_SPIRIT_DEBUG_NODE(varDecl);
			BOOST_SPIRIT_DEBUG_NODE(baseExpr);
			BOOST_SPIRIT_DEBUG_NODE(ifExpr);
			BOOST_SPIRIT_DEBUG_NODE(op_expr);
			BOOST_SPIRIT_DEBUG_NODE(returnExpr);
			BOOST_SPIRIT_DEBUG_NODE(callExpr);
			BOOST_SPIRIT_DEBUG_NODE(op);
			BOOST_SPIRIT_DEBUG_NODE(value);
			BOOST_SPIRIT_DEBUG_NODE(intLiteral);
			BOOST_SPIRIT_DEBUG_NODE(factor);
			BOOST_SPIRIT_DEBUG_NODE(whileLoop);
			*/
		}

		qi::rule<Iterator, base_expr_node(), qi::space_type> start;
		qi::rule<Iterator, base_expr(), qi::space_type> rootNode;

		qi::rule<Iterator, func_expr(), qi::space_type> funcExpr;
		qi::rule<Iterator, decl_expr(), qi::space_type> varDecl;
		qi::rule<Iterator, string(), qi::space_type> varDef;
		qi::rule<Iterator, binary_op(), qi::space_type> op_expr;
		qi::rule<Iterator, base_expr_node(), qi::space_type> baseExpr;
		qi::rule<Iterator, base_expr_node(), qi::space_type> callBaseExpr;
		qi::rule<Iterator, return_expr(), qi::space_type> returnExpr;
		qi::rule<Iterator, call_expr(), qi::space_type> callExpr;
		qi::rule<Iterator, if_expr(), qi::space_type> ifExpr;
		qi::rule<Iterator, while_loop(), qi::space_type> whileLoop;
		qi::rule<Iterator, var_assign(), qi::space_type> varAssign;

		qi::rule<Iterator, base_expr_node(), qi::space_type> factor;
		qi::rule<Iterator, std::string(), qi::space_type> varName;
		qi::rule<Iterator, std::string(), qi::space_type> intLiteral;
		qi::rule<Iterator, std::string(), qi::space_type> value;
		qi::rule<Iterator, std::string(), qi::space_type> op;
	};

}

namespace marklar {

	bool parse(const std::string& str, parser::base_expr_node& root) {
		parser::marklar_grammar<std::string::const_iterator> p;
		const bool r = qi::phrase_parse(str.begin(), str.end(), p, qi::space, root);

		return r;
	}

	bool parse(const std::string& str) {
		parser::base_expr_node root;

		return parse(str, root);
	}

}

