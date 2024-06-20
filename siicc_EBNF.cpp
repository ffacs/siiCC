#include "siicc_EBNF.h"
#include <iostream>
#include <stack>
#include <stdexcept>
namespace siicc {
uint32_t reduce_result[15] = {
  0,  17,  11,  11,  12,  13,  13,  14,  15,  15,  16,  16,  16,  16,  16 
};
uint32_t reduce_length[15] = {
  0,  1,  1,  2,  4,  1,  3,  1,  1,  2,  1,  1,  1,  1,  1 
};
int32_t action_table[19][17] = {
    0,    0,    0,    3,    0,    0,    0,    0,    0,    0,    0,    1,    2,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   -1,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,    3,    0,    0,    0,    0,    0,    0,   -2,    4,    2,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,    0,    5,    0,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   -3,    0,    0,    0,    0,    0,    0,  
    0,    0,   10,   11,   13,   14,   12,    0,    0,    0,    0,    0,    0,    6,    7,    8,    9,  
    0,    0,    0,    0,    0,    0,    0,    0,    0,   15,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,   16,    0,   -5,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,   -7,    0,   -7,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,   10,   11,   13,   14,   12,   -8,    0,   -8,    0,    0,    0,    0,    0,   17,    9,  
    0,    0,  -10,  -10,  -10,  -10,  -10,  -10,    0,  -10,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,  -11,  -11,  -11,  -11,  -11,  -11,    0,  -11,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,  -12,  -12,  -12,  -12,  -12,  -12,    0,  -12,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,  -13,  -13,  -13,  -13,  -13,  -13,    0,  -13,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,  -14,  -14,  -14,  -14,  -14,  -14,    0,  -14,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,   -4,    0,    0,    0,    0,    0,    0,   -4,    0,    0,    0,    0,    0,    0,  
    0,    0,   10,   11,   13,   14,   12,    0,    0,    0,    0,    0,    0,   18,    7,    8,    9,  
    0,    0,    0,    0,    0,    0,    0,   -9,    0,   -9,    0,    0,    0,    0,    0,    0,    0,  
    0,    0,    0,    0,    0,    0,    0,    0,    0,   -6,    0,    0,    0,    0,    0,    0,    0,  
};
static bool ShouldShift(uint32_t next_id) {
  return next_id < 11;
};
const char* DEBUG_INFO_TABLE[17] = {
  "Blank",
  "terminator",
  "nonterminator",
  "nonterminator_repeated",
  "nonterminator_optional",
  "nonterminator_one_more",
  "|",
  "::=",
  ";",
  "$",
  "Productions",
  "Production",
  "Production_Bodies",
  "Production_Body",
  "Production_Items",
  "Production_Item",
  "START",
};
std::string ASTNode::TypeToStr(Type type) {
  switch (type) {
    case ASTNode::Type::LEAF_Blank:
      return "Blank";
    case ASTNode::Type::LEAF_terminator:
      return "terminator";
    case ASTNode::Type::LEAF_nonterminator:
      return "nonterminator";
    case ASTNode::Type::LEAF_nonterminator_repreated:
      return "nonterminator_repreated";
    case ASTNode::Type::LEAF_nonterminator_optional:
      return "nonterminator_optional";
    case ASTNode::Type::LEAF_nonterminator_one_more:
      return "nonterminator_one_more";
    case ASTNode::Type::LEAF_or:
      return "or";
    case ASTNode::Type::LEAF_equals:
      return "equals";
    case ASTNode::Type::LEAF_semicolon:
      return "semicolon";
    case ASTNode::Type::LEAF_end:
      return "end";
    case ASTNode::Type::NODE_Productions:
      return "Productions";
    case ASTNode::Type::NODE_Production:
      return "Production";
    case ASTNode::Type::NODE_Production_Bodies:
      return "Production_Bodies";
    case ASTNode::Type::NODE_Production_Body:
      return "Production_Body";
    case ASTNode::Type::NODE_Production_Items:
      return "Production_Items";
    case ASTNode::Type::NODE_Production_Item:
      return "Production_Item";
    case ASTNode::Type::NODE_START:
      return "START";
    default: throw std::invalid_argument("Invalid argument.");
  }
}
static constexpr uint32_t ACCPET_TOKEN = 17;
ASTNodePtr CreateASTNode(uint32_t token_id, const std::string& value = "") {
  auto result = std::make_shared<ASTNode>();
  result->value_ = std::make_shared<std::string>(value);
  switch (token_id) {
    case 1:
      result->type_ = ASTNode::Type::LEAF_Blank;
      break;
    case 2:
      result->type_ = ASTNode::Type::LEAF_terminator;
      break;
    case 3:
      result->type_ = ASTNode::Type::LEAF_nonterminator;
      break;
    case 4:
      result->type_ = ASTNode::Type::LEAF_nonterminator_repreated;
      break;
    case 5:
      result->type_ = ASTNode::Type::LEAF_nonterminator_optional;
      break;
    case 6:
      result->type_ = ASTNode::Type::LEAF_nonterminator_one_more;
      break;
    case 7:
      result->type_ = ASTNode::Type::LEAF_or;
      break;
    case 8:
      result->type_ = ASTNode::Type::LEAF_equals;
      break;
    case 9:
      result->type_ = ASTNode::Type::LEAF_semicolon;
      break;
    case 10:
      result->type_ = ASTNode::Type::LEAF_end;
      break;
    case 11:
      result->type_ = ASTNode::Type::NODE_Productions;
      break;
    case 12:
      result->type_ = ASTNode::Type::NODE_Production;
      break;
    case 13:
      result->type_ = ASTNode::Type::NODE_Production_Bodies;
      break;
    case 14:
      result->type_ = ASTNode::Type::NODE_Production_Body;
      break;
    case 15:
      result->type_ = ASTNode::Type::NODE_Production_Items;
      break;
    case 16:
      result->type_ = ASTNode::Type::NODE_Production_Item;
      break;
    case 17:
      result->type_ = ASTNode::Type::NODE_START;
      break;
  }
  return result;
}

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
#ifdef DEBUG_MODE
    for (int i = 0; i < ast_stack.size(); i++) {
      auto& item = ast_stack[i];
      if (ASTNode::IsLeaf(item->type_)) {
        std::cerr << *ast_stack[i]->value_ << " ";
      } else {
        std::cerr << DEBUG_INFO_TABLE[static_cast<int>(ast_stack[i]->type_) - 1] << " ";
      }
    } std::cerr << "\n";
#endif
	}
}
} // namespace siicc 
