#include "tuz/lexer.h"

#include <cctype>

namespace tuz {

Lexer::Lexer(std::string_view source)
    : source_(source), position_(0), line_(1), column_(1), current_(0) {
  if (!source_.empty()) {
    current_ = source_[0];
  }
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

    while (true) {
        auto token = next_token();
        tokens.push_back(token);

        if (token.type == TokenType::END_OF_FILE)
            break;
    }

  return tokens;
}


void Lexer::skip_ignored() {
  while (true) {
    skip_whitespace();
    if (!skip_comment())
      break;
  } 
}

Token Lexer::next_token() {
  skip_ignored();

  auto location = current_location();

  if (is_at_end()) {
    return Token(TokenType::END_OF_FILE, "", location);
  }

  // Identifiers and keywords
  if (is_identifier_start(current_)) {
    return identifier();
  }

  // Numbers
  if (is_digit(current_)) {
    return number();
  }

  // Strings
  if (is_string_start(current_)) {
    return string();
  }

  // Operators and delimiters
  // Note: Tokens are sorted by the length
  for (auto& token: Tokens) {
    if (try_consume(token.value)) {
        return Token(token.type, token.value, location);
    }
  }

  // Invalid token
  auto invalidChar = std::string(1, current_);
  advance();
  return Token(TokenType::INVALID,  invalidChar , location);
}

bool Lexer::try_consume(std::string_view value) {
    if (position_ + value.size() > source_.size())
      return false;

    if (std::string_view(source_.data() + position_, value.size()) != value)
      return false;

    for (size_t i = 0; i < value.size(); ++i) {
        advance();
    }
    return true;
}

bool Lexer::is_at_end() {
  return position_ >= source_.size();
}

void Lexer::advance_while(LexPredicate predicate) {
  while (!is_at_end() && predicate(current_)) {
    advance();
  }
}

bool Lexer::advance_if(std::string_view chars) {
    if (is_at_end())
      return false;

    if (chars.find(current_) != std::string_view::npos) {
        advance();
        return true;
    }
    return false;
}

void Lexer::advance() {
  if (current_ == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }

  position_++;
  if (position_ < source_.size()) {
    current_ = source_[position_];
  } else {
    current_ = '\0';
  }
}

void Lexer::skip_whitespace() {
  advance_while(&Lexer::is_whitespace);
}

bool Lexer::skip_comment() {

  if (try_consume("//")) {
      while (!is_at_end() && current_ != '\n')
          advance();
      return true;
  }
  else if (try_consume("/*"))
  {
      // TODO: Ensure if closing comment is present
      // TODO: Nested block comments
      while (!is_at_end()) {
          if (try_consume("*/"))
              break;
          advance();
      }
      return true;
  }
  return false;

}

Token Lexer::make_token(TokenType type, std::string_view text) {
  return Token(type, text, line_, column_ - static_cast<uint32_t>(text.length()));
}

Token Lexer::identifier() {
  auto location = current_location();
  auto start_pos = position_;

  advance_while(&Lexer::is_identifier);

  std::string_view text(source_.data() + start_pos, position_ - start_pos);

  auto keyword = get_keyword(text);

  if (keyword) {
    return Token(*keyword, text, location);
  }

  return Token(TokenType::IDENTIFIER, text, location);
}

Token Lexer::number() {
  auto location = current_location();
  auto start_pos = position_;
  auto is_float = false;

  advance_while(&Lexer::is_digit);

  if (advance_if(".")) {
    is_float = true;
    advance_while(&Lexer::is_digit);
  }

  // Exponent
  if (advance_if("eE")) {
    // TODO: Ensure number is present after e
    is_float = true;
    advance_if("+-");
    advance_while(&Lexer::is_digit);
  }

  std::string_view text(source_.data() + start_pos, position_ - start_pos);

  auto token_type = is_float 
    ? TokenType::FLOAT_LITERAL 
    : TokenType::INTEGER_LITERAL;

  return Token(token_type, text, location);
}

Token Lexer::string() {
  auto location = current_location();
  advance(); // skip opening quote

  std::string value;
  while (!is_at_end() && current_ != '"') {
    if (current_ == '\\') {
      advance();
      switch (current_) {
      case 'n':
        value += '\n';
        break;
      case 't':
        value += '\t';
        break;
      case 'r':
        value += '\r';
        break;
      case '\\':
        value += '\\';
        break;
      case '"':
        value += '"';
        break;
      case '0':
        value += '\0';
        break;
      default:
        value += current_;
        break;
      }
    } else {
      value += current_;
    }
    advance();
  }

  if (current_ == '"') {
    advance(); // skip closing quote
  }

  return Token(TokenType::STRING_LITERAL, value, location);
}

bool Lexer::is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::is_digit(char c) {
  return c >= '0' && c <= '9';
}

bool Lexer::is_alphanumeric(char c) {
  return is_alpha(c) || is_digit(c);
}

bool Lexer::is_identifier_start(char c) {
  return is_alpha(c) || c == '_';
}

bool Lexer::is_identifier(char c) {
  return is_alphanumeric(c) || c == '_';
}

bool Lexer::is_string_start(char c) {
  return c == '"';
}

bool Lexer::is_whitespace(char c) {
  return c == ' ' || 
         c == '\t' || 
         c == '\n' || 
         c == '\r';
}

Location Lexer::current_location() const {
  return {line_, column_ };
}

std::optional<TokenType> Lexer::get_keyword(std::string_view text) {
  return get_keyword_token_type(text);
}

} // namespace tuz
