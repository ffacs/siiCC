#pragma once

#include "siicc_EBNF.h"
#include <fstream>

namespace siicc {

class BNFLexer : public Lexer {
public:
  BNFLexer(std::ifstream &is) : is_(is) {}
  virtual Token Next() override {
    std::string next;
    is_ >> next;
    if (is_.eof()) {
      return Token(Token::TokenType::TOKEN_end, "$");
    } else if (next.length() == 1 && next.front() == '|') {
      return Token(Token::TokenType::TOKEN_or, "|");
    } else if(next.length() == 1 && next.front() == ';') {
      return Token(Token::TokenType::TOKEN_semicolon, ";");
    } else if (next.front() == '{') {
      if (next.length() < 5) {
        throw std::invalid_argument(std::string("Invalid Token") + next);
      }
      auto last = next.back();
      switch (last) {
        case '+' :
          next.erase(next.begin());
          next.pop_back();next.pop_back();
          return Token(Token::TokenType::TOKEN_nonterminator_one_more, next);
        case '*':
          next.erase(next.begin());
          next.pop_back();next.pop_back();
          return Token(Token::TokenType::TOKEN_nonterminator_repreated, next);
        case '?': 
          next.erase(next.begin());
          next.pop_back();next.pop_back();
          return Token(Token::TokenType::TOKEN_nonterminator_optional, next);
        default:
          throw std::invalid_argument(std::string("Invalid Token") + next);
      }
    } else if (next.front() == '<') {
      return Token(Token::TokenType::TOKEN_nonterminator, next);
    } else if (next == "::=") {
      return Token(Token::TokenType::TOKEN_equals, "::=");
    } else {
      return Token(Token::TokenType::TOKEN_terminator, next);
    } 
  }

private:
  std::ifstream& is_;
};
}