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

	string* opVal = boost::get<string>(&decl->val);
	EXPECT_TRUE(opVal != nullptr);
	EXPECT_EQ("0", *opVal);
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

		string* lhsVal = boost::get<string>(&decl->val);
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

	string* exprRval = boost::get<string>(&exprR->ret);
	EXPECT_TRUE(exprRval != nullptr);
	EXPECT_EQ("1", *exprRval);
}

TEST(ASTTest, FunctionReturnComplex) {
	const auto testProgram =
		"int main() {"
		"  return a + b + c + 0 + 1 + d;"
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

TEST(ASTTest, MultipleFunction) {
	const auto testProgram =
		"int foo() {}"
		"int main() {}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_EQ(2, expr->children.size());

	func_expr* exprF_foo = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF_foo != nullptr);
	EXPECT_EQ("foo", exprF_foo->functionName);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	EXPECT_TRUE(exprF_main != nullptr);
	EXPECT_EQ("main", exprF_main->functionName);
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

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_EQ(3, expr->children.size());
}

TEST(ASTTest, FunctionArgs) {
	const auto testProgram =
		"int main(int a, int b) {"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_TRUE(expr != nullptr);

	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF != nullptr);

	EXPECT_EQ(2, exprF->args.size());
	EXPECT_EQ("a", exprF->args[0]);
	EXPECT_EQ("b", exprF->args[1]);
}

TEST(ASTTest, FunctionCall) {
	const auto testProgram =
		"int foo() {}"
		"int main() {"
		"  foo();"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	EXPECT_TRUE(expr != nullptr);

	func_expr* exprF_foo = boost::get<func_expr>(&expr->children[0]);
	EXPECT_TRUE(exprF_foo != nullptr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	EXPECT_TRUE(exprF_main != nullptr);
	EXPECT_EQ(1, exprF_main->expressions.size());

	call_expr* callExpr = boost::get<call_expr>(&exprF_main->expressions[0]);
	EXPECT_TRUE(callExpr != nullptr);

	EXPECT_EQ("foo", callExpr->funcName);
	EXPECT_EQ(0, callExpr->values.size());
}

TEST(ASTTest, FunctionCallArgs) {
	const auto testProgram =
		"int foo(int a) {}"
		"int main() {"
		"  foo(45);"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	call_expr* callExpr = boost::get<call_expr>(&exprF_main->expressions[0]);
	EXPECT_TRUE(callExpr != nullptr);

	EXPECT_EQ("foo", callExpr->funcName);
	EXPECT_EQ(1, callExpr->values.size());
	EXPECT_EQ("45", callExpr->values[0]);
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

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	
	return_expr* exprR = boost::get<return_expr>(&exprF_main->expressions[0]);
	EXPECT_TRUE(exprR != nullptr);

	call_expr* callExpr = boost::get<call_expr>(&exprR->ret);
	EXPECT_TRUE(callExpr != nullptr);

	EXPECT_EQ("bar", callExpr->funcName);
	EXPECT_EQ(2, callExpr->values.size());
	EXPECT_EQ("a", callExpr->values[0]);
	EXPECT_EQ("5", callExpr->values[1]);
}

TEST(ASTTest, FunctionIfStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	if_expr* exprIf = boost::get<if_expr>(&exprF_main->expressions[0]);
	EXPECT_TRUE(exprIf != nullptr);

	// Check the condition
	string* exprIfOpLhs = boost::get<string>(&exprIf->condition.lhs);
	EXPECT_TRUE(exprIfOpLhs != nullptr);
	EXPECT_EQ("i", *exprIfOpLhs);

	EXPECT_EQ(1, exprIf->condition.operation.size());
	EXPECT_EQ("<", exprIf->condition.operation[0].op);

	string* exprIfOpRhs = boost::get<string>(&exprIf->condition.operation[0].rhs);
	EXPECT_TRUE(exprIfOpRhs != nullptr);
	EXPECT_EQ("4", *exprIfOpRhs);
}

TEST(ASTTest, FunctionIfElseStmt) {
	const auto testProgram =
		"int main() {"
		"  if (i < 4) {"
		"    int j = 0;"
		"  } else {"
		"    int k = 0;"
		"  }"
		"}";

	base_expr_node root;
	EXPECT_TRUE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	if_expr* exprIf = boost::get<if_expr>(&exprF_main->expressions[0]);
	EXPECT_TRUE(exprIf != nullptr);

	EXPECT_EQ(1, exprIf->thenBranch.size());
	EXPECT_EQ(1, exprIf->elseBranch.size());
}

