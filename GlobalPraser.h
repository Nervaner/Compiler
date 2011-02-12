#include "parser.h"

enum SymbolType
{
	sym_none,
	sym_const,
	sym_variable,
	sym_type,
	sym_proc,
	sym_func
};


class Symbol
{
	string name;
	SymbolType type;
public:
	Symbol() { name = ""; }
	Symbol(string s): name(s) {}
	string GetName() { return name; }
	virtual void Print(ostream& os, bool f){};
	SymbolType GetSymType() { return sym_type; } 
	virtual SymType* GetType() {return NULL; }
};

class SymVariable : public Symbol
{
public:
	SymVariable(string s): name(s){}; 
	

};

map <string, Symbol*> SymTable; 