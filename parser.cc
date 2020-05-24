#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include "lexer.h"

using namespace std;

LexicalAnalyzer lexer;
Token token;

typedef struct
{
	string var_id;
	TokenType type;
	int init_line;
	int var_used;
	int line_declared;

}Var_decl_type;

typedef struct
{
	string code;
	string var_id;

}var_decl_error_type;

typedef struct
{
	string code;
	int line_num;

}type_mismatch_error_type;

typedef struct
{
	string var_id;
	int use_line;
	int decl_line;

}good_out_type;

typedef struct
{
	string var_id;
	int use_line;

}uninit_type;

typedef vector <vector<Var_decl_type>> var_decl_vector_type;

var_decl_vector_type var_decl_vector;

vector<Var_decl_type> curr_scope;

vector <Var_decl_type> temp_id_list;

var_decl_error_type var_decl_error;

type_mismatch_error_type type_mismatch_error;

vector <good_out_type> output;

vector <uninit_type> not_init_output;

typedef vector <uninit_type> while_inits_type;

vector <while_inits_type> while_inits;

vector <uninit_type> curr_while_init;

int scope_level = 0;

int uninit_scope_level = 0;

bool Syntax_error_happend = false;

void parse_scope();
void parse_scope_list();
void parse_var_decl();
void parse_id_list();
void parse_type_name();
void parse_stmt_list();
void parse_stmt();
void parse_assign_stmt();
void parse_while_stmt();
TokenType parse_expr();
void parse_arith_op();
void parse_bool_op();
void parse_rel_op();
void parse_primary();
//void parse_bool_const();
void parse_cond();
void print_scope(vector<Var_decl_type>);
bool FoundInVector(string srchString, int index, vector<Var_decl_type> srchVector);
void undecl_var_error();
void var_not_used_error();
void built_in_type_check(string varName);
TokenType FindType(string srchString);
void set_mismatch_error(int line, string code);
int Find_decl_line(string srchString);
void set_init(string srchString, int line);
void query_init(string srchString, int line);
bool FoundInInitVector(string srchString, int index, vector<uninit_type> srchVector);


bool expect(TokenType expected, TokenType actual)
{
	return (expected == actual);
}

void syntax_error(string value)
{
	Syntax_error_happend = true;
	//cout << "Syntax_error: " << value;
	//cout << "  line NO: " << token.line_no << "\n";
}

void parse_prog()
{
	token = lexer.GetToken();
	if (token.token_type == LBRACE)
	{
		lexer.UngetToken(token);
		parse_scope();
	}
	else
	{
		syntax_error("parse_prog");
	}
}

void parse_scope()
{
	token = lexer.GetToken();
	if (expect(LBRACE, token.token_type))
	{
		if (scope_level > 0)
		{
			var_decl_vector.push_back(curr_scope);
			curr_scope.clear();
		}
		scope_level = scope_level + 1;

		parse_scope_list();

		token = lexer.GetToken();
		if (expect(RBRACE, token.token_type))
		{
			var_not_used_error();

			if (scope_level > 1)
			{

				//print_scope(curr_scope);
				curr_scope.clear();
				curr_scope = var_decl_vector[var_decl_vector.size() - 1];
				var_decl_vector.pop_back();
				scope_level = scope_level - 1;
			}
			return;
		}
		else
		{
			syntax_error("parse_scope 1");
		}
	}
	else
	{
		syntax_error("parse_scope 2");
	}
}

void parse_scope_list()
{
	Token look_ahead_token;

	token = lexer.GetToken();

	if (token.token_type == LBRACE)
	{
		lexer.UngetToken(token);
		parse_scope();

		//checking for scope_list again
		token = lexer.GetToken();
		if (token.token_type == LBRACE ||
			token.token_type == ID ||
			token.token_type == WHILE)
		{
			lexer.UngetToken(token);
			parse_scope_list();
		}
		else if (token.token_type == RBRACE)
		{
			lexer.UngetToken(token);
			return;
		}
		else
		{

			syntax_error("parse_scope_list .5");
		}

	}
	else if (token.token_type == ID)
	{
		//looking head for comma, EOF, Equal
		look_ahead_token = lexer.GetToken();
		if (look_ahead_token.token_type == COLON ||
			look_ahead_token.token_type == COMMA)
		{
			lexer.UngetToken(look_ahead_token);
			lexer.UngetToken(token);

			parse_var_decl();

			//checking for scope_list again
			token = lexer.GetToken();
			if (token.token_type == LBRACE ||
				token.token_type == ID ||
				token.token_type == WHILE)
			{
				lexer.UngetToken(token);
				parse_scope_list();
			}
			else if (token.token_type == RBRACE)
			{
				lexer.UngetToken(token);
				return;
			}
			else
			{
				syntax_error("parse_scope_list 1");
			}
		}
		else if (look_ahead_token.token_type == EQUAL)
		{
			lexer.UngetToken(look_ahead_token);
			lexer.UngetToken(token);

			parse_stmt();

			//checking for scope_list again
			token = lexer.GetToken();
			if (token.token_type == LBRACE ||
				token.token_type == ID ||
				token.token_type == WHILE)
			{
				lexer.UngetToken(token);
				parse_scope_list();
			}
			else if (token.token_type == RBRACE)
			{
				lexer.UngetToken(token);
				return;
			}
			else
			{
				syntax_error("parse_scope_list 2");
			}
		}
		else
		{
			syntax_error("parse_scope_list 3");
		}
	}
	else if (token.token_type == WHILE)
	{
		lexer.UngetToken(token);
		parse_stmt();

		//checking for scope_list again
		token = lexer.GetToken();
		if (token.token_type == LBRACE ||
			token.token_type == ID ||
			token.token_type == WHILE)
		{
			lexer.UngetToken(token);
			parse_scope_list();
		}
		else if (token.token_type == RBRACE)
		{
			lexer.UngetToken(token);
			return;
		}
		else
		{
			syntax_error("parse_scope_list 4");
		}
	}
	else
	{
		syntax_error("parse_scope_list 5");
	}
}

void parse_var_decl()
{
	Token look_ahead_token;

	token = lexer.GetToken();

	if (token.token_type == ID)
	{
		lexer.UngetToken(token);
		parse_id_list();

		token = lexer.GetToken();
		if (expect(COLON, token.token_type))
		{
			parse_type_name();

			token = lexer.GetToken();
			if (expect(SEMICOLON, token.token_type))
			{
				return;
			}
			else
			{
				syntax_error("parse_var_decl 1");
			}
		}
		else
		{
			syntax_error("parse_var_decl 2");
		}
	}
	else
	{
		syntax_error("parse_var_decl 3");
	}
}

void parse_id_list()
{
	Var_decl_type temp_id;

	token = lexer.GetToken();
	if (token.token_type == ID)
	{
		built_in_type_check(token.lexeme);

		if (FoundInVector(token.lexeme, 0, temp_id_list) ||
			FoundInVector(token.lexeme, 0, curr_scope))
		{
			var_decl_error.code = "1.1";
			var_decl_error.var_id = token.lexeme;
		}
		else
		{
			temp_id.var_id = token.lexeme;
			temp_id.init_line = 0;
			temp_id.line_declared = token.line_no;
			temp_id.var_used = false;

			temp_id_list.push_back(temp_id);
		}

		token = lexer.GetToken();
		if (token.token_type == COMMA)
		{
			parse_id_list();
		}
		else if (token.token_type == COLON)
		{
			lexer.UngetToken(token);
			return;
		}
		else
		{
			syntax_error("parse_id_list 2");
		}

	}
	else
	{
		syntax_error("parse_id_list 3");
	}
}

void parse_type_name()
{
	token = lexer.GetToken();
	if (token.token_type == REAL ||
		token.token_type == INT ||
		token.token_type == BOOLEAN ||
		token.token_type == STRING)
	{
		for (int i = 0; i < temp_id_list.size(); i++)
		{
			temp_id_list[i].type = token.token_type;
			curr_scope.push_back(temp_id_list[i]);
		}
		temp_id_list.clear();
		return;
	}
	else
	{
		syntax_error("parse_type_name");
	}
}

void parse_stmt_list()
{
	token = lexer.GetToken();

	if (token.token_type == ID ||
		token.token_type == WHILE)
	{
		lexer.UngetToken(token);
		parse_stmt();

		token = lexer.GetToken();
		if (token.token_type == ID ||
			token.token_type == WHILE)
		{
			lexer.UngetToken(token);
			parse_stmt_list();
		}
		else
		{
			lexer.UngetToken(token);
			return;
		}
	}
	else
	{
		syntax_error("parse_stmt_list ");
	}

}

void parse_stmt()
{
	token = lexer.GetToken();
	if (token.token_type == ID)
	{
		lexer.UngetToken(token);

		parse_assign_stmt();
	}
	else if (token.token_type == WHILE)
	{
		lexer.UngetToken(token);

		parse_while_stmt();
	}
	else
	{
		syntax_error("parse_stmt");
	}
}

void parse_assign_stmt()
{
	TokenType LHS_type, RHS_type;
	good_out_type temp_out;
	int line;

	token = lexer.GetToken();

	line = token.line_no;

	if (token.token_type == ID)
	{
		//saving out_put refrence
		temp_out.var_id = token.lexeme;
		temp_out.use_line = token.line_no;
		temp_out.decl_line = Find_decl_line(token.lexeme);
		output.push_back(temp_out);

		set_init(token.lexeme, token.line_no);

		LHS_type = FindType(token.lexeme);

		built_in_type_check(token.lexeme);

		undecl_var_error();

		token = lexer.GetToken();
		if (expect(EQUAL, token.token_type))
		{
			RHS_type = parse_expr();

			token = lexer.GetToken();
			if (expect(SEMICOLON, token.token_type))
			{
				if (LHS_type != ERROR && RHS_type != ERROR)
				{
					if (LHS_type == RHS_type ||
						(LHS_type == REAL && RHS_type == INT))
					{
						return;
					}
					else if (LHS_type == REAL)
					{
						set_mismatch_error(line, "C2");
					}
					else
					{
						set_mismatch_error(line, "C1");
					}
				}
				return;
			}
			else
			{
				syntax_error("parse_assign_stmt 1");
			}
		}
		else
		{
			syntax_error("parse_assign_stmt 2");
		}
	}
	else
	{
		syntax_error("parse_assign_stmt 3");
	}
}

void parse_while_stmt()
{
	token = lexer.GetToken();

	if (token.token_type == WHILE)
	{
		parse_cond();

		token = lexer.GetToken();

		if (token.token_type == LBRACE)
		{
			if (uninit_scope_level > 0)
			{
				while_inits.push_back(curr_while_init);
			}
			uninit_scope_level = uninit_scope_level + 1;
			curr_while_init.clear();

			parse_stmt_list();

			token = lexer.GetToken();
			if (expect(RBRACE, token.token_type))
			{
				//debug output

				/*cout << "while scope: "<<uninit_scope_level<<"\n";
				for (int i = 0; i < curr_while_init.size(); i++)
				{
					cout << "var_id: " << curr_while_init[i].var_id << " line_no: " << curr_while_init[i].use_line << "\n";
				}*/

				if (uninit_scope_level > 1)
				{
					curr_while_init.clear();
					curr_while_init = while_inits[while_inits.size() - 1];
					while_inits.pop_back();
					uninit_scope_level = uninit_scope_level - 1;
				}
				else if (uninit_scope_level == 1)
				{
					curr_while_init.clear();
					uninit_scope_level = 0;
				}
				return;
			}
			else
			{
				syntax_error("parse_while_stmt 1");
			}
		}
		else if (token.token_type == ID ||
			token.token_type == WHILE)
		{
			lexer.UngetToken(token);

			parse_stmt();

			return;
		}
		else
		{
			syntax_error("parse_while_stmt 2");
		}
	}
	else
	{
		syntax_error("parse_while_stmt 3");
	}
}

TokenType parse_expr()
{
	int line;
	TokenType result = ERROR;
	TokenType first_arg;
	TokenType second_arg;
	TokenType math_oper;
	TokenType rel_oper;
	TokenType bool_oper;
	good_out_type temp_out;

	token = lexer.GetToken();

	if (token.token_type == ID)
	{
		built_in_type_check(token.lexeme);
		query_init(token.lexeme, token.line_no);
	}

	undecl_var_error();

	if (token.token_type == ID ||
		token.token_type == NUM ||
		token.token_type == REALNUM ||
		token.token_type == STRING_CONSTANT ||
		token.token_type == TRUE ||
		token.token_type == FALSE)
	{
		if (token.token_type == ID)
		{
			//saving out_put refrence
			temp_out.var_id = token.lexeme;
			temp_out.use_line = token.line_no;
			temp_out.decl_line = Find_decl_line(token.lexeme);
			output.push_back(temp_out);

			first_arg = FindType(token.lexeme);
		}
		else if (token.token_type == NUM)
		{
			first_arg = INT;
		}
		else if (token.token_type == REALNUM)
		{
			first_arg = REAL;
		}
		else if (token.token_type == TRUE || token.token_type == FALSE)
		{
			first_arg = BOOLEAN;
		}
		else
		{
			first_arg = STRING;
		}

		lexer.UngetToken(token);
		parse_primary();
		return first_arg;
	}
	else if (token.token_type == PLUS ||
		token.token_type == MINUS ||
		token.token_type == MULT ||
		token.token_type == DIV)
	{
		// saving values for mismatch check
		math_oper = token.token_type;
		line = token.line_no;

		lexer.UngetToken(token);

		parse_arith_op();

		first_arg = parse_expr();

		second_arg = parse_expr();

		/***************************
		checking for C3
		***************************/
		if (first_arg == STRING || first_arg == BOOLEAN ||
			second_arg == STRING || second_arg == BOOLEAN)
		{
			set_mismatch_error(line, "C3");
			result == ERROR;
		}
		else if (first_arg == REAL || second_arg == REAL || math_oper == DIV)
		{
			result = REAL;
		}
		else
		{
			result = INT;
		}
	}
	else if (token.token_type == AND ||
		token.token_type == OR ||
		token.token_type == XOR)
	{
		line = token.line_no;

		lexer.UngetToken(token);

		parse_bool_op();

		first_arg = parse_expr();

		second_arg = parse_expr();

		/***************************
		checking for C4
		***************************/

		if (first_arg == BOOLEAN && second_arg == BOOLEAN)
		{
			result = BOOLEAN;
		}
		else
		{
			set_mismatch_error(line, "C4");
			result == ERROR;
		}

	}
	else if (token.token_type == GREATER ||
		token.token_type == GTEQ ||
		token.token_type == LESS ||
		token.token_type == NOTEQUAL ||
		token.token_type == LTEQ)
	{
		line = token.line_no;

		lexer.UngetToken(token);

		parse_rel_op();

		first_arg = parse_expr();

		second_arg = parse_expr();

		

		/***************************
		checking for C5 and C6
		***************************/

		if (first_arg == INT || first_arg == REAL ||
			second_arg == INT || second_arg == REAL)
		{
			if (first_arg == STRING || first_arg == BOOLEAN ||
				second_arg == STRING || second_arg == BOOLEAN)
			{
				set_mismatch_error(line, "C6");
				
				result = ERROR;
			}
			else
			{
				result = BOOLEAN;
			}
		}
		else if (first_arg == BOOLEAN || second_arg == BOOLEAN)
		{
			if (first_arg == BOOLEAN && second_arg == BOOLEAN)
			{
				result = BOOLEAN;
			}
			else
			{
				set_mismatch_error(line, "C5");
				
				result = ERROR;
			}
		}
		else if (first_arg == STRING || second_arg == STRING)
		{
			if (first_arg == STRING && second_arg == STRING)
			{
				result = BOOLEAN;
			}
			else
			{
				set_mismatch_error(line, "C5");
				
				result = ERROR;
			}
		}

		else
		{
			cout << "Really weird inparse expression for Rel_op\n";
		}

	}
	else if (token.token_type == NOT)
	{
		line = token.line_no;
		first_arg = parse_expr();

		/***************************
		checking for C8
		***************************/

		if (first_arg == BOOLEAN)
		{
			result = BOOLEAN;
		}
		else
		{
			set_mismatch_error(line, "C8");
			result == ERROR;
		}

	}
	else
	{
		syntax_error("parse_expr");
	}

	//cout << "result: " << result << "\n";

	return result;
}

void parse_primary()
{
	token = lexer.GetToken();

	if (token.token_type == ID)
	{
		built_in_type_check(token.lexeme);
	}

	if (token.token_type == ID ||
		token.token_type == NUM ||
		token.token_type == REALNUM ||
		token.token_type == STRING_CONSTANT ||
		token.token_type == TRUE ||
		token.token_type == FALSE)
	{
		return;
	}
	else
	{
		syntax_error("parse_primary");
	}
}

void parse_arith_op()
{
	token = lexer.GetToken();

	if (token.token_type == PLUS ||
		token.token_type == MINUS ||
		token.token_type == MULT ||
		token.token_type == DIV)
	{
		return;
	}
	else
	{
		syntax_error("parse_arith_op");
	}
}

void parse_bool_op()
{
	token = lexer.GetToken();

	if (token.token_type == AND ||
		token.token_type == OR ||
		token.token_type == XOR)
	{
		return;
	}
	else
	{
		syntax_error("parse_bool_op");
	}
}

void parse_rel_op()
{
	token = lexer.GetToken();

	if (token.token_type == GREATER ||
		token.token_type == GTEQ ||
		token.token_type == LESS ||
		token.token_type == NOTEQUAL ||
		token.token_type == LTEQ)
	{
		return;
	}
	else
	{
		syntax_error("parse_rel_op");
	}
}

void parse_cond()
{
	TokenType argument;
	int line;

	token = lexer.GetToken();

	line = token.line_no;

	if (token.token_type == LPAREN)
	{
		argument = parse_expr();

		if (argument != BOOLEAN && type_mismatch_error.code == "NONE")
		{
			set_mismatch_error(line, "C7");
		}

		token = lexer.GetToken();

		if (expect(RPAREN, token.token_type))
		{
			return;
		}
		else
		{
			syntax_error("parse_cond 1");
		}
	}
	else
	{
		syntax_error("parse_cond 2");
	}
}

void print_scope(vector<Var_decl_type> scope)
{
	cout << "Scope level: " << scope_level << "\n";
	for (int i = 0; i < scope.size(); i++)
	{
		cout << "var name: " << scope[i].var_id << ",   ";
		cout << "type: " << scope[i].type << ",   ";
		cout << "Initialized: " << scope[i].init_line << ",   ";
		cout << "declared: " << scope[i].line_declared << ",   ";
		cout << "used: " << scope[i].var_used << "   ";
		cout << "\n";
	}
	cout << "**********************************************\n";
}

bool FoundInVector(string srchString, int index, vector<Var_decl_type> srchVector)
{

	bool result = false;

	if (srchVector.empty())
	{
		result = false;
	}
	else if (srchVector[index].var_id == srchString)
	{
		result = true;
	}
	else if (index < srchVector.size() - 1)
	{
		result = FoundInVector(srchString, index + 1, srchVector);
	}

	return result;
}

bool FoundInInitVector(string srchString, int index, vector<uninit_type> srchVector)
{

	bool result = false;

	if (srchVector.empty())
	{
		result = false;
	}
	else if (srchVector[index].var_id == srchString)
	{
		result = true;
	}
	else if (index < srchVector.size() - 1)
	{
		result = FoundInInitVector(srchString, index + 1, srchVector);
	}

	return result;
}

TokenType FindType(string srchString)
{

	TokenType result = END_OF_FILE;

	bool found = FoundInVector(token.lexeme, 0, curr_scope);

	if (found)
	{
		int index = 0;
		while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
		{
			index = index + 1;
		}
		if (index < curr_scope.size())
		{
			result = curr_scope[index].type;
		}
		else
		{
			cout << "in FindType went past curr_scope index\n";
		}
	}

	else // not found
	{
		//serching var_decl_vector from the bottom up
		int i = var_decl_vector.size() - 1;

		while (!found && i >= 0)
		{
			if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
			{
				found = true;

				int index = 0;
				while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
				{
					index = index + 1;
				}
				if (index < var_decl_vector[i].size())
				{
					result = var_decl_vector[i][index].type;
				}
				else
				{
					cout << "in FindType went past var_decl_vector[" << i << "] index\n";
				}

			}
			i = i - 1;
		}
	}

	return result;
}

int Find_decl_line(string srchString)
{

	int result = 0;

	bool found = FoundInVector(token.lexeme, 0, curr_scope);

	if (found)
	{
		int index = 0;
		while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
		{
			index = index + 1;
		}
		if (index < curr_scope.size())
		{
			result = curr_scope[index].line_declared;
		}
		else
		{
			cout << "in Find_decl_line went past curr_scope index\n";
		}
	}

	else // not found
	{
		//serching var_decl_vector from the bottom up
		int i = var_decl_vector.size() - 1;

		while (!found && i >= 0)
		{
			if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
			{
				found = true;

				int index = 0;
				while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
				{
					index = index + 1;
				}
				if (index < var_decl_vector[i].size())
				{
					result = var_decl_vector[i][index].line_declared;
				}
				else
				{
					cout << "in Find_decl_line went past var_decl_vector[" << i << "] index\n";
				}

			}
			i = i - 1;
		}
	}

	return result;
}

void set_init(string srchString, int line)
{
	bool found = false;
	uninit_type temp_uninit;
	int index = 0;

	//if in while loop
	if (uninit_scope_level > 0)
	{
		found = FoundInInitVector(srchString, 0, curr_while_init);
		if (!found)
		{
			index = while_inits.size();
			while (!found && index > 0)
			{
				found = FoundInInitVector(srchString, 0, while_inits[index - 1]);
				index = index - 1;
			}

			// not found in while scope checking var scope
			if (!found)
			{
				found = FoundInVector(token.lexeme, 0, curr_scope);

				if (found)
				{
					int index = 0;
					while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
					{
						index = index + 1;
					}
					if (index < curr_scope.size() && curr_scope[index].init_line == 0)
					{
						temp_uninit.var_id = srchString;
						temp_uninit.use_line = line;

						curr_while_init.push_back(temp_uninit);
					}
				}

				else // not found
				{
					//serching var_decl_vector from the bottom up
					int i = var_decl_vector.size() - 1;

					while (!found && i >= 0)
					{
						if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
						{
							found = true;

							int index = 0;
							while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
							{
								index = index + 1;
							}
							if (index < var_decl_vector[i].size() && var_decl_vector[i][index].init_line == 0)
							{
								temp_uninit.var_id = srchString;
								temp_uninit.use_line = line;

								curr_while_init.push_back(temp_uninit);
							}
						}
						i = i - 1;
					}
				}
			}
		}
		if (!found)
		{
			temp_uninit.var_id = srchString;
			temp_uninit.use_line = line;

			curr_while_init.push_back(temp_uninit);
		}

	}

	// not in while loop
	else
	{
		found = FoundInVector(token.lexeme, 0, curr_scope);

		if (found)
		{
			int index = 0;
			while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
			{
				index = index + 1;
			}
			if (index < curr_scope.size() && curr_scope[index].init_line == 0)
			{
				curr_scope[index].init_line = line;
			}
		}

		else // not found
		{
			//serching var_decl_vector from the bottom up
			int i = var_decl_vector.size() - 1;

			while (!found && i >= 0)
			{
				if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
				{
					found = true;

					int index = 0;
					while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
					{
						index = index + 1;
					}
					if (index < var_decl_vector[i].size() && var_decl_vector[i][index].init_line == 0)
					{
						var_decl_vector[i][index].init_line = line;
					}
				}
				i = i - 1;
			}
		}
	}

}

void query_init(string srchString, int line)
{
	uninit_type temp_uninit;

	bool found = false;

	//if in while loop
	if (uninit_scope_level > 0)
	{
		found = FoundInInitVector(token.lexeme, 0, curr_while_init);

			if (found)
			{
				int index = 0;
				while (token.lexeme != curr_while_init[index].var_id && index < curr_while_init.size())
				{
					index = index + 1;
				}
				if (index < curr_while_init.size())
				{
					if (curr_while_init[index].use_line == line)
					{
						temp_uninit.use_line = line;
						temp_uninit.var_id = srchString;
						not_init_output.push_back(temp_uninit);
					}
				}

			}

			else // not found
			{
				//serching while_inits from the bottom up
				int i = while_inits.size() - 1;

				while (!found && i >= 0)
				{
					if (FoundInInitVector(token.lexeme, 0, while_inits[i]))
					{
						found = true;

						int index = 0;
						while (token.lexeme != while_inits[i][index].var_id && index < while_inits[i].size())
						{
							index = index + 1;
						}
						if (index < while_inits[i].size())
						{
							if (while_inits[i][index].use_line == line)
							{
								temp_uninit.use_line = line;
								temp_uninit.var_id = srchString;
								not_init_output.push_back(temp_uninit);
							}
						}
					}
					i = i - 1;
				}
			}
	
	}

	//checking var_decl
	if(!found)
	{
		found = FoundInVector(token.lexeme, 0, curr_scope);

		if (found)
		{
			int index = 0;
			while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
			{
				index = index + 1;
			}
			if (index < curr_scope.size())
			{
				if (curr_scope[index].init_line >= line || curr_scope[index].init_line == 0)
				{
					temp_uninit.use_line = line;
					temp_uninit.var_id = srchString;
					not_init_output.push_back(temp_uninit);
				}
			}

		}

		else // not found
		{
			//serching var_decl_vector from the bottom up
			int i = var_decl_vector.size() - 1;

			while (!found && i >= 0)
			{
				if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
				{
					found = true;

					int index = 0;
					while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
					{
						index = index + 1;
					}
					if (index < var_decl_vector[i].size())
					{
						if (var_decl_vector[i][index].init_line >= line || var_decl_vector[i][index].init_line == 0)
						{
							temp_uninit.use_line = line;
							temp_uninit.var_id = srchString;
							not_init_output.push_back(temp_uninit);
						}
					}


				}
				i = i - 1;
			}
		}
	}

}


void undecl_var_error()
{
	if (token.token_type == ID)
	{
		bool found = FoundInVector(token.lexeme, 0, curr_scope);

		if (found)
		{
			int index = 0;
			while (token.lexeme != curr_scope[index].var_id && index < curr_scope.size())
			{
				index = index + 1;
			}
			if (index < curr_scope.size())
			{
				curr_scope[index].var_used = true;
			}
			else
			{
				cout << "in undecl_var_error went past curr_scope index\n";
			}
		}

		else // not found
		{
			//serching var_decl_vector from the bottom up
			int i = var_decl_vector.size() - 1;

			while (!found && i >= 0)
			{
				if (FoundInVector(token.lexeme, 0, var_decl_vector[i]))
				{
					found = true;

					int index = 0;
					while (token.lexeme != var_decl_vector[i][index].var_id && index < var_decl_vector[i].size())
					{
						index = index + 1;
					}
					if (index < var_decl_vector[i].size())
					{
						var_decl_vector[i][index].var_used = true;
					}
					else
					{
						cout << "in undecl_var_error went past var_decl_vector[" << i << "] index\n";
					}

				}
				i = i - 1;
			}
		}

		if (!found)
		{
			// varibale is undeclared in scope
			var_decl_error.code = "1.2";
			var_decl_error.var_id = token.lexeme;
		}
	}
}

void var_not_used_error()
{
	for (int i = 0; i < curr_scope.size(); i++)
	{
		if (curr_scope[i].var_used == false)
		{
			// varibale is unused in scope
			var_decl_error.code = "1.3";
			var_decl_error.var_id = curr_scope[i].var_id;
		}
	}
}

void built_in_type_check(string varName)
{
	if (varName == "REAL" ||
		varName == "INT" ||
		varName == "BOOLEAN" ||
		varName == "STRING")
	{
		syntax_error("built_in_type_check 1");
	}
}

void set_mismatch_error(int line, string code)
{
	type_mismatch_error.code = code;
	type_mismatch_error.line_num = line;
}



int main()
{
	var_decl_error.code = "NONE"; // initialize error code
	type_mismatch_error.code = "NONE"; // initialize error code
	curr_scope.clear();  // initialize curr_scope
	temp_id_list.clear();
	parse_prog();

	//print_scope(curr_scope);

	token = lexer.GetToken();
	if (token.token_type != END_OF_FILE)
	{
		syntax_error("the end of file was not found");
	}

	if (Syntax_error_happend)
	{
		cout << "Syntax Error\n";
	}
	else if (var_decl_error.code != "NONE")
	{
		cout << "ERROR CODE " << var_decl_error.code << " " << var_decl_error.var_id << "\n";
	}
	else if (type_mismatch_error.code != "NONE")
	{
		cout << "TYPE MISMATCH " << type_mismatch_error.line_num << " " << type_mismatch_error.code << "\n";
	}
	else if (!not_init_output.empty())
	{
		for (int i = 0; i < not_init_output.size(); i++)
		{
			cout <<"UNINITIALIZED "<< not_init_output[i].var_id << " " << not_init_output[i].use_line << "\n";
		}
	}
	else
	{
		for (int i = 0; i < output.size(); i++)
		{
			cout << output[i].var_id << " " << output[i].use_line << " " << output[i].decl_line << "\n";
		}
	}
}
