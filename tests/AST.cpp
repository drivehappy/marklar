#include <gtest/gtest.h>

#include <parser.h>

#include <map>
#include <string>
#include <tuple>

#include <boost/variant/get.hpp>

using namespace marklar;
using namespace parser;
using namespace std;


TEST(ASTTest, BasicFunction) {
	const auto testProgram =
		"int main() {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_TRUE(expr != nullptr);
	
	// We expect to have our func_expr in this root node
	EXPECT_EQ(1, expr->children.size());
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF != nullptr);

	// Check the function name
	EXPECT_EQ("main", exprF->functionName);
}

TEST(ASTTest, FunctionSingleDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_TRUE(expr != nullptr);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF != nullptr);

	// Check declarations
	EXPECT_EQ(1, exprF->declarations.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->declarations[0]);
	EXPECT_TRUE(decl != nullptr);

	EXPECT_EQ("i", decl->declName);

	binary_op* opExpr = boost::get<binary_op>(&decl->val);
	EXPECT_TRUE(opExpr != nullptr);

	EXPECT_EQ(0, opExpr->operation.size());
}

TEST(ASTTest, FunctionMultiDecl) {
	const auto testProgram =
		"int main() {"
		"  int i = 0;"
		"  int j = 1;"
		"  int k = 2;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);

	EXPECT_EQ(3, exprF->declarations.size());

	const map<string, string> expectedNameVals = {
		{"i", "0"},
		{"j", "1"},
		{"k", "2"}
	};

	for (auto& funcDecl : exprF->declarations) {
		// Check declarations
		decl_expr* decl = boost::get<decl_expr>(&funcDecl);
		EXPECT_TRUE(decl != nullptr);

		const auto itr = expectedNameVals.find(decl->declName);
		EXPECT_TRUE(itr != expectedNameVals.end());

		EXPECT_EQ(itr->first, decl->declName);

		//
		binary_op* opExpr = boost::get<binary_op>(&decl->val);
		EXPECT_TRUE(opExpr != nullptr);

		EXPECT_EQ(0, opExpr->operation.size());

		string* lhsVal = boost::get<string>(&opExpr->lhs);
		EXPECT_EQ(itr->second, *lhsVal);
	}
}

TEST(ASTTest, FunctionDeclAssign) {
	const auto testProgram =
		"int main() {"
		"  int r = 1 + 2;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);

	// Check declarations
	EXPECT_EQ(1, exprF->declarations.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->declarations[0]);
	EXPECT_TRUE(decl != nullptr);

	EXPECT_EQ("r", decl->declName);

	binary_op* opExpr = boost::get<binary_op>(&decl->val);
	EXPECT_TRUE(opExpr != nullptr);
	EXPECT_EQ(1, opExpr->operation.size());

	// Check decl value
	EXPECT_EQ("+", opExpr->operation[0].op);

	string* rhsVal = boost::get<string>(&opExpr->operation[0].rhs);
	EXPECT_EQ("2", *rhsVal);
}

TEST(ASTTest, FunctionMultiDeclAssign) {
	const auto testProgram =
		"int main() {"
		"  int i = 1 + 2;"
		"  int j = i + 2;"
		"  int k = i + j;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);

	// Check declarations
	EXPECT_EQ(3, exprF->declarations.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->declarations[0]);
	EXPECT_TRUE(decl != nullptr);

	EXPECT_EQ("i", decl->declName);

	const vector<tuple<string, string, string, string>> expectedNameVals = {
		make_tuple("i", "1", "+", "2"),
		make_tuple("j", "i", "+", "2"),
		make_tuple("k", "i", "+", "j"),
	};

	int index = 0;
	for (auto& funcDecl : exprF->declarations) {
		// Check declarations
		decl_expr* decl = boost::get<decl_expr>(&funcDecl);
		EXPECT_TRUE(decl != nullptr);

		const auto expectedValues = expectedNameVals[index];

		EXPECT_EQ(get<0>(expectedValues), decl->declName);

		binary_op* opExpr = boost::get<binary_op>(&decl->val);
		EXPECT_TRUE(opExpr != nullptr);

		EXPECT_EQ(1, opExpr->operation.size());

		string* lhsVal = boost::get<string>(&opExpr->lhs);
		EXPECT_EQ(get<1>(expectedValues), *lhsVal);

		// Check decl value
		EXPECT_EQ(get<2>(expectedValues), opExpr->operation[0].op);

		string* rhsVal = boost::get<string>(&opExpr->operation[0].rhs);
		EXPECT_EQ(get<3>(expectedValues), *rhsVal);

		index += 1;
	}
}

TEST(ASTTest, FunctionReturn) {
	const auto testProgram =
		"int main() {"
		"  return 1;"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF != nullptr);

	EXPECT_EQ(0, exprF->declarations.size());
	EXPECT_EQ(1, exprF->expressions.size());

	return_expr* exprR = boost::get<return_expr>(&exprF->expressions[0]);
	EXPECT_TRUE(exprR != nullptr);
}

/*
TEST(ASTTest, FunctionReturnComplex) {
	const auto testProgram =
		"int main() {"
		"  return a + b + c + 0 + 1 + d;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, MultipleFunction) {
	const auto testProgram =
		"int foo() {}"
		"int main() {}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, MultipleComplexFunction) {
	const auto testProgram =
		"int bar() {"
		"  int a = 0;"
		"  int b = 2;"
		"  return a + b + 0;"
		"}"
		"int foo() {"
		"  return a + 0;"
		"}"
		"int main() {"
		"  return 0 + 1;"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionArgs) {
	const auto testProgram =
		"int main(int a, int b) {"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionCall) {
	const auto testProgram =
		"int foo() {}"
		"int main() {"
		"  foo();"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionCallArgs) {
	const auto testProgram =
		"int foo(int a) {}"
		"int main() {"
		"  foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionCallArgsComplex) {
	const auto testProgram =
		"int bar(int a, int b) {}"
		"int foo(int a) {"
		"  return bar(a, 5);"
		"}"
		"int main() {"
		"  return foo(45);"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionIfStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}

TEST(ASTTest, FunctionIfElseStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"  } else {"
		"  }"
		"}";

	EXPECT_TRUE(parse(testProgram));
}
*/

