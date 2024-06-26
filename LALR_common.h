#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace siicc {
namespace LALR {
struct Token {
  enum class Type : uint32_t {
    Terminator = 0,
    Nonterminator = 1,
    BLANK = 2,
  };
  Token(Type type, const std::string &name, const std::string &debug_name)
      : type_(type), name_(name), debug_name_(debug_name) {}
  Type type_;
  std::string name_;
  std::string debug_name_;
  uint32_t id_;
  std::string to_string() const { return debug_name_; }
};

typedef std::shared_ptr<Token> TokenPtr;
typedef std::set<TokenPtr> TokenPtrSet;
typedef std::vector<TokenPtr> TokenPtrVec;

static inline TokenPtr NewTerminator(const std::string &name,
                                     const std::string &debug_name) {
  return std::make_shared<Token>(Token::Type::Terminator, name, debug_name);
}

static inline TokenPtr NewBlank() {
  return std::make_shared<Token>(Token::Type::BLANK, "Blank", "Blank");
}

static inline TokenPtr NewEnd() {
  return std::make_shared<Token>(Token::Type::BLANK, "End", "End");
}

static inline TokenPtr NewNonTerminator(const std::string &name) {
  return std::make_shared<Token>(Token::Type::Nonterminator, name, name);
}

struct Production {
  TokenPtr head_;
  TokenPtrVec body_;
  uint32_t id_;
  Production(TokenPtr head, TokenPtrVec body) : head_(head), body_(body) {}
  std::string to_string() const {
    std::stringstream ss;
    ss << head_->to_string() << " -> ";
    for (const auto &token : body_) {
      ss << token->to_string() << " ";
    }
    return ss.str();
  }
};

typedef std::shared_ptr<Production> ProductionPtr;
typedef std::vector<ProductionPtr> ProductionPtrVec;

struct Grammar {
  TokenPtrSet nonterminators_;
  TokenPtrSet terminators_;
  ProductionPtrVec productions_;
  std::map<std::string, ProductionPtrVec> productions_of_;
  TokenPtr start_;
  TokenPtr end_;
  TokenPtr blank_;
  void BuildProductionsOf() {
    for (const auto &production : productions_) {
      if (production->body_.empty()) {
        std::stringstream ss;
        ss << "Production with head:" << production->head_ << " has empty body";
        throw std::invalid_argument(ss.str());
      }
      for (const auto &token : production->body_) {
        if (token->type_ == Token::Type::BLANK &&
            production->body_.size() != 1) {
          std::vector<TokenPtr> new_production_body;
          for (const auto &token : production->body_) {
            if (token->type_ == Token::Type::BLANK) {
              continue;
            }
            new_production_body.push_back(token);
          }
          production->body_ = new_production_body;
        }
      }
      const auto &body = production->body_;
      if (production->head_->type_ != Token::Type::Nonterminator) {
        throw std::invalid_argument("Production head is not nonterminator");
      }
      productions_of_[production->head_->name_].push_back(production);
    }
  }
  std::string to_string() const {
    std::stringstream ss;
    for (const auto &production : productions_) {
      ss << production->to_string() << "\n";
    }
    return ss.str();
  }
};

typedef std::shared_ptr<Grammar> GrammarPtr;

struct KernelItem {
  ProductionPtr production_;
  uint32_t matched_;
  TokenPtrSet end_with_;
  KernelItem(ProductionPtr production, uint32_t matched)
      : production_(production), matched_(matched) {}
  bool operator==(const KernelItem &other) const {
    return production_ == other.production_ && matched_ == other.matched_;
  }
  bool operator<(const KernelItem &other) const {
    return production_ == other.production_ ? matched_ < other.matched_
                                            : production_ < other.production_;
  }
  std::string to_string() const;
};

struct NormalItem {
  TokenPtr head_;
  TokenPtrSet end_with_;
  NormalItem(TokenPtr head) : head_(head) {}

  bool operator==(const NormalItem &other) const {
    return head_ == other.head_;
  }
  bool operator<(const NormalItem &other) const { return head_ < other.head_; }
  std::string
  to_string(std::map<std::string, ProductionPtrVec> &production_of) const;
};

typedef std::vector<KernelItem> KernelItemVec;
typedef std::vector<NormalItem> NormalItemVec;

struct Closure {
  KernelItemVec kernel_items_;
  NormalItemVec normal_items_;
  uint32_t id_;

  bool operator==(const KernelItemVec &other) const;
  bool operator==(const Closure &other) const;
  std::string
  to_string(std::map<std::string, ProductionPtrVec> &production_of) const;

  std::optional<size_t> GetNormalItem(TokenPtr token);
};
typedef std::shared_ptr<Closure> ClosurePtr;
} // namespace LALR
} // namespace siicc