#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <set>
#include <vector>
#include <sstream>
#include <map>
#include <optional>

struct Token {
  enum class Type : uint32_t {
    Terminator = 0,
    Nonterminator = 1,
    BLANK = 2,
    END = 3
  };
  uint32_t id_;
  Token(Type type, const std::string &name) : type_(type), name_(name) {
    static uint32_t id = 0;
    id_ = id++; 
  }
  Type type_;
  std::string name_;
  std::string to_string() const { return name_; }
};

typedef std::shared_ptr<Token> TokenPtr;
typedef std::set<TokenPtr> TokenPtrSet;
typedef std::vector<TokenPtr> TokenPtrVec;

struct Production {
  TokenPtr head_;
  TokenPtrVec body_;
  uint32_t id_;
  Production(TokenPtr head, TokenPtrVec body) : head_(head), body_(body) {
    static uint32_t id = 0;
    id_ = id++;
  }
  std::string to_string() const {
    std::stringstream ss;
    ss << head_->to_string() << " -> ";
    for (const auto &token : body_) {
      ss << token->to_string();
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
      for (const auto& token : production->body_) {
        if (token->type_ == Token::Type::BLANK && production->body_.size() != 1) {
          std::vector<TokenPtr> new_production_body;
          for (const auto& token : production->body_) {
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

  Closure() {
    static uint32_t id = 0;
    id_ = id++;
  }

  bool operator==(const KernelItemVec &other) const;
  bool operator==(const Closure &other) const;
  std::string
  to_string(std::map<std::string, ProductionPtrVec> &production_of) const;

  std::optional<size_t> GetNormalItem(TokenPtr token);
};
typedef std::shared_ptr<Closure> ClosurePtr;