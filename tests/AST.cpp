#include "catch.hpp"

#include <parser.h>

#include <map>
#include <string>
#include <tuple>

#include <boost/variant/get.hpp>

using namespace marklar;
using namespace parser;
using namespace std;

TEST_CASE("ASTTest_BasicFunction") {
	const auto testProgram =
		"i32 main() {"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr != nullptr);
	
	// We expect to have our func_expr in this root node
	CHECK(1u == expr->children.size());
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF != nullptr);

	// Check the function name
	CHECK("main" == exprF->functionName);
}

TEST_CASE("ASTTest_FunctionSingleDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr != nullptr);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF != nullptr);

	// Check declarations
	REQUIRE(1u == exprF->expressions.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->expressions[0]);
	REQUIRE(decl != nullptr);

	CHECK("i" == decl->declName);

	binary_op* opVal = boost::get<binary_op>(&decl->val);
	REQUIRE(opVal != nullptr);

	string* opValStr = boost::get<string>(&opVal->lhs);
	REQUIRE(opValStr != nullptr);
	CHECK("0" == *opValStr);
}

TEST_CASE("ASTTest_FunctionMultiDecl") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 0;"
		"  i32 j = 1;"
		"  i32 k = 2;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr);

	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF);

	REQUIRE(3u == exprF->expressions.size());

	const map<string, string> expectedNameVals = {
		{"i", "0"},
		{"j", "1"},
		{"k", "2"}
	};

	for (auto& funcDecl : exprF->expressions) {
		// Check declarations
		decl_expr* decl = boost::get<decl_expr>(&funcDecl);
		REQUIRE(decl != nullptr);

		const auto itr = expectedNameVals.find(decl->declName);
		REQUIRE(itr != expectedNameVals.end());

		CHECK(itr->first == decl->declName);

		binary_op* valOp = boost::get<binary_op>(&decl->val);
		CHECK(valOp);

		string* lhsVal = boost::get<string>(&valOp->lhs);
		CHECK(lhsVal);
		CHECK(itr->second == *lhsVal);
	}
}

TEST_CASE("ASTTest_FunctionDeclAssign") {
	const auto testProgram =
		"i32 main() {"
		"  i32 r = 1 + 2;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);

	// Check declarations
	REQUIRE(1u == exprF->expressions.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->expressions[0]);
	REQUIRE(decl != nullptr);

	CHECK("r" == decl->declName);

	binary_op* opExpr = boost::get<binary_op>(&decl->val);
	REQUIRE(opExpr != nullptr);
	CHECK(1u == opExpr->operation.size());

	// Check decl value
	CHECK("+" == opExpr->operation[0].op);

	string* rhsVal = boost::get<string>(&opExpr->operation[0].rhs);
	CHECK("2" == *rhsVal);
}

TEST_CASE("ASTTest_FunctionMultiDeclAssign") {
	const auto testProgram =
		"i32 main() {"
		"  i32 i = 1 + 2;"
		"  i32 j = i + 2;"
		"  i32 k = i + j;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);

	// Check declarations
	CHECK(3u == exprF->expressions.size());
	decl_expr* decl = boost::get<decl_expr>(&exprF->expressions[0]);
	REQUIRE(decl != nullptr);

	CHECK("i" == decl->declName);

	const vector<tuple<string, string, string, string>> expectedNameVals = {
		make_tuple("i", "1", "+", "2"),
		make_tuple("j", "i", "+", "2"),
		make_tuple("k", "i", "+", "j"),
	};

	int index = 0;
	for (auto& funcDecl : exprF->expressions) {
		// Check declarations
		decl_expr* decl = boost::get<decl_expr>(&funcDecl);
		REQUIRE(decl != nullptr);

		const auto expectedValues = expectedNameVals[index];

		CHECK(get<0>(expectedValues) == decl->declName);

		binary_op* opExpr = boost::get<binary_op>(&decl->val);
		REQUIRE(opExpr != nullptr);

		CHECK(1u == opExpr->operation.size());

		string* lhsVal = boost::get<string>(&opExpr->lhs);
		CHECK(get<1>(expectedValues) == *lhsVal);

		// Check decl value
		CHECK(get<2>(expectedValues) == opExpr->operation[0].op);

		string* rhsVal = boost::get<string>(&opExpr->operation[0].rhs);
		CHECK(get<3>(expectedValues) == *rhsVal);

		index += 1;
	}
}

TEST_CASE("ASTTest_FunctionReturn") {
	const auto testProgram =
		"i32 main() {"
		"  return 1;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF != nullptr);

	REQUIRE(1u == exprF->expressions.size());

	return_expr* exprR = boost::get<return_expr>(&exprF->expressions[0]);
	REQUIRE(exprR != nullptr);

	binary_op* exprRval = boost::get<binary_op>(&exprR->ret);
	CHECK(exprRval);

	string* exprRvalStr = boost::get<string>(&exprRval->lhs);
	CHECK(exprRvalStr);
	CHECK("1" == *exprRvalStr);
}

TEST_CASE("ASTTest_FunctionReturnComplex") {
	const auto testProgram =
		"i32 main() {"
		"  return a + b + c + 0 + 1 + d;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF != nullptr);

	REQUIRE(1u == exprF->expressions.size());

	return_expr* exprR = boost::get<return_expr>(&exprF->expressions[0]);
	REQUIRE(exprR != nullptr);
}

TEST_CASE("ASTTest_MultipleFunction") {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	CHECK(2u == expr->children.size());

	func_expr* exprF_foo = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF_foo != nullptr);
	CHECK("foo" == exprF_foo->functionName);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	REQUIRE(exprF_main != nullptr);
	CHECK("main" == exprF_main->functionName);
}

TEST_CASE("ASTTest_MultipleComplexFunction") {
	const auto testProgram =
		"i32 bar() {"
		"  i32 a = 0;"
		"  i32 b = 2;"
		"  return a + b + 0;"
		"}"
		"i32 foo() {"
		"  i32 a = 0;"
		"  return a + 0;"
		"}"
		"i32 main() {"
		"  return 0 + 1;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	CHECK(3u == expr->children.size());
}

TEST_CASE("ASTTest_FunctionArgs") {
	const auto testProgram =
		"i32 main(i32 a, i32 b) {"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr != nullptr);

	func_expr* exprF = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF != nullptr);

	REQUIRE(2u == exprF->args.size());

	auto* argDef1 = boost::get<def_expr>(&exprF->args[0]);
	REQUIRE(argDef1 != nullptr);
	REQUIRE("a" == argDef1->defName);
	CHECK("i32" == argDef1->typeName);

	auto* argDef2 = boost::get<def_expr>(&exprF->args[1]);
	REQUIRE(argDef2 != nullptr);
	REQUIRE("b" == argDef2->defName);
	CHECK("i32" == argDef2->typeName);
}

TEST_CASE("ASTTest_FunctionCall") {
	const auto testProgram =
		"i32 foo() {}"
		"i32 main() {"
		"  foo();"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr != nullptr);

	func_expr* exprF_foo = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF_foo != nullptr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	REQUIRE(exprF_main != nullptr);
	CHECK(1u == exprF_main->expressions.size());

	call_expr* callExpr = boost::get<call_expr>(&exprF_main->expressions[0]);
	REQUIRE(callExpr != nullptr);

	CHECK("foo" == callExpr->funcName);
	CHECK(0u == callExpr->values.size());
}

TEST_CASE("ASTTest_FunctionCallArgs") {
	const auto testProgram =
		"i32 foo(i32 a) {"
		"  return a;"
		"}"
		"i32 main() {"
		"  return foo(45);"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr != nullptr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	REQUIRE(exprF_main != nullptr);

	return_expr* retExpr = boost::get<return_expr>(&exprF_main->expressions[0]);
	REQUIRE(retExpr != nullptr);

	call_expr* callExpr = boost::get<call_expr>(&retExpr->ret);
	REQUIRE(callExpr != nullptr);

	CHECK("foo" == callExpr->funcName);
	CHECK(1u == callExpr->values.size());

	binary_op* val = boost::get<binary_op>(&callExpr->values[0]);
	REQUIRE(val != nullptr);

	string* strVal = boost::get<string>(&val->lhs);
	REQUIRE(strVal != nullptr);
	CHECK("45" == *strVal);
}

TEST_CASE("ASTTest_FunctionCallArgsComplex") {
	const auto testProgram =
		"i32 bar(i32 a, i32 b) {}"
		"i32 foo(i32 a) {"
		"  return bar(a, 5);"
		"}"
		"i32 main() {"
		"  return foo(45);"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	CHECK(expr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	CHECK(exprF_main);
	
	return_expr* exprR = boost::get<return_expr>(&exprF_main->expressions[0]);
	REQUIRE(exprR != nullptr);

	call_expr* callExpr = boost::get<call_expr>(&exprR->ret);
	REQUIRE(callExpr != nullptr);

	CHECK("bar" == callExpr->funcName);
	CHECK(2u == callExpr->values.size());

	binary_op* val1 = boost::get<binary_op>(&callExpr->values[0]);
	CHECK(val1);

	string* val1Str = boost::get<string>(&val1->lhs);
	CHECK(val1Str);
	CHECK("a" == *val1Str);

	binary_op* val2 = boost::get<binary_op>(&callExpr->values[1]);
	CHECK(val2);

	string* val2Str = boost::get<string>(&val2->lhs);
	CHECK(val2Str);
	CHECK("5" == *val2Str);
}

TEST_CASE("ASTTest_FunctionIfStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"  }"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	if_expr* exprIf = boost::get<if_expr>(&exprF_main->expressions[0]);
	REQUIRE(exprIf != nullptr);

	// Check the condition
	string* exprIfOpLhs = boost::get<string>(&exprIf->condition.lhs);
	REQUIRE(exprIfOpLhs != nullptr);
	CHECK("i" == *exprIfOpLhs);

	CHECK(1u == exprIf->condition.operation.size());
	CHECK("<" == exprIf->condition.operation[0].op);

	string* exprIfOpRhs = boost::get<string>(&exprIf->condition.operation[0].rhs);
	REQUIRE(exprIfOpRhs != nullptr);
	CHECK("4" == *exprIfOpRhs);
}

TEST_CASE("ASTTest_FunctionIfElseStmt") {
	const auto testProgram =
		"i32 main() {"
		"  if (i < 4) {"
		"    i32 j = 0;"
		"  } else {"
		"    i32 k = 0;"
		"  }"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	if_expr* exprIf = boost::get<if_expr>(&exprF_main->expressions[0]);
	REQUIRE(exprIf != nullptr);

	CHECK(1u == exprIf->thenBranch.size());
	CHECK(1u == exprIf->elseBranch.size());
}

TEST_CASE("ASTTest_Assignment") {
	const auto testProgram =
		"i32 main() {"
		"  i32 a = 3;"
		"  a = a + 1;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	// Check the assignment 
	REQUIRE(2u == exprF_main->expressions.size());

	var_assign* varAssign = boost::get<var_assign>(&exprF_main->expressions[1]);
	REQUIRE(varAssign != nullptr);

	CHECK("a" == varAssign->varName);
	
	binary_op* exprRhs = boost::get<binary_op>(&varAssign->varRhs);
	REQUIRE(exprRhs != nullptr);

	string* exprRhs_Lhs = boost::get<string>(&exprRhs->lhs);
	REQUIRE(exprRhs_Lhs != nullptr);
	CHECK("a" == *exprRhs_Lhs);

	CHECK(1u == exprRhs->operation.size());
	CHECK("+" == exprRhs->operation[0].op);

	string* exprRhs_Rhs = boost::get<string>(&exprRhs->operation[0].rhs);
	REQUIRE(exprRhs_Rhs != nullptr);
	CHECK("1" == *exprRhs_Rhs);
}

TEST_CASE("ASTTest_WhileStmt") {
	const auto testProgram =
		"i32 main() {"
		"  while (i < 4) {"
		"  }"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);

	while_loop* exprLoop = boost::get<while_loop>(&exprF_main->expressions[0]);
	REQUIRE(exprLoop != nullptr);

	// Check the condition
	string* exprLoopOpLhs = boost::get<string>(&exprLoop->condition.lhs);
	CHECK(exprLoopOpLhs);
	CHECK("i" == *exprLoopOpLhs);

	CHECK(1u == exprLoop->condition.operation.size());
	CHECK("<" == exprLoop->condition.operation[0].op);

	string* exprLoopOpRhs = boost::get<string>(&exprLoop->condition.operation[0].rhs);
	REQUIRE(exprLoopOpRhs != nullptr);
	CHECK("4" == *exprLoopOpRhs);
}

TEST_CASE("ASTTest_FuncCallInIfStmt") {
	const auto testProgram =
		"i32 func1(i32 a) {"
		"  return 0;"
		"}"
		"i32 main() {"
		"  if (func1(45) > 0) {"
		"    return 1;"
		"  }"
		"  return 0;"
		"}";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	CHECK(expr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[1]);
	CHECK(exprF_main);
	
	if_expr* exprIf = boost::get<if_expr>(&exprF_main->expressions[0]);
	REQUIRE(exprIf != nullptr);

	call_expr* callExpr = boost::get<call_expr>(&exprIf->condition.lhs);
	CHECK(callExpr);
}

TEST_CASE("ASTTest_DuplicateDefinition") {
	const auto testProgram = R"mrk(
		i32 main() {
		  i32 a;
		  i32 a;
		  return 0;
		}
		)mrk";

	base_expr_node root;
	REQUIRE(parse(testProgram, root));

	base_expr* expr = boost::get<base_expr>(&root);
	REQUIRE(expr);

	func_expr* exprF_main = boost::get<func_expr>(&expr->children[0]);
	REQUIRE(exprF_main);

	REQUIRE(3u == exprF_main->expressions.size());

	// First expression should be available, the second should not be	
	auto* exprDef = boost::get<def_expr>(&exprF_main->expressions[0]);
	REQUIRE(exprDef != nullptr);

	auto* exprDef2 = boost::get<def_expr>(&exprF_main->expressions[1]);
	REQUIRE(exprDef2 != nullptr);
}

