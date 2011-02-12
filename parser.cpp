#include "parser.h"
#pragma warning(disable:4018)
#pragma warning(disable:4996)

const int exprArrSize = 500;
int exprArrLength;
char exprArr[exprArrSize][exprArrSize];
map <string, short int> operationPriority;

void Expr::prt(int row, int col, string& data)
{
	for (int i = 0; i < data.length(); ++i)
		exprArr[row * row_step_size][col * col_step_size + i] = data[i];
	int a = col * col_step_size + data.length();
	exprArrLength = exprArrLength < a ? a : exprArrLength;
}
void DrawCor(int col, int row_begin, int row_end)
{
 	for (int i = row_begin * row_step_size + 1; i < row_end * row_step_size; ++i)
		exprArr[i][col * col_step_size] = symb_ver;
	exprArr[row_end * row_step_size][col * col_step_size] = symb_cor;
	for (int i = col * col_step_size + 1; i < (col+1) * col_step_size; ++i)
		exprArr[row_end * row_step_size][i] = symb_hor; 
}

int BinaryOp::Print(int row, int col)
{
	prt(row, col, name);
	DrawCor(col, row, row + 1);
	int r = left->Print(row + 1, col + 1);
	DrawCor(col, row + 1, r + 1);
	return right->Print(r + 1, col + 1);
}

int UnaryOp::Print(int row, int col)
{
	prt(row, col, name);
	DrawCor(col, row, row + 1);
	return exp->Print(row + 1, col + 1);
}


int ConstantInt::Print(int row, int col)
{
	char c[10];
	sprintf(c,"%d",value);
	string s = c;
	prt(row,col, s);
	return row;
}

int ConstantDouble::Print(int row, int col)
{
	char c[10];
	sprintf(c,"%f",value);
	string s = c;
	prt(row, col, s);
	return row;
}

int ConstantString::Print(int row, int col)
{
	prt(row, col, value);
	return row;
}

int Variable::Print(int row, int col)
{
	prt(row, col, name);
	return row;
}

int FunctionCall::Print(int row, int col)
{
	int o_r = row + 1;
	string s = "()";
	prt(row ,col, s);
	int r = Value->Print(row + 1, col + 1);
	DrawCor(col, row, row + 1);
	for (list<Expr*>::iterator it = args.begin(); it != args.end(); ++it)
	{
		DrawCor(col, o_r, r + 1);
		o_r = r;
		r = (*it)->Print(r + 1, col + 1);
	}
	return r;
}

Expr* Parser::ParseFactor()
{
	Expr* res;
	if (t.GetType() == end_of_file)
		return NULL;
	if (t == "not" || t == "-" || t == "+")
	{	
		string s = t.GetString();
		scan.Next();
		Expr* exp = ParseFactor();
		if (!exp)
			throw Error("Lexem expected, EOF found");
		res = new UnaryOp(s, exp);
	}
	else if (t == "(")
	{
		scan.Next();
		if (scan.GetToken() == ")")
			throw Error("Empty bracket sequence");
		res = ParseSimpleExpression(4);
		if (scan.GetToken() != ")")
			throw Error("Unclosed bracket"); 
		scan.Next();
		res = ExprCallAccess(res);
	}
	else if (t.GetType() == identifier)
	{
		res = new Variable(t.GetString());
		scan.Next();
		if (t == "[" || t == "(" || (t == "." && !isRecord))
			res = ExprCallAccess(res);
	}
	else 
	{
		switch(t.GetType())
		{
		case constant_int:
			res = new ConstantInt(t.GetInt());
			break;
		case constant_double:
			res = new ConstantDouble(t.GetFloat());
			break;
		case constant_str:
			res = new ConstantString(t.GetString());
			break;
		default:
			throw Error("Unexpected lexem found");
		}
		scan.Next();
	}
	return res;
}


Expr* Parser::ParseNext(){
	scan.Next();
	Expr* exp = ParseSimpleExpression(4);
	if (!exp)
		throw Error("Lexem expected, EOF found");
	return exp;
}

Expr* Parser::ParseSimpleExpression(int operationLvl)
{	
	if (operationLvl == 1)
		 return ParseFactor(); 
	Expr* exp1 = ParseSimpleExpression(operationLvl - 1);
	Expr* exp2;
	while (t.GetType() != end_of_file && operationPriority[t.GetString()] == operationLvl)
	{
		string str = t.GetString();
		scan.Next();
		if(t.GetType() == end_of_file)
			throw Error("Lexem expected, EOF found");
		exp2 = ParseSimpleExpression(operationLvl - 1);
		exp1 = new BinaryOp(str, exp1, exp2);	
	}
	return exp1;
}

Expr* Parser::ParseFunctionCall(Expr* res)
{
	Expr* exp = ParseNext();
	list<Expr*> arg;
	while (t != ")")
	{
		if (t == ","){
			arg.push_back(exp);
			exp = ParseNext();
		}
		else if (t != ")")
			throw Error("Unexpected lexem found");
	}
	arg.push_back(exp);
	res = new FunctionCall(res, arg);
	scan.Next();
	return res;
}

Expr* Parser::ParseRecordAccess(Expr* res)
{
	Expr* exp = ParseNext();
	if (!exp->IsVariable() && !exp->IsFunction())
		throw Error("Invalid record access");
	res = new RecordAccess(".", res, exp);
	while (t ==  ".")
	{
		exp = ParseNext();
		if (!exp->IsVariable() && !exp->IsFunction())
			throw Error("Invalid record access");
		res = new RecordAccess(".", res, exp);
	}
	return res;
}

Expr* Parser::ParseArrayAccess(Expr* res)
{	
	Expr* exp = ParseNext();
	res = new ArrayAccess("[]", res, exp);
	while (t != "]")
	{
		if (t == ",")
		{
			exp = ParseNext();
			res = new BinaryOp("[]", res, exp);
		}
	}
	scan.Next();
	return res;
}

Expr* Parser::ExprCallAccess(Expr* exp)
{
	Expr* res = exp;
	inAccess = true;
	while (t == "(" || t == "[" || t == "."){
		if (t == "(")
			res = ParseFunctionCall(res);
		else if (t == "[")
			res = ParseArrayAccess(res);
		else if (t == ".")
		{
			isRecord = true;
			res = ParseRecordAccess(res);
		}
	}
	inAccess = false;
	isRecord = false;
	return res;

}

void Parser::PrintTree(Expr* exp)
{
	for (int i = 0; i < exprArrSize; ++i)
		for (int j = 0; j < exprArrSize; ++j)
			exprArr[i][j] = '\0';
	exprArrLength = 0;
	int n = exp->Print(0,0);
	
	for (int i = 0; i <= n*(col_step_size-1); ++i)
	{
		for (int j = 0; j < exprArrLength; ++j)
			cout << exprArr[i][j];
		cout << endl;
	}
}

Parser::Parser(Scanner& s):scan(s),t(s.GetToken())
{
	operationPriority["not"] = 1; 
	operationPriority["*"] = operationPriority["/"] = operationPriority["div"] = operationPriority["mod"] = 2;
	operationPriority["and"] = operationPriority["shl"] = operationPriority["shr"] = 2; 
	operationPriority["-"] = operationPriority["+"] = operationPriority["or"] = operationPriority["xor"] = 3;
	operationPriority["="] = operationPriority["<>"] = operationPriority["<"] = operationPriority["<="] = operationPriority[">"] = 4; 
	operationPriority[">="] = 4;
	inAccess = isRecord = false;
}