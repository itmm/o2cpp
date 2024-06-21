#include <iostream>
#include <fstream>
#include <map>
#include <utility>

void convert(const std::string& path);

using Error = std::runtime_error;

int main(int argc, const char** argv) {
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

enum class Token {
	unknown, eof, identifier, integer_literal, float_literal, string_literal,
	char_literal, plus, minus, star, slash, left_parenthesis, right_parenthesis,
	semicolon, period, comma, colon, assign, equals, bar, not_equals,
	left_bracket, right_bracket, ptr, andop, notop, left_brace, right_brace,
	less, less_or_equal, greater, greater_or_equal, range, ARRAY, BEGIN, BY,
	CASE, CONST, DIV, DO, END, ELSE, ELSIF, FALSE, FOR, IF, IMPORT, IN, IS, MOD,
	MODULE, NIL, OF, OR, POINTER, PROCEDURE, RECORD, REPEAT, RETURN, THEN, TO,
	TRUE, TYPE, UNTIL, VAR, WHILE
};

struct State {
	const std::string base;
	std::ifstream& in;
	std::ofstream& h;
	std::ofstream& cxx;
	int ch { ' ' };
	Token token { Token::unknown };

	std::string value { };
	std::map<std::string, std::string> module_mapping;

	void next();
	void add_ch_to_value();
	void set_token(const Token& tok);
	void set_bi_char_token(char trigger, const Token& with_trigger, const Token& others);
	void do_comment();

	void advance();

	State(
		std::string base, std::ifstream& in,
		std::ofstream& h, std::ofstream& cxx
	):
		base { std::move(base) }, in { in }, h { h }, cxx { cxx }
	{ advance(); }

	void expect(const Token& token) const;
	void consume(const Token& token);
};

void State::expect(const Token& tok) const {
	if (tok != token) {
		throw Error { "wrong token" };
	}
}

std::map<std::string, Token> keywords {
	{ "ARRAY", Token::ARRAY }, { "BEGIN", Token::BEGIN }, { "BY", Token::BY },
	{ "CASE", Token::CASE }, { "CONST", Token::CONST }, { "DIV", Token::DIV },
	{ "DO", Token::DO }, { "END", Token::END}, { "ELSE", Token::ELSE },
	{ "ELSIF", Token::ELSIF }, { "FALSE", Token::FALSE }, { "FOR", Token::FOR },
	{ "IF", Token::IF }, { "IMPORT", Token::IMPORT }, { "IN", Token::IN },
	{ "IS", Token::IS }, { "MOD", Token::MOD }, { "MODULE", Token::MODULE },
	{ "NIL", Token::NIL }, { "OF", Token::OF }, { "OR", Token::OR },
	{ "POINTER", Token::POINTER }, { "PROCEDURE", Token::PROCEDURE },
	{ "RECORD", Token::RECORD }, { "REPEAT", Token::REPEAT },
	{ "RETURN", Token::RETURN }, { "THEN", Token::THEN }, { "TO", Token::TO },
	{ "TRUE", Token::TRUE }, { "TYPE", Token::TYPE }, { "UNTIL", Token::UNTIL },
	{ "VAR", Token::VAR }, { "WHILE", Token::WHILE }
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
void State::set_token(const Token& tok) { token = tok; next(); }

void State::set_bi_char_token(char trigger, const Token& with_trigger, const Token& others) {
	next();
	if (ch == trigger) {
		set_token(with_trigger);
	} else {
		token = others;
	}
}

void State::advance() {
	while (ch != EOF && is_whitespace(ch)) { next(); }

	constexpr int dot_dot { 1000 };

	if (ch == EOF) { token = Token::eof; return; }

	if (is_letter(ch)) {
		value.clear();
		while (is_letter(ch) || is_digit(ch)) { add_ch_to_value(); }
		auto got { keywords.find(value) };
		token = got == keywords.end() ? Token::identifier : got->second;
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
			is_hex = true;
			next();
		} else if (ch == 'X') {
			set_token(Token::char_literal);
			return;
		} else if (is_hex) {
			token = Token::unknown;
			return;
		}
		if (ch == '.') {
			ch = in.get();
			if (ch == '.') {
				ch = dot_dot;
				token = Token::integer_literal;
				return;
			}
			value += '.';
			if (is_hex) { set_token(Token::unknown); return; }
			while (is_digit(ch)) { add_ch_to_value(); }
			if (ch == 'E') {
				add_ch_to_value();
				if (ch == '+' || ch == '-') { add_ch_to_value(); }
				if (!is_digit(ch)) {
					token = Token::unknown;
					return;
				}
				while (is_digit(ch)) { add_ch_to_value(); }
			}
			token = Token::float_literal;
			return;
		}
		token = Token::integer_literal;
		return;
	}

	switch (ch) {
		case '+': set_token(Token::plus); break;
		case '-': set_token(Token::minus); break;
		case '*': set_token(Token::star); break;
		case '/': set_token(Token::slash); break;
		case ')': set_token(Token::right_parenthesis); break;
		case ',': set_token(Token::comma); break;
		case ';': set_token(Token::semicolon); break;
		case '=': set_token(Token::equals); break;
		case '#': set_token(Token::not_equals); break;
		case '|': set_token(Token::bar); break;
		case '[': set_token(Token::left_bracket); break;
		case ']': set_token(Token::right_bracket); break;
		case '^': set_token(Token::ptr); break;
		case '&': set_token(Token::andop); break;
		case '~': set_token(Token::notop); break;
		case '{': set_token(Token::left_brace); break;
		case '}': set_token(Token::right_brace); break;
		case '.': set_bi_char_token('.', Token::range, Token::period); break;
		case dot_dot: set_token(Token::range); break;
		case ':': set_bi_char_token('=', Token::assign, Token::colon); break;
		case '<': set_bi_char_token('=', Token::less_or_equal, Token::less); break;
		case '>': set_bi_char_token('=', Token::greater_or_equal, Token::greater); break;
		case '"':
			next();
			value.clear();
			while (ch != EOF && ch != '"') { add_ch_to_value(); }
			if (ch != '"') {
				token = Token::unknown;
			} else {
				set_token(Token::string_literal);
			}
			break;
		case '(':
			next();
			if (ch == '*') {
				do_comment();
			} else {
				token = Token::left_parenthesis;
			}
			break;
		default: set_token(Token::unknown);
	}
}

void State::consume(const Token& tok) {
	expect(tok);
	advance();
}

void parse_module(State& state);

void convert(const std::string& path) {
	std::cout << "converting " << path << "\n";
	if (path.size() < 4 || path.substr(path.size() - 4) != ".mod") {
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
	state.consume(Token::MODULE);
	state.expect(Token::identifier);
	auto module_name { state.value };
	if (module_name != state.base) {
		throw Error { "MODULE name doesn't match file name" };
	}
	state.advance();
	state.consume(Token::semicolon);
	if (state.token == Token::IMPORT) {
		parse_import_list(state);
	}
	parse_declaration_sequence(state);
	if (state.token == Token::BEGIN) {
		state.advance();
		parse_statement_sequence(state);
	}
	state.consume(Token::END);
	state.expect(Token::identifier);
	if (module_name != state.value) {
		throw Error { "MODULE names don't match" };
	}
	state.advance();
	state.consume(Token::period);
	state.expect(Token::eof);
}

void State::do_comment() {
	throw Error { "comments not implemented" };
}

void parse_import(State& state);

void parse_import_list(State& state) {
	state.consume(Token::IMPORT);
	parse_import(state);
	while (state.token == Token::comma) {
		state.advance();
		parse_import(state);
	}
	state.consume(Token::semicolon);
	state.h << "\n";
}

void parse_import(State& state) {
	state.expect(Token::identifier);
	auto name { state.value };
	auto full_name { name };
	state.advance();
	if (state.token == Token::assign) {
		state.advance();
		state.expect(Token::identifier);
		full_name = state.value;
		state.advance();
	}
	state.module_mapping[name] = full_name;
	state.h << "#include \"" << full_name << ".h\"\n";
}

void parse_const_declaration();
void parse_type_declaration();
void parse_variable_declaration();
void parse_procedure_declaration();

void parse_declaration_sequence(State& state) {
	if (state.token == Token::CONST) {
		state.advance();
		while (state.token == Token::identifier) {
			parse_const_declaration();
			state.consume(Token::semicolon);
		}
	}
	if (state.token == Token::TYPE) {
		state.advance();
		while (state.token == Token::identifier) {
			parse_type_declaration();
			state.consume(Token::semicolon);
		}
	}
	if (state.token == Token::VAR) {
		state.advance();
		while (state.token == Token::identifier) {
			parse_variable_declaration();
			state.consume(Token::semicolon);
		}
	}

	while (state.token == Token::PROCEDURE) {
		parse_procedure_declaration();
		state.consume(Token::semicolon);
	}
}

void parse_const_declaration() {
	throw Error { "parse_const_declaration not implemented" };
}

void parse_type_declaration() {
	throw Error { "parse_type_declaration not implemented" };
}

void parse_variable_declaration() {
	throw Error { "parse_variable_declaration not implemented" };
}

void parse_procedure_declaration() {
	throw Error { "parse_procedure_declaration not implemented" };
}

void parse_statement(State& state);

void parse_statement_sequence(State& state) {
	parse_statement(state);
	while (state.token == Token::semicolon) {
		state.advance();
		parse_statement(state);
	}
}

void parse_assignment_or_procedure_call();
void parse_if_statement();
void parse_case_statement();
void parse_while_statement();
void parse_repeat_statement();
void parse_for_statement();

void parse_statement(State& state) {
	if (state.token == Token::identifier) {
		parse_assignment_or_procedure_call();
	} else if (state.token == Token::IF) {
		parse_if_statement();
	} else if (state.token == Token::CASE) {
		parse_case_statement();
	} else if (state.token == Token::WHILE) {
		parse_while_statement();
	} else if (state.token == Token::REPEAT) {
		parse_repeat_statement();
	} else if (state.token == Token::FOR) {
		parse_for_statement();
	}
}

void parse_assignment_or_procedure_call() {
	throw Error { "parse_assigment_or_procedure_call not implemented" };
}

void parse_if_statement() {
	throw Error { "parse_if_statement not implemented" };
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

