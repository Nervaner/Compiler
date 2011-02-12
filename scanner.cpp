#include "scanner.h"
#pragma warning(disable:4018)

char* arrKeyWords[KEY_WORDS_QUANTITY]={
	"array","begin", "break", "case","char", "const","continue","do","downto","else","end",
	"exit","false","far","file","for","function","if","integer",
	"nil","of", "procedure","program","real", "record", "repeat","text",
	"then","to","true","type","until","uses","var","while","with","read","write","readln","writeln"};
char* arrSeparators[SEPARATORS_QUANTITY]={"(",")",";",":",",",".","[","]",".."};
char* arrOperations[OPERATIONS_QUANTITY]={"+","-","*","=",":=","/","div","mod","==",">=","<=",">","<","<>"};
char* arrTypeNames[]={
	"none      ",
	"identifier",
	"key_word  ",
	"integer   ",
	"double    ",
	"string    ",
	"operation ",
	"separator ",
};

Scanner::Scanner(char* fName)
{
	for (int i = 0; i < KEY_WORDS_QUANTITY; ++i)
		vocab[arrKeyWords[i]] = key_word;
	
	for (int i = 0; i < SEPARATORS_QUANTITY; ++i)
		vocab[arrSeparators[i]] = separator;

	for (int i = 0; i < OPERATIONS_QUANTITY; ++i)
		vocab[arrOperations[i]] = operation;

	vocab["\t"] = space;
	vocab[" "] = space;

	vocab["//"] = comment;
	
	fileName = fName;
	line = 1;
	pos = 1;
	currentToken = new Token(); 
	inf.open(fileName);
}

ostream& operator << (ostream &outstr, const Token& t)
{
	outstr << t.line << " " << t.pos << "\t" << arrTypeNames[t.tType] << "\t";
	switch(t.tType)
	{
	case constant_double:
		outstr << t.GetFloat() << endl;
		break;
	case constant_int:
		outstr << t.GetInt() << endl;
		break;
	default:
		outstr << "\"" << t.value << "\"" << endl;
	}
	return outstr;		
}

bool operator == (Token& t, const char* c)
{
	return strcmp(t.value.c_str(),c) == 0;	
}

bool operator != (Token& t, const char* c)
{
	return strcmp(t.value.c_str(),c) != 0;
}

void Scanner::Err(string& s, char* c)
{
	currentToken->value = s;
	throw Error(currentToken, c);	
}

void Scanner::Get(char& c)
{
	inf.get(c);
	c = tolower(c);
	pos++;
}

void Scanner::Putback(char& c)
{
	if (!inf.eof())
		inf.putback(c);
	pos--;
}

long int Token::GetInt() const
{ 
	if (value[0] != '$')
		return atoi(value.c_str());
	long int ans = 0;
	for (int i = 1; i < value.length() ; i++)
	{
		if (isdigit(value[i]))
			ans = ans * 16 + value[i] - '0';
		else
			ans = ans * 16 + 10 + value[i]-'a';
	}
	return ans;
}

string Scanner::ReadDouble()
{
	char c;
	Get(c);
	bool sgn_f = false, e_f = c == 'e';
	string s;
	Get(c);
	while (!EndOfLex(c) || (e_f && (c == '+' || c == '-')))
	{
		if ( (!isdigit(c) && (c != 'e' || e_f) && ( (c != '+' && c != '-') || sgn_f)) || (sgn_f && !e_f) ) 
			Err(s,"Incorrect real literal");	
		if (c == 'e')
			e_f = true;
		if (c == '+' || c == '-')
			sgn_f = true;
		s += c;
		Get(c);
	}
	Putback(c);
	return s;
}

bool Scanner::EndOfLex(char c)
{
	string es = "";
	TokenType supType = vocab[es + c];
	return inf.eof() || supType == space || supType == operation || supType == separator || c == '\n';
}

void Scanner::ReadInt()
{
	char c;
	bool hex = false;
	string s;
	currentToken->tType = constant_int;
	Get(c);
	if(c == '$')
	{
		hex = true;
		s += c;
		Get(c);
	}
	while (!EndOfLex(c) || (c == '.'))
	{
		s += c;
		if (!(!hex && (isdigit(c) || c == '.' || c == 'e')) && !(hex && isxdigit(c)))	
			Err(s,"Incorrect literal in integer");
		if (!hex && (c == '.'|| c == 'e'))
		{
			Putback(c);
			int l = s.length();
			s += ReadDouble();
			if (s[l-1] == 'e' && l == s.length())
				Err(s,"Incorrect real literal");
			if (s[l-1] == '.' && l == s.length())
			{
				s = s.substr(0,l-1);
				break;
			}
			currentToken->tType = constant_double;
		}
		Get(c);
	}
	if (currentToken->tType == constant_int)
		if ((s[0] != '$' && s.length() > 9 || (s.length() == 9 && s >= "2147483648")) || (s[0] == '$' && s.length() > 8 && s >= "$7fffffff"))
			Err(s,"Integer constant is too long");
	currentToken->value = s;
	Putback(c);

}

void Scanner::ReadIdentifier()
{
	char c;
	string s;
	Get(c);
	do	
	{
		if (!isalpha(c)&& !isdigit(c) && c != '_' && vocab[s + c] == none)
			Err(s,"Incorrect literal in identifier");
		s += c;
		Get(c);
	} while (!EndOfLex(c));
	if (vocab[s] != none)
		currentToken->tType = vocab[s];
	else
		currentToken->tType = identifier;
	currentToken->value = s;
	Putback(c);
}

void Scanner::ReadStr()
{
	char c;
	bool in_str = false, in_esc = false, after_ap = false;
	string s, esc;
	while(true)
	{
		inf.get(c);
		pos++;
		//---------в строке-------------
		if (in_str)
		{
			if (c == '\'')
			{
				in_str = false;
				after_ap = true;
			}
			if (c == '\n')
				Err(s,"Unexpected end of string");
			if (in_str)
				s += c;
		}
		else
		//---------не в строке-------------
		{
			//в последовательности 
			if (in_esc)
			{
				if(c == '#' || c == '\'' || inf.eof())
				{
					in_esc = c == '#';
					in_str = c == '\'';
					s += atoi(esc.c_str());
					esc = "";
				}
				else
				{
					if (EndOfLex(c))
					{
						s += atoi(esc.c_str());
						break;
					}
					if (isdigit(c))
						esc += c;
					else
						Err(s,"Bad symbol in escape sequence");
				}
			}
			else
			//не в последовательности
			{
				if (EndOfLex(c))
					break;
				if (c == '\'')
				{
					in_str = true;
					if (after_ap)
						s += '\'';
				}
				after_ap = false;
				if (!in_str)
				{
					if(c == '#')
						in_esc = true;
					else
						Err(s,"Unexpected symbol in string");
				}
			}
		}	
	}
	Putback(c);
	currentToken->tType = constant_str;
	currentToken->value = s;
}
void Scanner::ReadSeporatorOperation()
{
	char c;
	string s = "";
	Get(c);
	s += c;
	if (vocab[s] == none)
		Err(s,"Unknown symbol");
	Get(c);
	if (!inf.eof() && vocab[s + c] != none)
		s += c;	
	else
		Putback(c);
	currentToken->value = s;
	currentToken->tType = vocab[s];
}


// функция выкидывающая пробелы и комменты 
void Scanner::SkipSpaceComment()
{
	char c;
	string s = "";
	bool in_com = false;
	Get(c);
	if (inf.eof())
		return;
	while (!inf.eof() && (c == '\n' || c == ' ' || c == '\t' || c == '{' || c == '/' || c == '('))
	{
		switch(c)
		{
		case '\n':
			line++;
			pos = 1;
			break;
		case '/':
			Get(c);
			if (c != '/')
			{
				Putback(c);
				c = '/';
				Putback(c);
				return;
			}
			while(!inf.eof() && c != '\n')
				Get(c);
			line++;
			break;
		case '{':
			in_com = true;
			break;
		case'(':
			Get(c);
			if (c != '*')
			{
				Putback(c);
				c = '(';
				Putback(c);
				return;
			}	
			in_com = true;
			break;
		}
		Get(c);
		while(in_com)
		{
			if (c == '}')
				in_com = false;
			if (c == '*')
			{
				Get(c);
				if (c == ')')
					in_com = false;
				else
					Putback(c);
			}
			if (c == '\n')
				line++;
			if (inf.eof())
				Err(s,"Unclosed comment");
			Get(c);
		}
	}
	Putback(c);
}

void Scanner::Next()
{
	char c;
	SkipSpaceComment();
	delete currentToken;
	currentToken = new Token(line, pos, end_of_file, "");
	
	if (inf.eof())
		return;
	Get(c);
	Putback(c);
	if (isalpha(c) || c == '_')
		ReadIdentifier();
	else if (isdigit(c) || c == '$')
		ReadInt();
	else if (c == '\'' || c == '#')
		ReadStr();
	else 
		ReadSeporatorOperation();
}