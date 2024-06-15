#include "siicc_header.h"
uint32_t reduce_result[7] = {
  0,  8,  5,  5,  6,  6,  7 
};
uint32_t reduce_length[7] = {
  0,  1,  3,  1,  2,  1,  1 
};
int32_t action_table[10][8] = {
    0,    4,    5,    0,    0,    1,    2,    3,  
    0,    0,    0,    0,   -1,    0,    0,    0,  
    0,    0,    0,    6,   -6,    0,    0,    0,  
    0,    0,    0,    0,   -3,    0,    0,    0,  
    0,    4,    5,    0,    0,    0,    8,    7,  
    0,    0,    0,   -5,   -5,    0,    0,    0,  
    0,    4,    5,    0,    0,    0,    8,    9,  
    0,    0,    0,   -4,   -4,    0,    0,    0,  
    0,    0,    0,   -6,   -6,    0,    0,    0,  
    0,    0,    0,    0,   -2,    0,    0,    0,  
};
static bool ShouldShift(uint32_t next_id) {
  return next_id < 5;
};
const char* DEBUG_INFO_TABLE[8] = {
  "*",
  "id",
  "=",
  "$",
  "S",
  "L",
  "R",
  "GRAMMAR_START",
};
static constexpr uint32_t ACCPET_TOKEN = 8;
ASTNodePtr CreateASTNode(uint32_t token_id, const std::string& value = "") {
  auto result = std::make_shared<ASTNode>();
  result->value_ = std::make_shared<std::string>(value);
  switch (token_id) {
    case 1:
      result->type_ = ASTNode::Type::LEAF;
      break;
    case 2:
      result->type_ = ASTNode::Type::LEAF;
      break;
    case 3:
      result->type_ = ASTNode::Type::LEAF;
      break;
    case 4:
      result->type_ = ASTNode::Type::LEAF;
      break;
    case 5:
      result->type_ = ASTNode::Type::NODE_S;
      break;
    case 6:
      result->type_ = ASTNode::Type::NODE_L;
      break;
    case 7:
      result->type_ = ASTNode::Type::NODE_R;
      break;
    case 8:
      result->type_ = ASTNode::Type::NODE_GRAMMAR_START;
      break;
  }  return result;
}
#include <stack>
#include <stdexcept>
#include <iostream>
ASTNodePtr Parse(std::shared_ptr<Lexer> lexer) {
	std::stack<int32_t> state_stack, next_token_id;
	std::vector<std::shared_ptr<ASTNode>> ast_stack;
	int32_t current_state = 0;
	state_stack.push(current_state);
	while (true) {
		if (next_token_id.empty()) {
			Token next_token = lexer->Next();
			next_token_id.push(static_cast<int32_t>(next_token.type_));
		}
		int32_t next_id = next_token_id.top(); next_token_id.pop();
		int32_t action = action_table[current_state][next_id];
		if (action == 0) {
			throw std::invalid_argument(std::string(DEBUG_INFO_TABLE[next_id - 1]) + " not accpeted");
		} else if (action > 0) {
			if (ShouldShift(next_id)) {
				auto new_AST_Node = CreateASTNode(next_id, DEBUG_INFO_TABLE[next_id - 1]);
				ast_stack.push_back(new_AST_Node);
			}
			state_stack.push(action);
			current_state = action;
		} else {
			action *= -1;
			next_token_id.push(next_id);
			int32_t reduce_count = reduce_length[action];
			auto new_token = reduce_result[action];
			auto new_AST_Node = CreateASTNode(new_token);
			for (size_t i = 0; i < reduce_count; i++) {
				new_AST_Node->children_.push_back(ast_stack.at(ast_stack.size() + i - reduce_count));
			}
			if (new_token == ACCPET_TOKEN) {
				return new_AST_Node;
			}
			for (int i = 0; i < reduce_count; i++) {
				state_stack.pop();
				ast_stack.pop_back();
			}
			next_token_id.push(new_token);
			ast_stack.push_back(new_AST_Node);
			current_state = state_stack.top();
		}
	}
}
