

/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/
/*
	Grammar
		Calculate
			print
			quite
			statement()

		Statement
			declaration()
			squareroot()
			power() //not yet working
			expression()

		Declaration   //variable declaration
			name
			value 
				expression()
		Expression
			primary
			+ term
			- term
		Term
			primary
			* primary
			/ primary
			% primary
		Primary
			( expression )
			neg number
			number
			declaration value  //if a variable

*/


#include "../../std_lib_facilities.h"

//------------------------------------------------------------------------------

class Token {
public:
	char kind;
	double value;
	string name;
	Token() :kind{ 0 } {}
	Token(char ch) :kind{ ch }, value{ 0 } {}
	Token(char ch, double val) :kind{ ch }, value{ val } {}
	Token(char ch, string n) :kind{ ch }, name{ n } {}
};

class Token_stream {
	bool full;
	Token buffer;
public:
	Token_stream() :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer = t; full = true; }

	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char squareroot = 's';
const char power = 'p';  //for use with power function
bool is_pwr = false;

Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	cin >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':    
	case ';':
	case '=':
	case ',':  //for use by power 
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();
	    double val;
	    cin >> val;
	    return Token(number, val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;  //bug was here += missing +
			cin.unget();
			if (s == "let") return Token(let);
			if (s == "sqrt") return Token(squareroot);
			if (s == "pow") return Token(power);
			if (s == "quit") return Token(quit);  //replaced name with quit
			return Token(name, s);  //if it gets here it is not an expression
			                        //and it is not a number
			                        // it is not let nor quit.  it is a variable name 
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c)
{
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
	Variable(string n, double v) :name(n), value(v) { }
};

vector<Variable> names;

double get_value(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(string s, double d)
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

Token_stream ts;
double expression(Token_stream& );

double primary(Token_stream& ts)
{
	Token t = ts.get(); //get a token
	switch (t.kind) {
	case '(':  //  then ... evaluate expression inside...
	{	
	    double d = expression(ts);  //evaluate the expression which will call primary again
	    t = ts.get();  //if you get here, expression was evaluated. get another token which should be
	               // a ')'
	    if (is_pwr) {
		if (t.kind != ',') error(" ',' expected.");
		ts.unget(t);   //put back the ','
	    }   
	    else {
		if (t.kind != ')') error("'(' expected");
	    } 
	return d;   //need to return the value of the expression inside (...)
	}
	
	case ',':
	{
		double d = expression(ts);
		t = ts.get();
		if (t.kind != ')') error("'(' expected to close power");
		return d;
	}

	case '-':
		return -primary(ts);  //if '-' return minus primary
	case number:
		return t.value;  //if a number return the number
	case name:
		return get_value(t.name); //if it is a variable in an expression get_value of variable
	/*
	case ')':
		if (is_pwr = true) {
			return 
		}
		else {
			error("')' not expected");
		}  
		*/
	default:
		error("primary expected");
	}
}

double term(Token_stream& ts)
{
	double left = primary(ts);  //evaluate * or /
	while (true) {
		Token t = ts.get();  //if you get here primary was defined and value stored in left
		switch (t.kind) {
		case '*':   //given math convention * and / evaluated first
			left *= primary(ts);
			break;
		case '/':
		{	double d = primary(ts);
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
		case '%':
		{
			double d = primary(ts);
			if (d == 0) error("mod divide by zero");
			return fmod(left, d);
			//book has a ts.get here but that is handled by parent?  I think 9/21/19.
		}
		default:
			ts.unget(t);  //if not * or / then put the token back
			return left;  
		}
	}
}

double expression(Token_stream& ts)
{
	double left = term(ts);  //send to term to ensure * or / or expression in ( ) evaluated first

	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term(ts);
			break;
		case '-':
			left -= term(ts);
			break;
		default:
			ts.unget(t);  //if not + or - put token back
			return left;
		}
	}
}

double declaration(Token_stream& ts)
{
	Token t = ts.get();
	if (t.kind != 'a') error("name expected in declaration");  // 'a' is the identifier for a variable name
	string name = t.name;                          //variable name
	if (is_declared(name)) error(name, " declared twice");  //if this variable is already declared
	Token t2 = ts.get();  //get another token.  should be an "=" sign if user typed correctly.
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression(ts);  // if it gets here it is a correctly formatted expression that 
	                          // needs to be returned
	names.push_back(Variable(name, d));  //  add this variable to the variable table vector
	return d;
}

double pwr_expression(Token_stream& ts)
{
	double t1 = expression(ts); //first term in power function
	int t2 = narrow_cast <int> (expression(ts));  //second term in power function
	return pow(t1, t2);

}

double statement(Token_stream& ts)
{
	Token t = ts.get();  //this will establish t.kind value
	is_pwr = false;
	switch (t.kind) {
	  case let:
		return declaration(ts);
	  case squareroot:   //could not call the variable sqrt
	  {
		double d = expression(ts);
		if (d <= 0) error("expression result less than zero!");
		return sqrt(d);  //no unget
	  }
	  case power:
	  {
		  //pow(expression(),term() (but could be expression)
		  is_pwr = true;
		  double d = pwr_expression(ts);  //should return value of expression up to ','
		  return d;
		  
	  }
	  default:
	  {
		ts.unget(t);  //put the token back it might be needed for expression
		return expression(ts);  //if not let put it back and call expression
	  }
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();  //get a token 
		while (t.kind == print) t = ts.get(); //print is ;. if ; get another
		if (t.kind == quit) return; //if quit, retun back to main
		ts.unget(t);  //not quit or print so put it back
		cout << result << statement(ts) << endl;   //determine what statement we have
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

try {
	calculate();  //call the calculate function with loop
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}

//------------------------------------------------------------------------------
