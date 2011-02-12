#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <fstream>
#include <stdlib.h>
#include <map>

using namespace std;

const int TOKEN_TYPE_QUANTITY = 8;
const int KEY_WORDS_QUANTITY = 40;
const int SEPARATORS_QUANTITY = 9;
const int OPERATIONS_QUANTITY = 14;
 
enum TokenType 
{
	none,							//0
	identifier,						//1
	key_word,						//2
	constant_int,					//3
	constant_double,				//4
	constant_str,					//5
	operation,						//6
	separator,						//7
	space,
	comment,
	end_of_file,
};

class Token
{
	TokenType tType;
	int line, pos;
	string value;
	friend class Scanner;
public:
	Token():tType(end_of_file), line(0), pos(0), value(""){};
	Token(int l, int p, TokenType t, string s):line(l), pos(p), tType(t),value(s){}
	TokenType GetType() { return tType; }
	string GetString() const {return value; }
	double GetFloat() const { return atof(value.c_str()); }
	long int GetInt() const ;
	int GetLine() { return line; }
	int GetPos() { return pos; }
	friend ostream& operator << (ostream &outstr, const Token& t);
	friend bool operator == (Token& t, const char* c);
	friend bool operator != (Token& t, const char* c);
};

class Scanner
{
	Token* currentToken;
	int line, pos;
	char* fileName;
	ifstream inf;
	map<string, TokenType> vocab;
	bool isalpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	bool isdigit(char c) { return c >= '0' && c <= '9'; }
	bool EndOfLex(char c);
	void Get(char& c);
	void Putback(char& c);
	void SkipSpaceComment();
	void ReadInt();
	string ReadDouble();
	void ReadIdentifier();
	void ReadSeporatorOperation();
	void ReadStr();
public:
	
	Scanner(char* fName);
	Token& GetToken(){ return *currentToken; }
	void Next();
	class Error
	{ 
	public:		
		char* message;
		Token* token;
		Error(Token* t,char* c): token(t), message(c){}
	};
	void Err(string& s,char* c);
};

#endif 