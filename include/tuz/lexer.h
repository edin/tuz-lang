#pragma once

#include "token.h"

#include <optional>
#include <string_view>
#include <vector>
#include <functional>

namespace tuz {

using LexPredicate = bool(*)(char);

class Lexer {
public:
  explicit Lexer(std::string_view source);

  // Tokenize the entire source and return all tokens
  std::vector<Token> tokenize();

  // Get next token (for streaming)
  Token next_token();

  // Peek at current character
  char peek() const { return current_; }

private:
  std::string_view source_;
  size_t position_;
  uint32_t line_;
  uint32_t column_;
  char current_;

  void advance();
  void skip_whitespace();
  bool skip_comment();
  void skip_ignored();

  Token make_token(TokenType type, std::string_view text);

  Token identifier();
  Token number();
  Token string();
  
  bool try_consume(std::string_view value);
  void advance_while(LexPredicate predicate);
  bool advance_if(std::string_view chars);
  bool is_at_end();

  Location current_location() const;

  static bool is_alpha(char c);
  static bool is_digit(char c);
  static bool is_alphanumeric(char c);
  static bool is_identifier_start(char c);
  static bool is_identifier(char c);
  static bool is_whitespace(char c);
  static bool is_string_start(char c);
  

  static std::optional<TokenType> get_keyword(std::string_view text);
};

} // namespace tuz
