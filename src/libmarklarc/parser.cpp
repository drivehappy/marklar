//#define BOOST_SPIRIT_X3_DEBUG

#include "parser.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <regex>
#include <string>
#include <vector>


using namespace std;

namespace x3 = boost::spirit::x3;


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
	(std::string, typeName)
	(std::string, declName)
	(parser::base_expr_node, val)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::def_expr,
	(std::string, typeName)
	(std::string, defName)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::func_expr,
	(std::string, returnType)
	(std::string, functionName)
	(std::vector<parser::base_expr_node>, args)
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

	// Skip parser for handling comments
	namespace skipper {

		const x3::rule<class startSkip, std::string> 		startSkip = "startSkip";
		
		const auto comment = x3::confix("//", x3::eol);

		const auto startSkip_def = 
			  x3::space
			| comment[*(x3::char_ - x3::eol)]
			;

		BOOST_SPIRIT_DEFINE(startSkip);
	}

	namespace marklar {
		// Rules decls
		const x3::rule<class start, base_expr_node>			start = "start";
		const x3::rule<class rootNode, base_expr>   		rootNode = "rootNode";
		const x3::rule<class funcExpr, func_expr>   		funcExpr = "funcExpr";
		const x3::rule<class baseExpr, base_expr_node> 		baseExpr = "baseExpr";
		const x3::rule<class callBaseExpr, base_expr_node> 	callBaseExpr = "callBaseExpr";
		const x3::rule<class returnExpr, return_expr>  		returnExpr = "returnExpr";
		const x3::rule<class op_expr, binary_op>     		op_expr = "op_expr";
		const x3::rule<class op, std::string>      			op = "op";
		const x3::rule<class callExpr, call_expr>   		callExpr = "callExpr";
		const x3::rule<class ifExpr, if_expr>       		ifExpr = "ifExpr";
		const x3::rule<class whileLoop, while_loop> 		whileLoop = "whileLoop";

		const x3::rule<class varName, std::string>  		varName = "varName";
		const x3::rule<class varDef, def_expr>				varDef = "varDef";
		const x3::rule<class varDecl, decl_expr>    		varDecl = "varDecl";
		const x3::rule<class varAssign, var_assign> 		varAssign = "varAssign";
		const x3::rule<class value, std::string>      		value = "value";
		const x3::rule<class factor, base_expr_node>  		factor = "factor";
		const x3::rule<class intLiteral, std::string> 		intLiteral = "intLiteral";
		const x3::rule<class quotedString, std::string> 	quotedString = "quotedString";

		const x3::rule<class typeName, std::string>			typeName = "typeName";

		
		// Rules defs
		const auto start_def = rootNode;

		const auto rootNode_def = x3::eps >> +funcExpr >> x3::eoi;

		const auto funcExpr_def =
			   typeName
			>> varName
			>> '(' >> *(varDef % ',') >> ')'
			>> '{'
			>> *baseExpr
			>> '}'
			;

		const auto varDecl_def =
			   typeName
			>> varName
			>> -('=' >> (op_expr | value))
			>> ';'
			;

		const auto varDef_def =
			   typeName
			>> varName
			;

		const auto op_expr_def =
			   factor
			>> *(op >> factor);

		const auto factor_def =
			  x3::lit('(') >> op_expr >> ')'
			| callExpr
			| value
			| quotedString
			;

		const auto quotedString_def =
			  x3::lexeme[x3::char_("\"") >> *(x3::char_ - "\"") >> x3::char_("\"")]
			;

		const auto baseExpr_def = intLiteral | returnExpr | (callExpr >> ';') | ifExpr | (varDef >> ';') | varDecl | varAssign | whileLoop;

		// Small hack to only allow op_expr, but allow boost::fusion to use
		// the base_node_expr type still (if we didn't, then baseExpr would
		// be used, and it would parse odd things)
		const auto callBaseExpr_def = op_expr;

		const auto callExpr_def =
			   varName
			>> '(' >> *(callBaseExpr % ',') >> ')'
			;

		const auto returnExpr_def =
			   "return"
			>> (callExpr | op_expr | value)
			>> ';'
			;

		const auto ifExpr_def =
			   x3::lit("if")
			>> '('
			>> op_expr
			>> ')' >> '{'
			>> *baseExpr
			>> '}'
			>> -(x3::lit("else") >> '{' >> *baseExpr >> '}')
			;

		const auto whileLoop_def=
			   x3::lit("while")
			>> '('
			>> op_expr
			>> ')' >> '{'
			>> *baseExpr
			>> '}'
			;

		const auto varAssign_def =
			   varName
			>> ('=' >> (op_expr | value))
			>> ';'
			;

		const auto varName_def = x3::lexeme[x3::char_("a-zA-Z_") >> *x3::char_("a-zA-Z_0-9'")];
		const auto intLiteral_def = +x3::char_("0-9");
		const auto value_def = (varName | intLiteral);

		// '>>' before the next '>' or else it will be matched as greater-than
		const auto op_def =
			  x3::string(">>")
			| x3::string("<<")
			| x3::string(">=")
			| x3::string("<=")
			| x3::string("!=")
			| x3::string("==")
			| x3::string("||")
			| x3::string("&&")
			| -x3::char_("+<>%/*&-")
			;

		const auto typeName_def = x3::lexeme[x3::char_("a-zA-Z_") >> *x3::char_("a-zA-Z_0-9")];

		BOOST_SPIRIT_DEFINE(
			start,
			rootNode,
			funcExpr,
			baseExpr,
			callBaseExpr,
			returnExpr,
			op_expr,
			op,
			callExpr,
			ifExpr,
			whileLoop,
			varName,
			varDef,
			varDecl,
			varAssign,
			value,
			factor,
			intLiteral,
			quotedString,
			typeName
		);
	}
}

namespace marklar {

	bool parse(const std::string& str, parser::base_expr_node& root) {
		return x3::phrase_parse(str.begin(), str.end(), parser::marklar::start, parser::skipper::startSkip, root);
	}

	bool parse(const std::string& str) {
		parser::base_expr_node root;

		return parse(str, root);
	}

}

