#include <iostream>
#include <fstream>
#include <map>
#include <utility>

#include "Scanner.h"

void convert(const std::string& path);

using Error = std::runtime_error;

int main(int argc, const char** argv) {
	Scanner_init_module();
	try {
		for (int i = 1; i < argc; ++i) {
			convert(argv[i]);
		}
	}
	catch (const Error& err) {
		std::cerr << err.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

using Token = SYSTEM_INTEGER;

struct State {
	const std::string base;
	std::ifstream& in;
	std::ofstream& h;
	std::ofstream& cxx;
	int ch { ' ' };

	std::string value { };
	std::map<std::string, std::string> module_mapping;
	int level { 1 };

	void next();
	void add_ch_to_value();
	void set_token(const Token& tok);
	void set_bi_char_token(char trigger, const Token& with_trigger, const Token& others);
	void do_comment();
	void indent();

	void advance();

	State(
		std::string base, std::ifstream& in,
		std::ofstream& h, std::ofstream& cxx
	):
		base { std::move(base) }, in { in }, h { h }, cxx { cxx }
	{ module_mapping["SYSTEM"] = "SYSTEM"; Scanner_token = Token_unknown; advance(); }

	void expect(const Token& token) const;
	void consume(const Token& token);
};

void State::indent() {
	for (int i { level }; i > 0; --i) { cxx << '\t'; }
}

std::string token_name(const Token& token, const std::string& value) {
	switch (token) {
		case Token_unknown: return "unknown";
		case Token_eof: return "eof";
		case Token_identifier:	return "identifier " + value;
		case Token_integerLiteral: return "integer " + value;
		case Token_floatLiteral: return "float " + value;
		case Token_stringLiteral: return "\"" + value + "\"";
		case Token_charLiteral: return "'\\x" + value + "'";
		case Token_plus: return "+";
		case Token_minus: return "-";
		case Token_star: return "*";
		case Token_slash: return "/";
		case Token_leftParenthesis: return "(";
		case Token_rightParenthesis: return ")";
		case Token_semicolon: return ";";
		case Token_period: return ".";
		case Token_comma: return ",";
		case Token_colon: return ":";
		case Token_assign: return ":=";
		case Token_equals: return "=";
		case Token_bar: return "|";
		case Token_notEquals: return "#";
		case Token_leftBracket: return "[";
		case Token_rightBracket: return "]";
		case Token_ptr: return "^";
		case Token_andop: return "&";
		case Token_notop: return "~";
		case Token_leftBrace: return "{";
		case Token_rightBrace: return "}";
		case Token_less: return "<";
		case Token_lessOrEqual: return "<=";
		case Token_greater: return ">";
		case Token_greaterOrEqual: return ">=";
		case Token_range: return "..";
		case Token_kwARRAY: return "ARRAY";
		case Token_kwBEGIN: return "BEGIN";
		case Token_kwBY: return "BY";
		case Token_kwCASE: return "CASE";
		case Token_kwCONST: return "CONST";
		case Token_kwDIV: return "DIV";
		case Token_kwDO: return "DO";
		case Token_kwEND: return "END";
		case Token_kwELSE: return "ELSE";
		case Token_kwELSIF: return "ELSIF";
		case Token_kwFALSE: return "FALSE";
		case Token_kwFOR: return "FOR";
		case Token_kwIF: return "IF";
		case Token_kwIMPORT: return "IMPORT";
		case Token_kwIN: return "IN";
		case Token_kwIS: return "IS";
		case Token_kwMOD: return "MOD";
		case Token_kwMODULE: return "MODULE";
		case Token_kwNIL: return "NIL";
		case Token_kwOF: return "OF";
		case Token_kwOR: return "OR";
		case Token_kwPOINTER: return "POINTER";
		case Token_kwPROCEDURE: return "PROCEDURE";
		case Token_kwRECORD: return "RECORD";
		case Token_kwREPEAT: return "REPEAT";
		case Token_kwRETURN: return "RETURN";
		case Token_kwTHEN: return "THEN";
		case Token_kwTO: return "TO";
		case Token_kwTRUE: return "TRUE";
		case Token_kwTYPE: return "TYPE";
		case Token_kwUNTIL: return "UNTIL";
		case Token_kwVAR: return "VAR";
		case Token_kwWHILE: return "WHILE";
		default: return "??";
	}
}

void State::expect(const Token& tok) const {
	if (tok != Scanner_token) {
		throw Error {
			"wrong token " + token_name(Scanner_token, value) +
			" (expected " + token_name(tok, "") + ")"
		};
	}
}

std::map<std::string, Token> keywords {
	{ "ARRAY", Token_kwARRAY }, { "BEGIN", Token_kwBEGIN },
	{ "BY", Token_kwBY }, { "CASE", Token_kwCASE }, { "CONST", Token_kwCONST },
	{ "DIV", Token_kwDIV }, { "DO", Token_kwDO }, { "END", Token_kwEND},
	{ "ELSE", Token_kwELSE }, { "ELSIF", Token_kwELSIF },
	{ "FALSE", Token_kwFALSE }, { "FOR", Token_kwFOR }, { "IF", Token_kwIF },
	{ "IMPORT", Token_kwIMPORT }, { "IN", Token_kwIN }, { "IS", Token_kwIS },
	{ "MOD", Token_kwMOD }, { "MODULE", Token_kwMODULE },
	{ "NIL", Token_kwNIL }, { "OF", Token_kwOF }, { "OR", Token_kwOR },
	{ "POINTER", Token_kwPOINTER }, { "PROCEDURE", Token_kwPROCEDURE },
	{ "RECORD", Token_kwRECORD }, { "REPEAT", Token_kwREPEAT },
	{ "RETURN", Token_kwRETURN }, { "THEN", Token_kwTHEN },
	{ "TO", Token_kwTO }, { "TRUE", Token_kwTRUE }, { "TYPE", Token_kwTYPE },
	{ "UNTIL", Token_kwUNTIL }, { "VAR", Token_kwVAR },
	{ "WHILE", Token_kwWHILE }
};

inline bool is_whitespace(int c) {
	return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
		   c == '\r' || c == '\n';
}

inline bool is_digit(int c) {
	return c >= '0' && c <= '9';
}

inline bool is_letter(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void State::add_ch_to_value() {
	value += static_cast<char>(ch);
	ch = in.get();
}

void State::next() { if (ch != EOF) { ch = in.get(); } }
void State::set_token(const Token& tok) { Scanner_token = tok; next(); }

void State::set_bi_char_token(
	char trigger, const Token& with_trigger, const Token& others
) {
	next();
	if (ch == trigger) {
		set_token(with_trigger);
	} else {
		Scanner_token = others;
	}
}

void State::advance() {
	while (ch != EOF && is_whitespace(ch)) { next(); }

	constexpr int dot_dot { 1000 };

	if (ch == EOF) { Scanner_token = Token_eof; return; }

	if (is_letter(ch)) {
		value.clear();
		while (is_letter(ch) || is_digit(ch)) { add_ch_to_value(); }
		auto got { keywords.find(value) };
		Scanner_token = got == keywords.end() ? Token_identifier : got->second;
		return;
	}

	if (is_digit(ch)) {
		value.clear();
		bool is_hex { false };
		for (;;) {
			if (is_digit(ch)) {
				add_ch_to_value();
			} else if (ch >= 'A' && ch <= 'F') {
				add_ch_to_value();
				is_hex = true;
			} else { break; }
		}
		if (ch == 'H') {
			value = "0x" + value;
			is_hex = true;
			next();
		} else if (ch == 'X') {
			set_token(Token_charLiteral);
			return;
		} else if (is_hex) {
			Scanner_token = Token_unknown;
			return;
		}
		if (ch == '.') {
			ch = in.get();
			if (ch == '.') {
				ch = dot_dot;
				Scanner_token = Token_integerLiteral;
				return;
			}
			value += '.';
			if (is_hex) { set_token(Token_unknown); return; }
			while (is_digit(ch)) { add_ch_to_value(); }
			if (ch == 'E') {
				add_ch_to_value();
				if (ch == '+' || ch == '-') { add_ch_to_value(); }
				if (!is_digit(ch)) {
					Scanner_token = Token_unknown;
					return;
				}
				while (is_digit(ch)) { add_ch_to_value(); }
			}
			Scanner_token = Token_floatLiteral;
			return;
		}
		Scanner_token = Token_integerLiteral;
		return;
	}

	switch (ch) {
		case '+': set_token(Token_plus); break;
		case '-': set_token(Token_minus); break;
		case '*': set_token(Token_star); break;
		case '/': set_token(Token_slash); break;
		case ')': set_token(Token_rightParenthesis); break;
		case ',': set_token(Token_comma); break;
		case ';': set_token(Token_semicolon); break;
		case '=': set_token(Token_equals); break;
		case '#': set_token(Token_notEquals); break;
		case '|': set_token(Token_bar); break;
		case '[': set_token(Token_leftBracket); break;
		case ']': set_token(Token_rightBracket); break;
		case '^': set_token(Token_ptr); break;
		case '&': set_token(Token_andop); break;
		case '~': set_token(Token_notop); break;
		case '{': set_token(Token_leftBrace); break;
		case '}': set_token(Token_rightBrace); break;
		case '.': set_bi_char_token('.', Token_range, Token_period); break;
		case dot_dot: set_token(Token_range); break;
		case ':': set_bi_char_token('=', Token_assign, Token_colon); break;
		case '<': set_bi_char_token('=', Token_lessOrEqual, Token_less); break;
		case '>':
			set_bi_char_token('=', Token_greaterOrEqual, Token_greater); break;
		case '"':
			next();
			value.clear();
			while (ch != EOF && ch != '"') { add_ch_to_value(); }
			if (ch != '"') {
				Scanner_token = Token_unknown;
			} else {
				set_token(Token_stringLiteral);
			}
			break;
		case '(':
			next();
			if (ch == '*') {
				do_comment();
			} else {
				Scanner_token = Token_leftParenthesis;
			}
			break;
		default: set_token(Token_unknown);
	}
}

void State::consume(const Token& tok) {
	expect(tok);
	advance();
}

void parse_module(State& state);

void convert(const std::string& path) {
	std::cout << "converting " << path << "\n";
	if (path.size() < 4 || path.substr(path.size() - 4) != ".Mod") {
		throw Error { "no mod file" };
	}
	auto base_path { path.substr(0, path.size() - 4) };
	auto h_path { base_path + ".h" };
	auto cxx_path { base_path + ".cpp" };
	auto start_of_file { base_path.rfind('/') };
	auto base {
		start_of_file == std::string::npos ?
			base_path : base_path.substr(start_of_file + 1)
	};

	std::ifstream mod_file { path.c_str() };
	std::ofstream h_file { h_path.c_str() };
	std::ofstream cxx_file { cxx_path.c_str() };

	h_file << "#pragma once\n\n#include \"SYSTEM.h\"\n\n";
	cxx_file << "#include \"" << base << ".h\"\n\n";
	State state { base, mod_file, h_file, cxx_file };
	parse_module(state);
}

void parse_import_list(State& state);
void parse_declaration_sequence(State &state);
void parse_statement_sequence(State& state);

void parse_module(State& state) {
	state.consume(Token_kwMODULE);
	state.expect(Token_identifier);
	auto module_name { state.value };
	if (module_name != state.base) {
		throw Error { "MODULE name doesn't match file name" };
	}
	state.advance();
	state.consume(Token_semicolon);
	state.cxx << "static void init_module_imports() {\n";

	if (Scanner_token == Token_kwIMPORT) {
		parse_import_list(state);
	}
	state.cxx << "}\n\n";

	parse_declaration_sequence(state);
	state.h << "void " << state.base << "_init_module();\n";
	state.cxx << "void " << state.base << "_init_module() {\n";
	state.indent(); state.cxx << "static bool already_run { false };\n";
	state.indent(); state.cxx << "if (already_run) { return; }\n";
	state.indent(); state.cxx << "already_run = true;\n";
	state.indent(); state.cxx << "init_module_imports();\n";
	if (Scanner_token == Token_kwBEGIN) {
		state.advance();
		parse_statement_sequence(state);
	}
	state.consume(Token_kwEND);
	state.expect(Token_identifier);
	if (module_name != state.value) {
		throw Error { "MODULE names don't match" };
	}
	state.advance();
	state.consume(Token_period);
	state.expect(Token_eof);
	state.cxx << "}\n";
}

void State::do_comment() {
	throw Error { "comments not implemented" };
}

void parse_import(State& state);

void parse_import_list(State& state) {
	state.consume(Token_kwIMPORT);
	parse_import(state);
	while (Scanner_token == Token_comma) {
		state.advance();
		parse_import(state);
	}
	state.consume(Token_semicolon);
	state.h << "\n";
}

void parse_import(State& state) {
	state.expect(Token_identifier);
	auto name { state.value };
	auto full_name { name };
	state.advance();
	if (Scanner_token == Token_assign) {
		state.advance();
		state.expect(Token_identifier);
		full_name = state.value;
		state.advance();
	}
	state.module_mapping[name] = full_name;
	state.indent(); state.cxx << full_name << "_init_module();\n";
	state.h << "#include \"" << full_name << ".h\"\n";
}

void parse_const_declaration(State& state);
void parse_type_declaration();
void parse_variable_declaration(State& state);
void parse_procedure_declaration();

void parse_declaration_sequence(State& state) {
	if (Scanner_token == Token_kwCONST) {
		state.advance();
		while (Scanner_token == Token_identifier) {
			parse_const_declaration(state);
			state.consume(Token_semicolon);
		}
	}
	if (Scanner_token == Token_kwTYPE) {
		state.advance();
		while (Scanner_token == Token_identifier) {
			parse_type_declaration();
			state.consume(Token_semicolon);
		}
	}
	if (Scanner_token == Token_kwVAR) {
		state.advance();
		while (Scanner_token == Token_identifier) {
			parse_variable_declaration(state);
			state.consume(Token_semicolon);
		}
	}

	while (Scanner_token == Token_kwPROCEDURE) {
		parse_procedure_declaration();
		state.consume(Token_semicolon);
	}
}

std::string parse_ident_def(State& state);
void parse_const_expression(State& state);

void parse_const_declaration(State& state) {
	state.h << "constexpr auto ";
	state.h << parse_ident_def(state);
	state.consume(Token_equals);
	state.h << " { ";
	parse_const_expression(state);
	state.h << " };\n";
}

std::string parse_ident_def(State& state) {
	state.expect(Token_identifier);
	auto result { state.base + "_" + state.value };
	state.advance();
	if (Scanner_token == Token_star) {
		state.advance();
	}
	return result;
}

std::string parse_expression(State& state);

void parse_const_expression(State& state) {
	state.h << parse_expression(state);
}

void parse_type_declaration() {
	throw Error { "parse_type_declaration not implemented" };
}

std::string parse_ident_list(State& state);
std::string parse_type(State& state);

void parse_variable_declaration(State& state) {
	auto idents { parse_ident_list(state) };
	state.consume(Token_colon);
	auto type { parse_type(state) };
	state.h << "extern " << type << " " << idents << ";\n";
	state.cxx << type << " " << idents << ";\n";
}

std::string parse_ident_list(State& state) {
	auto idents { parse_ident_def(state) };
	while (Scanner_token == Token_comma) {
		state.advance();
		idents += ", ";
		idents += parse_ident_def(state);
	}
	return idents;
}

std::string parse_qual_ident(State& state);
std::string parse_array_type(State& state);
std::string parse_record_type(State& state);
std::string parse_pointer_type(State& state);
std::string parse_procedure_type(State& state);

std::string parse_type(State& state) {
	if (Scanner_token == Token_identifier) {
		return parse_qual_ident(state);
	} else if (Scanner_token == Token_kwARRAY) {
		return parse_array_type(state);
	} else if (Scanner_token == Token_kwRECORD) {
		return parse_record_type(state);
	} else if (Scanner_token == Token_kwPOINTER) {
		return parse_pointer_type(state);
	} else if (Scanner_token == Token_kwPROCEDURE) {
		return parse_procedure_type(state);
	} else {
		throw Error { "type expected" };
	}
}

std::string parse_array_type(State& state) {
	throw Error { "parse_array_type not implemented" };
}

std::string parse_record_type(State& state) {
	throw Error { "parse_record_type not implemented" };
}

std::string parse_pointer_type(State& state) {
	throw Error { "parse_pointer_type not implemented" };
}

std::string parse_procedure_type(State& state) {
	throw Error { "parse_procedure_type not implemented" };
}

void parse_procedure_declaration() {
	throw Error { "parse_procedure_declaration not implemented" };
}

void parse_statement(State& state);

void parse_statement_sequence(State& state) {
	parse_statement(state);
	while (Scanner_token == Token_semicolon) {
		state.advance();
		parse_statement(state);
	}
}

void parse_assignment_or_procedure_call(State& state);
void parse_if_statement(State& state);
void parse_case_statement();
void parse_while_statement();
void parse_repeat_statement();
void parse_for_statement();

void parse_statement(State& state) {
	if (Scanner_token == Token_identifier) {
		parse_assignment_or_procedure_call(state);
	} else if (Scanner_token == Token_kwIF) {
		parse_if_statement(state);
	} else if (Scanner_token == Token_kwCASE) {
		parse_case_statement();
	} else if (Scanner_token == Token_kwWHILE) {
		parse_while_statement();
	} else if (Scanner_token == Token_kwREPEAT) {
		parse_repeat_statement();
	} else if (Scanner_token == Token_kwFOR) {
		parse_for_statement();
	}
}

std::string parse_designator(State& state);
std::string parse_actual_parameters(State& state);

void parse_assignment_or_procedure_call(State& state) {
	state.indent(); state.cxx << parse_designator(state);
	if (Scanner_token == Token_assign) {
		state.advance();
		state.cxx << " = " << parse_expression(state) << ";\n";
	} else {
		if (Scanner_token == Token_leftParenthesis) {
			state.cxx << "(";
			state.cxx << parse_actual_parameters(state);
			state.cxx << ");\n";
		} else { state.cxx << "();\n"; }
	}
}

std::string parse_expression_list(State& state, const char* separator);

std::string parse_designator(State& state) {
	auto qual_ident { parse_qual_ident(state) };

	for (;;) {
		if (Scanner_token == Token_period) {
			state.advance();
			state.expect(Token_identifier);
			qual_ident = "(" + qual_ident + ")." + state.value;
			state.advance();
		} else if (Scanner_token == Token_leftBracket) {
			state.advance();
			qual_ident = "(" + qual_ident + ")[";
			qual_ident += parse_expression_list(state, "][");
			state.consume(Token_rightBracket);
			qual_ident += "]";
		} else if (Scanner_token == Token_ptr) {
			qual_ident = "*(" + qual_ident + ")";
			state.advance();
			/* TODO: Implement cast
		} else if (token::is(token::left_parenthesis)) {
			advance();
			auto type { type::Type::as_type(parse_qual_ident()) };
			if (!type) { diag::report(diag::err_type_expected); }
			consume(token::right_parenthesis);
			expression = expr::Unary::create(
				type, token::left_parenthesis, expression
			);
			 */
		} else { break; }
	}
	return qual_ident;
}

std::string parse_qual_ident(State& state) {
	state.expect(Token_identifier);
	auto name { state.value };
	state.advance();
	auto module { state.module_mapping.find(name) };
	if (module != state.module_mapping.end()) {
		if (Scanner_token == Token_period) {
			state.advance();
			state.expect(Token_identifier);
			name = state.module_mapping[name] + "_" + state.value;
			state.advance();
		} else { throw Error { ". after module expected" }; }
	} else if (name == "INTEGER") {
		name = "SYSTEM_INTEGER";
	} else {
		name = state.base + "_" + name;
	}
	return name;
}

std::string parse_expression_list(State& state, const char* separator) {
	auto result { parse_expression(state) };
	while (Scanner_token == Token_comma) {
		state.advance();
		result += separator;
		result += parse_expression(state);
	}
	return result;
}

std::string parse_simple_expression(State& state);

std::string parse_expression(State& state) {
	auto result { parse_simple_expression(state) };
	for (;;) {
		switch (Scanner_token) {
			case Token_equals: result += " == "; break;
			case Token_notEquals: result += " != "; break;
			case Token_less: result += " < "; break;
			case Token_lessOrEqual: result += " <= "; break;
			case Token_greater: result += " > "; break;
			case Token_greaterOrEqual: result += " >= "; break;
			// TODO: Token::IN
			// TODO: Token::IS
			default: return result;
		}
		state.advance();
		result += parse_simple_expression(state);
	}
}

std::string parse_term(State& state);

std::string parse_simple_expression(State& state) {
	std::string result;
	if (Scanner_token == Token_plus) {
		result += "+"; state.advance();
	} else if (Scanner_token == Token_minus) {
		result += "-"; state.advance();
	}
	result += parse_term(state);

	for (;;) {
		switch (Scanner_token) {
			case Token_plus: result += " + "; break;
			case Token_minus: result += " - "; break;
			case Token_kwOR: result += " || "; break;
			default: return result;
		}
		state.advance();
		result += parse_term(state);
	}
}

std::string parse_factor(State& state);

std::string parse_term(State& state) {
	auto result { parse_factor(state) };

	for (;;) {
		char* postfix = "";
		switch (Scanner_token) {
			case Token_star: result += " * "; break;
			case Token_slash: result = "static_cast<double>(" + result + ") / "; break;
			case Token_kwDIV: result += "static_cast<int>(" + result + " / "; postfix = ")"; break;
			case Token_kwMOD: result += " % "; break;
			case Token_andop: result += " && "; break;
			default: return result;
		}
		state.advance();
		result += parse_factor(state);
		result += postfix;
	}
}

std::string parse_set();

std::string parse_factor(State& state) {
	switch (Scanner_token) {
		case Token_integerLiteral:
		case Token_floatLiteral: {
			auto result { state.value };
			state.advance();
			return result;
		}
		case Token_stringLiteral: {
			auto result { "\"" + state.value + "\"" };
			state.advance();
			return result;
		}
		case Token_charLiteral: {
			auto result { "'\\x" + state.value + "'" };
			state.advance();
			return result;
		}
		case Token_kwNIL:
			state.advance();
			return "nullptr";
		case Token_kwTRUE:
			state.advance();
			return "true";
		case Token_kwFALSE:
			state.advance();
			return "false";
		case Token_leftBrace:
			return parse_set();
		case Token_identifier: {
			auto result { parse_designator(state) };
			if (Scanner_token == Token_leftParenthesis) {
				result += "(";
				result += parse_actual_parameters(state);
				result += ")";
			}
			return result;
		}
		case Token_leftParenthesis: {
			state.advance();
			auto inner { parse_expression(state) };
			state.consume(Token_rightParenthesis);
			return "(" + inner + ")";
		}
		case Token_notop: {
			state.advance();
			auto factor { parse_expression(state) };
			return "!(" + factor + ")";
		}
		default: throw Error { "factor expected "};
	}
}

std::string parse_set() {
	throw Error { "parse_set not implemented" };
}

std::string parse_actual_parameters(State& state) {
	std::string result;
	state.consume(Token_leftParenthesis);
	if (Scanner_token != Token_rightParenthesis) {
		result = parse_expression_list(state, ", ");
	}
	state.consume(Token_rightParenthesis);
	return result;
}

void parse_if_statement(State& state) {
	state.consume(Token_kwIF);
	state.indent(); state.cxx << "if (" << parse_expression(state) << ") {\n";
	state.consume(Token_kwTHEN);
	++state.level;
	parse_statement_sequence(state);
	--state.level;
	while (Scanner_token == Token_kwELSIF) {
		state.advance();
		state.indent(); state.cxx << "} else if (" << parse_expression(state) << ") {\n";
		state.consume(Token_kwTHEN);
		++state.level;
		parse_statement_sequence(state);
		--state.level;
	}
	if (Scanner_token == Token_kwELSE) {
		state.advance();
		state.indent(); state.cxx << "} else {\n";
		++state.level;
		parse_statement_sequence(state);
		--state.level;
	}
	state.consume(Token_kwEND);
	state.indent(); state.cxx << "}\n";
}

void parse_case_statement() {
	throw Error { "parse_case_statement not implemented" };
}

void parse_while_statement() {
	throw Error { "parse_while_statement not implemented" };
}

void parse_repeat_statement() {
	throw Error { "parse_repeat_statement not implemented" };
}

void parse_for_statement() {
	throw Error { "parse_for_statement not implemented" };
}

