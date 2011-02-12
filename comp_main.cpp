#include <iostream>
#include "scanner.h"
#include "parser.h"
using namespace std;

enum CompilationParameter
{
	cp_scan,
	cp_parse,
};

int main(int argc, char* argv[])
{
	CompilationParameter param;
	if (argc == 1) 
	{
		cout << "Simple compiler PASCAL." << endl;
		cout << "		Kanckuk A. 238gr" << endl;
		cout << "How to run:" << endl;
		cout << "pascal_compiler.exe some_file.pas -s         :to get token stream" << endl;
		cout << "pascal_compiler.exe some_file.pas -p         :to get parsing tree" << endl;
		return 0;
	}
	string p = argv[2];
	if (p == "-s")
		param = cp_scan;
	else if (p == "-p")
		param = cp_parse;
	else 
	{
		cout << "Compilatoin parameter not found or wrong." << endl;
		return 0;
	}
	Scanner scan = argv[1];
	Parser pars = Parser(scan);
	try
	{	
		scan.Next();
		Token& t = scan.GetToken();
		while (t.GetType() != end_of_file)
		{
			if (param == cp_scan)
			{
				cout << t;
			}
			if (param == cp_parse)
			{
				Expr* exp = pars.ParseSimpleExpression(4);
				// не забыть переделать пролистывание лексем
				//Expr* exp = pars.ParseArrayAccess();
				//Expr* exp = pars.ParseSimpleExpression();
				pars.PrintTree(exp);
			}
			scan.Next();
		}
	} catch (Scanner::Error e)
	{
		cout << "(" << e.token->GetLine() << "," << e.token->GetPos() << ") \"" << e.token->GetString() << "\" " << e.message << endl;
	}
	catch (Parser::Error e)
	{
		cout << e.message << endl;
	}

	return 0;
}
