#include "siicc_header.h"
#include <queue>
#include <iostream>

class dummpyLexer : public Lexer{
public:
	dummpyLexer(std::vector<Token> tokens) : tokens_(tokens) {}
  virtual Token Next() override {
	Token result = std::move(tokens_.front());
	tokens_.erase(tokens_.begin());
	return result;
  }

private:
  std::vector<Token> tokens_;
};

void visit(ASTNodePtr root) {
	std::queue<ASTNodePtr> queue;
	std::queue<std::string> result;
	queue.push(root);
	result.push("BEGIN");
	result.push("|");
	result.push("\n");
  uint32_t current_left = 1;
  uint32_t next_has = 0;
	while (!queue.empty()) {
		auto curr = queue.front();
		queue.pop();
    current_left--;
		result.push("(");
		for (auto child : curr->children_) {
      if (child->type_ == ASTNode::Type::LEAF) {
        result.push(*child->value_);
      } else {
			  result.push(std::to_string(static_cast<uint32_t>(child->type_)));
      }
			queue.push(child);
      next_has++;
		}
		result.push(")");
    if (current_left == 0) {
      result.push("\n");
      current_left = next_has;
      next_has = 0;
    }
	}
	while(!result.empty()) {
		auto curr = result.front();
		result.pop();
		std::cout << curr;
	}
}

int main() {
	Token star(Token::TokenType::TOKEN_star, "*"); 
	Token equal(Token::TokenType::TOKEN_equal, "="); 
	Token left(Token::TokenType::TOKEN_id, "left"); 
	Token right(Token::TokenType::TOKEN_id, "right"); 
	Token end(Token::TokenType::TOKEN_end, "end"); 
	
	std::vector<Token> queue{star, left, equal, star, right, end};
	auto lexer = std::make_shared<dummpyLexer>(queue);
	auto node = Parse(lexer);
	visit(node);
}