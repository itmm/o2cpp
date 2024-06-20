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

void State::advance() {
	while (is_whitespace(ch)) {
		ch = in.get();
	}

	constexpr int dot_dot { 1000 };

	if (ch == EOF) { token = Token::eof; return; }

	if (is_letter(ch)) {
		value.clear();
		while (is_letter(ch) || is_digit(ch)) {
			value += static_cast<char>(ch); ch = in.get();
		}
		auto got { keywords.find(value) };
		token = got == keywords.end() ? Token::identifier : got->second;
		return;
	}

	if (is_digit(ch)) {
		value.clear();
		bool is_hex { false };
		for (;;) {
			if (is_digit(ch)) {
				value += static_cast<char>(ch); ch = in.get();
			} else if (ch >= 'A' && ch <= 'F') {
				value += static_cast<char>(ch); ch = in.get();
				is_hex = true;
			} else { break; }
		}
		if (ch == 'H') {
			is_hex = true;
			ch = in.get();
		} else if (ch == 'X') {
			ch = in.get();
			token = Token::char_literal;
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
			if (is_hex) {
				token = Token::unknown;
				return;
			}
			while (is_digit(ch)) {
				value += static_cast<char>(ch); ch = in.get();
			}
			if (*end == 'E') {
				++end;
				if (*end == '+' || *end == '-') { ++end; }
				if (!char_info::is_digit(*end)) {
					form_token(end, token::unknown);
					return;
				}
				while (char_info::is_digit(*end)) { ++end; }
			}
			form_token(end, token::float_literal);
			return;
		}
		form_token(end, token::integer_literal);
		return;
	}

	switch (*ptr_) {
		#define TOK(ch, kind) case ch: form_token(ptr_ + 1, kind); break;
		TOK('+', token::plus)
		TOK('-', token::minus)
		TOK('*', token::star)
		TOK('/', token::slash)
		TOK(')', token::right_parenthesis)
		TOK(',', token::comma)
		TOK(';', token::semicolon)
		TOK('=', token::equals)
		TOK('#', token::not_equals)
		TOK('|', token::bar)
		TOK('[', token::left_bracket)
		TOK(']', token::right_bracket)
		TOK('^', token::ptr)
		TOK('&', token::andop)
		TOK('~', token::notop)
		TOK('{', token::left_brace)
		TOK('}', token::right_brace)
		#undef TOK
		case '.':
			if (ptr_[1] == '.') {
				form_token(ptr_ + 2, token::range);
			} else {
				form_token(ptr_ + 1, token::period);
			}
			break;
		case ':':
			if (ptr_[1] == '=') {
				form_token(ptr_ + 2, token::assign);
			} else {
				form_token(ptr_ + 1, token::colon);
			}
			break;
		case '<':
			if (ptr_[1] == '=') {
				form_token(ptr_ + 2, token::less_or_equal);
			} else {
				form_token(ptr_ + 1, token::less);
			}
			break;
		case '>':
			if (ptr_[1] == '=') {
				form_token(ptr_ + 2, token::greater_or_equal);
			} else {
				form_token(ptr_ + 1, token::greater);
			}
			break;
		case '"': {
			const char* end = ptr_ + 1;
			while (*end && *end != '"') { ++end; }
			if (!*end) {
				form_token(end, token::unknown);
			} else {
				form_token(end + 1, token::string_literal);
			}
			break;
		}
		case '(':
			if (ptr_[1] == '*') {
				do_comment();
			} else {
				form_token(ptr_ + 1, token::left_parenthesis);
			}
			break;
		default:
			form_token(ptr_ + 1, token::unknown);
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

void parse_import_list();
void parse_declaration_sequence();
void parse_statement_sequence();

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
		parse_import_list();
	}
	parse_declaration_sequence();
	if (state.token == Token::BEGIN) {
		state.advance();
		parse_statement_sequence();
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

void parse_import_list() {
	throw Error { "parse_import_list not implemented" };
}

void parse_declaration_sequence() {
	throw Error { "parse_declaration_sequence not implemented" };
}

void parse_statement_sequence() {
	throw Error { "parse_statement_sequence not implemented" };
}

