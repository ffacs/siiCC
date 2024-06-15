#pragma once
#include <string>
#include <memory>
#include <cstdint>
struct Token {
  enum class TokenType : int32_t {
    TOKEN_star = 1, // *
    TOKEN_id = 2, // id
    TOKEN_equal = 3, // =
    TOKEN_end = 4, // $
  };
  Token(TokenType type, const std::string& value) : type_(type), value_(std::make_shared<std::string>(value)) {}  TokenType type_;
  std::shared_ptr<std::string> value_;
};

class Lexer { public: virtual Token Next() = 0; };

#include <vector>
struct ASTNode {
  enum class Type : int {
    NODE_S = 1,
    NODE_L = 2,
    NODE_R = 3,
    NODE_GRAMMAR_START = 4,
    LEAF = 5,
  };
  Type type_;
  std::shared_ptr<std::string> value_;
  std::vector<std::shared_ptr<ASTNode>> children_;
};
typedef std::shared_ptr<ASTNode> ASTNodePtr;
ASTNodePtr Parse(std::shared_ptr<Lexer> lexer);
