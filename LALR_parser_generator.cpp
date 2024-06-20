#include "LALR_parser_generator.h"
#include <iomanip>

namespace siicc {
namespace LALR {
class PrintHelper {
public:
  PrintHelper(uint8_t default_indent = 2)
      : indent_(0), default_indent_(default_indent) {}

  void Indent() { indent_ += default_indent_; }
  void Indent(uint32_t count) { indent_ += count; }
  void Deindent(uint32_t count) { indent_ -= count; }
  void Deindent() { indent_ -= default_indent_; }
  std::ostream &operator<<(std::ostream &os) {
    PrintIndent(os);
    return os;
  }

private:
  void PrintIndent(std::ostream &os) {
    for (int i = 0; i < indent_; i++) {
      os << " ";
    }
  }

  uint8_t indent_;
  uint8_t default_indent_ = 2;
};

LALRParserGenerator::LALRParserGenerator(ActionType &&action,
                                         ReduceType &&reduce,
                                         std::vector<ClosurePtr> &&closures,
                                         GrammarPtr grammar)
    : action_(action), reduce_(reduce), closures_(closures), grammar_(grammar) {
  uint32_t idx = 0;
  for (auto &token : grammar_->terminators_) {
    token->id_ = ++idx;
  }
  for (auto &token : grammar_->nonterminators_) {
    token->id_ = ++idx;
  }
  idx = 0;
  for (auto &closure : closures) {
    closure->id_ = idx++;
  }
  idx = 0;
  for (auto &production : grammar_->productions_) {
    production->id_ = ++idx;
  }
}

static inline void OutputTokenDef(PrintHelper &ph, std::ostream &os,
                                  const TokenPtrSet &terminators) {

  ph << os << "struct Token {\n";
  ph.Indent();
  ph << os << "enum class TokenType : int32_t {\n";
  ph.Indent();
  for (const auto &terminator : terminators) {
    ph << os << "TOKEN_" << terminator->name_ << " = " << terminator->id_
       << ", // " << terminator->debug_name_ << "\n";
  }
  ph.Deindent();
  ph << os << "};\n";
  ph << os
     << "Token(TokenType type, const std::string& value) : type_(type), "
        "value_(std::make_shared<std::string>(value)) {}";
  ph << os << "TokenType type_;\n";
  ph << os << "std::shared_ptr<std::string> value_;\n";
  ph.Deindent();
  ph << os << "};\n\n";

  ph << os << "class Lexer { public: virtual Token Next() = 0; };\n\n";
}

static inline void OutputNodeDef(PrintHelper &ph, std::ostream &os,
                                 const GrammarPtr &grammar) {
  ph << os << "struct ASTNode {\n";
  ph.Indent();
  ph << os << "enum class Type : int {\n";
  ph.Indent();
  for (const auto& terminator : grammar->terminators_) {
    ph << os << "LEAF_" << terminator->name_ << " = " << terminator->id_
       << ",\n";
  }
  for (const auto &nonterminator : grammar->nonterminators_) {
    ph << os << "NODE_" << nonterminator->name_ << " = " << nonterminator->id_
       << ",\n";
  }
  ph.Deindent();
  ph << os << "};\n";
  ph << os << "Type type_;\n";
  ph << os << "std::shared_ptr<std::string> value_;\n";
  ph << os << "std::vector<std::shared_ptr<ASTNode>> children_;\n";
  ph << os
     << "static bool IsLeaf(Type type) { return static_cast<int>(type) <= "
     << grammar->terminators_.size() << "; }\n";
  ph << os << "static std::string TypeToStr(Type type);\n";
  ph.Deindent();
  ph << os << "};\n";
  ph << os << "typedef std::shared_ptr<ASTNode> ASTNodePtr;\n";
  ph << os << "ASTNodePtr Parse(std::shared_ptr<Lexer> lexer);\n";
}

void LALRParserGenerator::OutputHeader(std::ostream &os) {
  os << "#pragma once\n";
  os << "#include <string>\n";
  os << "#include <memory>\n";
  os << "#include <cstdint>\n";
  os << "#include <vector>\n";
  os << "namespace siicc {\n";
  PrintHelper ph;
  OutputTokenDef(ph, os, grammar_->terminators_);
  OutputNodeDef(ph, os, grammar_);
  os << "} // namespace sii\n";
}

static void OutputAcceptDefine(PrintHelper &ph, std::ostream &os,
                               uint32_t accpeted) {
  ph << os << "static constexpr uint32_t ACCPET_TOKEN = " << accpeted << ";\n";
}

static std::string GetPrintStr(const std::string &str) {
  std::string result;
  for (auto ch : str) {
    if (ch == '\'') {
      result.append("\'");
    } else if (ch == '\\') {
      result.append("\\");
    } else if (ch == '\"') {
      result.append("\"");
    } else {
      result.push_back(ch);
    }
  }
  return result;
}

static void OutputDebugInfo(PrintHelper &ph, std::ostream &os,
                            const GrammarPtr &grammar) {
  ph << os << "const char* DEBUG_INFO_TABLE["
     << grammar->nonterminators_.size() + grammar->terminators_.size()
     << "] = {\n";
  ph.Indent();
  for (const auto &terminator : grammar->terminators_) {
    ph << os << "\"" << GetPrintStr(terminator->debug_name_) << "\",\n";
  }
  for (const auto &nonterminator : grammar->nonterminators_) {
    ph << os << "\"" << GetPrintStr(nonterminator->debug_name_) << "\",\n";
  }
  ph.Deindent();
  ph << os << "};\n";

  ph << os << "std::string ASTNode::TypeToStr(Type type) {\n";
  ph.Indent();
  ph << os << "switch (type) {\n";
  ph.Indent();
  uint32_t idx = 1;
  for (const auto& terminator : grammar->terminators_) {
    ph << os << "case ASTNode::Type::LEAF_" << terminator->name_ << ":\n";
    ph.Indent();
    ph << os << "return \"" << terminator->name_ << "\";\n";
    ph.Deindent();
  }
  for (const auto &nonterminator : grammar->nonterminators_) {
    ph << os << "case ASTNode::Type::NODE_" << nonterminator->name_  << ":\n";
    ph.Indent();
    ph << os << "return \"" << nonterminator->name_ << "\";\n";
    ph.Deindent();
  }
  ph << os << "default: throw std::invalid_argument(\"Invalid argument.\");\n";
  ph.Deindent();
  ph << os << "}\n";
  ph.Deindent();
  ph << os << "}\n";
}

static void OutputTables(PrintHelper &ph, std::ostream &os,
                         const std::vector<uint32_t> reduce_result,
                         const std::vector<uint32_t> &reduce_length,
                         const std::vector<std::vector<int32_t>> &action_table,
                         const GrammarPtr &grammar) {
  ph << os << "uint32_t reduce_result[" << reduce_result.size() << "] = {\n";
  ph.Indent();
  for (size_t i = 0; i < reduce_result.size(); i++) {
    ph << os << reduce_result[i] << ", "[i + 1 == reduce_result.size()];
  }
  ph.Deindent();
  ph << os << "\n};\n";
  ph << os << "uint32_t reduce_length[" << reduce_length.size() << "] = {\n";
  ph.Indent();
  for (size_t i = 0; i < reduce_length.size(); i++) {
    ph << os << reduce_length[i] << ", "[i + 1 == reduce_length.size()];
  }
  ph.Deindent();
  ph << os << "\n};\n";
  ph << os << "int32_t action_table[" << action_table.size() << "]["
     << action_table[0].size() << "] = {\n";
  ph.Indent();
  for (size_t i = 0; i < action_table.size(); i++) {
    for (size_t j = 0; j < action_table[i].size(); j++) {
      ph << os << std::setw(3) << action_table[i][j] << ",";
    }
    ph << os << "\n";
  }
  ph.Deindent();
  ph << os << "};\n";

  ph << os << "static bool ShouldShift(uint32_t next_id) {\n";
  ph.Indent();
  ph << os << "return next_id < " << grammar->terminators_.size() + 1 << ";\n";
  ph.Deindent();
  ph << os << "};\n";
}

static void OutputGenerateNode(PrintHelper &ph, std::ostream &os,
                               const GrammarPtr &grammar) {
  ph << os
     << R"""(ASTNodePtr CreateASTNode(uint32_t token_id, const std::string& value = "") {)"""
     << "\n";
  ph.Indent();
  ph << os << "auto result = std::make_shared<ASTNode>();\n";
  ph << os << "result->value_ = std::make_shared<std::string>(value);\n";
  ph << os << "switch (token_id) {\n";
  ph.Indent();
  uint32_t idx = 1;
  for (const auto& terminator : grammar->terminators_) {
    ph << os << "case " << (idx++) << ":\n";
    ph.Indent();
    ph << os << "result->type_ = ASTNode::Type::LEAF_" << terminator->name_ << ";\n";
    ph << os << "break;\n";
    ph.Deindent();
  }
  for (const auto &nonterminator : grammar->nonterminators_) {
    ph << os << "case " << (idx++) << ":\n";
    ph.Indent();
    ph << os << "result->type_ = ASTNode::Type::NODE_" << nonterminator->name_
       << ";\n";
    ph << os << "break;\n";
    ph.Deindent();
  }
  ph.Deindent();
  ph << os << "}\n";
  ph << os << "return result;\n";
  ph.Deindent();
  ph << os << "}\n";

}

static void OutputAlgo(PrintHelper &ph, std::ostream &os) {

  std::string algo = R"""(
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
)""";

  ph << os << algo;
}

void LALRParserGenerator::OutputCpp(const std::string &header_name,
                                    std::ostream &os) {
  PrintHelper ph;
  ph << os << "#include \"" << header_name << "\"\n";
  ph << os << "#include <iostream>\n";
  ph << os << "#include <stack>\n";
  ph << os << "#include <stdexcept>\n";
  ph << os << "namespace siicc {\n";

  std::vector<uint32_t> reduce_result(grammar_->productions_.size() + 1);
  std::vector<uint32_t> reduce_length(grammar_->productions_.size() + 1);
  std::vector<std::vector<int32_t>> action_table;
  for (const auto &production : grammar_->productions_) {
    reduce_result[production->id_] = production->head_->id_;
    reduce_length[production->id_] = production->body_.size();
  }
  for (const auto &closure : closures_) {
    action_table.emplace_back(
        grammar_->terminators_.size() + grammar_->nonterminators_.size(), 0);
    const auto &action = action_[closure];
    const auto &reduce = reduce_[closure];
    for (const auto &terminator : grammar_->terminators_) {
      auto action_iter = action.find(terminator);
      auto reduce_iter = reduce.find(terminator);
      if (action_iter != action.end()) {
        action_table[closure->id_][terminator->id_] = action_iter->second->id_;
      } else if (reduce_iter != reduce.end()) {
        action_table[closure->id_][terminator->id_] =
            -1 * reduce_iter->second->id_;
      }
    }
    for (const auto &nonterminator : grammar_->nonterminators_) {
      auto action_iter = action.find(nonterminator);
      if (action_iter != action.end()) {
        action_table[closure->id_][nonterminator->id_] =
            action_iter->second->id_;
      }
    }
  }
  OutputTables(ph, os, reduce_result, reduce_length, action_table, grammar_);
  OutputDebugInfo(ph, os, grammar_);
  OutputAcceptDefine(ph, os, grammar_->start_->id_);
  OutputGenerateNode(ph, os, grammar_);
  OutputAlgo(ph, os);

  ph << os << "} // namespace siicc \n";
}
} // namespace LALR
} // namespace siicc