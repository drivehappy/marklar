#include <gtest/gtest.h>

#include <parser.h>
#include <codegen.h>

#include <boost/variant/get.hpp>


using namespace marklar;
using namespace parser;


TEST(Codegen, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	ast_codegen codeGenerator;
	codeGenerator(expr);
}

