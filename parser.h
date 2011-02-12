#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <list>
#include "scanner.h" 

	const int row_step_size = 2;
	const int col_step_size = 3;
	const char symb_fork = 25;//'+';
	const char symb_ver = '|';
	const char symb_hor = '_';
	const char symb_cor = 3;

class Expr
{	
public:
	void prt(int row, int col, string& data);
	virtual int Print(int row, int col) = 0;
	virtual bool IsFunction() { return false; }
	virtual bool IsVariable() { return false; }
};

class BinaryOp: public Expr
{
	string name;
	Expr* left;
	Expr* right;
public:
	BinaryOp(string s, Expr* exp1,Expr* exp2): name(s), left(exp1),right(exp2){}
	int Print(int row, int col);
};



class UnaryOp: public Expr
{
	string name;
	Expr* exp;
public:
	UnaryOp(string s, Expr* exp1): name(s), exp(exp1){}
	int Print(int row, int col);
};

class ConstantInt: public Expr
{
	long int value;
public:
	ConstantInt(long int v):value(v){}
	int Print(int row, int col);
};

class ConstantDouble: public Expr
{
	double value;
public:
	ConstantDouble(double v):value(v){}
	int Print(int row, int col);
};

class ConstantString: public Expr
{
	string value;
public:
	ConstantString(string v):value(v){}
	int Print(int row, int col);
};

class Variable: public Expr
{
	string name;
public:
	Variable(string s): name(s){}
	int Print(int row, int col);
	bool IsVariable() { return true; }
};

class ArrayAccess: public BinaryOp
{
public:
	ArrayAccess(string val, Expr* l, Expr* r): BinaryOp(val, l, r){}
	int Print(int row, int col){return BinaryOp::Print(row,col);}
	bool IsVariable() { return true; }
};

class RecordAccess: public BinaryOp
{
public:
	RecordAccess(string val, Expr* l, Expr* r): BinaryOp(val, l, r){}
	int Print(int row, int col){ return BinaryOp::Print(row,col); }
	bool IsVariable() { return true; }
};

class FunctionCall: public Expr{
	list<Expr*> args;
	Expr* Value;
public: 
	FunctionCall(Expr* val, list<Expr*> ar): Value(val), args(ar){}
	int Print(int row, int col);
	bool IsFunction() {return true;}	
};

class Parser
{
	Scanner& scan;
public:
	Parser(Scanner& s);
	Expr* ParseNext();
	Expr* ParseFunctionCall(Expr* res);
	Expr* ParseRecordAccess(Expr* res);
	Expr* ParseArrayAccess(Expr* res);
	Expr* ExprCallAccess(Expr* exp);
	Expr* ParseSimpleExpression(int operationLvl);
	Expr* ParseAddSub();
	Expr* ParseMulDiv();
	Expr* ParseFactor();
	void PrintTree(Expr* exp);
	class Error
	{
	public:
		char* message;
		Error(char* s): message(s){}
	};
	Token& t;
	bool inAccess;
	bool isRecord;

};



#endif