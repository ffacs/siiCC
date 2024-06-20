#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#define DEBUG_MODE
namespace siicc {
struct Token {
  enum class TokenType : int32_t {
    TOKEN_Blank = 1, // Blank
    TOKEN_terminator = 2, // terminator
    TOKEN_nonterminator = 3, // nonterminator
    TOKEN_nonterminator_repreated = 4, // nonterminator_repeated
    TOKEN_nonterminator_optional = 5, // nonterminator_optional
    TOKEN_nonterminator_one_more = 6, // nonterminator_one_more
    TOKEN_or = 7, // |
    TOKEN_equals = 8, // ::=
    TOKEN_semicolon = 9, // ;
    TOKEN_end = 10, // $
  };
  Token(TokenType type, const std::string& value) : type_(type), value_(std::make_shared<std::string>(value)) {}  TokenType type_;
  std::shared_ptr<std::string> value_;
};

class Lexer { public: virtual Token Next() = 0; };

struct ASTNode {
  enum class Type : int {
    LEAF_Blank = 1,
    LEAF_terminator = 2,
    LEAF_nonterminator = 3,
    LEAF_nonterminator_repreated = 4,
    LEAF_nonterminator_optional = 5,
    LEAF_nonterminator_one_more = 6,
    LEAF_or = 7,
    LEAF_equals = 8,
    LEAF_semicolon = 9,
    LEAF_end = 10,
    NODE_Productions = 11,
    NODE_Production = 12,
    NODE_Production_Bodies = 13,
    NODE_Production_Body = 14,
    NODE_Production_Items = 15,
    NODE_Production_Item = 16,
    NODE_START = 17,
  };
  Type type_;
  std::shared_ptr<std::string> value_;
  std::vector<std::shared_ptr<ASTNode>> children_;
  static bool IsLeaf(Type type) { return static_cast<int>(type) <= 10; }
  static std::string TypeToStr(Type type);
};
typedef std::shared_ptr<ASTNode> ASTNodePtr;
ASTNodePtr Parse(std::shared_ptr<Lexer> lexer);
} // namespace sii
