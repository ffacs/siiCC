#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Token {
  enum class Type : uint32_t {
    Terminator = 0,
    Nonterminator = 1,
    BLANK = 2,
    END = 3
  };
  Token(Type type, const std::string &name) : type_(type), name_(name) {}
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
  Production(TokenPtr head, TokenPtrVec body) : head_(head), body_(body) {}
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
      const auto &body = production->body_;
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
  std::string to_string() const {
    std::stringstream ss;
    ss << production_->head_->to_string() << " -> ";
    for (uint32_t i = 0; i < production_->body_.size(); i++) {
      if (i == matched_) {
        ss << "*";
      }
      ss << production_->body_[i]->to_string();
    }
    if (matched_ == production_->body_.size()) ss << "*";
    ss << "    : ";
    for (const auto &token : end_with_) {
      ss << token->to_string() << " ";
    }
    ss << "\n";
    return ss.str();
  }
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
  to_string(std::map<std::string, ProductionPtrVec> &production_of) const {
    const auto &productions = production_of[head_->name_];
    std::stringstream ss;
    for (const auto &production : productions) {
      ss << head_->to_string() << " -> *";
      for (const auto &token : production->body_) {
        ss << token->to_string();
      }
      ss << "    : ";
      for (const auto &token : end_with_) {
        ss << token->to_string() << " ";
      }
      ss << "\n";
    }
    return ss.str();
  }
};

typedef std::vector<KernelItem> KernelItemVec;
typedef std::vector<NormalItem> NormalItemVec;

struct Closure {
  KernelItemVec kernel_items_;
  NormalItemVec normal_items_;

  bool operator==(const KernelItemVec &other) const {
    if (kernel_items_.size() != other.size())
      return false;
    for (const auto &kernel_item : kernel_items_) {
      if (std::find(other.cbegin(), other.cend(), kernel_item) == other.end()) {
        return false;
      }
    }
    return true;
  }
  bool operator==(const Closure &other) const {
    return *this == other.kernel_items_;
  }
  std::string
  to_string(std::map<std::string, ProductionPtrVec> &production_of) const {
    std::stringstream ss;
    for (const auto &item : kernel_items_) {
      ss << item.to_string();
    }
    for (const auto &item : normal_items_) {
      ss << item.to_string(production_of);
    }
    return ss.str();
  }

  std::optional<size_t> GetNormalItem(TokenPtr token) {
    for (size_t i = 0; i < normal_items_.size(); i++) {
      if (normal_items_[i] == token) {
        return i;
      }
    }
    return std::nullopt;
  }
};

typedef std::shared_ptr<Closure> ClosurePtr;

class ParserGenerator {
public:
  ParserGenerator(const Grammar &grammar) {
    Grammar new_grammer = grammar;
    auto new_start =
        std::make_shared<Token>(Token::Type::Nonterminator, "LALR_START");
    new_grammer.start_ = new_start;
    auto new_production =
        std::make_shared<Production>(new_start, TokenPtrVec{grammar.start_});
    new_grammer.productions_.insert(new_grammer.productions_.begin(),
                                    new_production);
    new_grammer.BuildProductionsOf();
    grammar_ = new_grammer;
  }

  void GenerateLALRTable() {
    KernelItem begin_closure_kernel_item(
        grammar_.productions_of_[grammar_.start_->name_][0], 0);
    begin_closure_kernel_item.end_with_ = {grammar_.end_};

    auto begin_closure_pair = GetClosure({begin_closure_kernel_item});
    auto &begin_closure = begin_closure_pair.second;

    std::queue<ClosurePtr> queue;
    queue.push(begin_closure);
    while (!queue.empty()) {
      auto closure = queue.front();
      queue.pop();
      auto next_token_set = GetNextTokens(closure);
      for (const auto &token : next_token_set) {
        auto next_closure_pair = GetNextClosureFor(token, closure);
        const auto &next_closure = next_closure_pair.second;
        edges_[closure][token] = next_closure;
        std::cout << "------------------------\n";
        std::cout << closure->to_string(grammar_.productions_of_) << " >> "
                  << token->to_string() << " >> \n"
                  << next_closure->to_string(grammar_.productions_of_);
        std::cout << "------------------------\n";
        if (!next_closure_pair.first) {
          queue.push(next_closure);
        }
      }
    }
  }

private:
  TokenPtrSet GetNextTokens(const ClosurePtr &closure) {
    TokenPtrSet result;
    for (const auto &kernel_item : closure->kernel_items_) {
      if (kernel_item.matched_ != kernel_item.production_->body_.size()) {
        result.insert(kernel_item.production_->body_[kernel_item.matched_]);
      }
    }
    for (const auto &normal_item : closure->normal_items_) {
      for (const auto &production :
           grammar_.productions_of_[normal_item.head_->name_]) {
        if (!production->body_.empty()) {
          result.insert(production->body_.front());
        }
      }
    }
    return result;
  }

  std::pair<bool, ClosurePtr> GetNextClosureFor(const TokenPtr &token,
                                                const ClosurePtr &closure) {
    KernelItemVec new_closure_kernel;
    for (const auto &kernel_item : closure->kernel_items_) {
      if (kernel_item.matched_ != kernel_item.production_->body_.size() &&
          kernel_item.production_->body_[kernel_item.matched_] == token) {
        KernelItem new_item(kernel_item.production_, kernel_item.matched_ + 1);
        new_item.end_with_ = kernel_item.end_with_;
        new_closure_kernel.push_back(std::move(new_item));
      }
    }
    for (const auto &normal_item : closure->normal_items_) {
      for (const auto &production :
           grammar_.productions_of_[normal_item.head_->name_]) {
        if (!production->body_.empty() && production->body_.front() == token) {
          KernelItem new_item(production, 1);
          new_item.end_with_ = normal_item.end_with_;
          new_closure_kernel.push_back(std::move(new_item));
        }
      }
    }
    return GetClosure(std::move(new_closure_kernel));
  }

  std::pair<bool, ClosurePtr> GetClosure(KernelItemVec &&kernel_items) {
    for (uint32_t i = 0; i < closures_.size(); i++) {
      const auto &closure = closures_[i];
      if (*closure == kernel_items) {
        return {true, closure};
      }
    }
    auto new_closure = std::make_shared<Closure>();
    new_closure->kernel_items_ = std::move(kernel_items);
    std::queue<TokenPtr> queue;
    for (const auto &kernel_item : new_closure->kernel_items_) {
      if (kernel_item.matched_ != kernel_item.production_->body_.size()) {
        const auto& token = kernel_item.production_->body_[kernel_item.matched_];
        auto normal_item_opt = new_closure->GetNormalItem(token);
        size_t idx;
        if (!normal_item_opt.has_value()) {
          new_closure->normal_items_.push_back(NormalItem{token});
          idx = new_closure->normal_items_.size() - 1;
          queue.push(token);
        }
        auto &end_with = new_closure->normal_items_[idx].end_with_;
        end_with.insert(kernel_item.end_with_.begin(), kernel_item.end_with_.end());
      }
    }
    while (!queue.empty()) {
      auto token = queue.front();
      queue.pop();
      for (const auto &production : grammar_.productions_of_[token->name_]) {
        if (production->body_.empty())
          continue;
        const auto &new_token = production->body_.front();
        if (new_token->type_ != Token::Type::Nonterminator)
          continue;
        auto normal_item_opt = new_closure->GetNormalItem(new_token);
        size_t idx;
        if (!normal_item_opt.has_value()) {
          queue.push(new_token);
          new_closure->normal_items_.push_back(NormalItem{new_token});
          idx = new_closure->normal_items_.size() - 1;
        } else {
          idx = normal_item_opt.value();
        }
        auto &end_with = new_closure->normal_items_[idx].end_with_;
        if (production->body_.size() == 1) {
          auto old_normal_item_idx = new_closure->GetNormalItem(token).value();
          const auto& old_normal_item = new_closure->normal_items_[old_normal_item_idx];
          end_with.insert(old_normal_item.end_with_.begin(), old_normal_item.end_with_.end());
        } else {
          const auto& first = GetFirstOf(production->body_[1]);
          end_with.insert(first.begin(), first.end());
        }
      }
    }
    closures_.emplace_back(new_closure);
    return {false, new_closure};
  }

  const TokenPtrSet &getFollowOf(const TokenPtr &token) {
    auto iter = follow_of_.find(token);
    if (iter != follow_of_.end())
      return iter->second;

    follow_of_[token] = {};
    if (token->type_ != Token::Type::Nonterminator) {
      return follow_of_[token];
    }

    for (const auto &production : grammar_.productions_) {
      if (production->body_.empty())
        continue;
      for (size_t i = 0; i + 1 < production->body_.size(); i++) {
        if (production->body_[i] == token) {
          const auto &other_follow = getFollowOf(production->body_[i + 1]);
          follow_of_[token].insert(other_follow.begin(), other_follow.end());
        }
      }
      if (production->body_.back() == token) {
        const auto &other_follow = getFollowOf(production->head_);
        follow_of_[token].insert(other_follow.begin(), other_follow.end());
      }
    }
    return follow_of_[token];
  }

  const TokenPtrSet &GetFirstOf(const TokenPtr &token) {
    auto iter = first_of_.find(token);
    if (iter != first_of_.end())
      return iter->second;

    first_of_[token] = {};
    if (token->type_ != Token::Type::Nonterminator) {
      first_of_[token].insert(token);
      return first_of_[token];
    }

    for (const auto &production : grammar_.productions_of_[token->name_]) {
      bool blank_ntil_the_end = true;
      for (size_t i = 0; i < production->body_.size(); i++) {
        const auto &other_first = GetFirstOf(production->body_[i]);
        first_of_[token].insert(other_first.begin(), other_first.end());
        if (other_first.find(grammar_.blank_) == other_first.end()) {
          blank_ntil_the_end = false;
          break;
        }
      }
      if (blank_ntil_the_end) {
        const auto &follows = getFollowOf(production->head_);
        for (const auto &follow : follows) {
          const auto &other_first = GetFirstOf(follow);
          first_of_[token].insert(other_first.begin(), other_first.end());
        }
      }
    }

    return first_of_[token];
  }

private:
  std::vector<ClosurePtr> closures_;
  Grammar grammar_;
  std::map<TokenPtr, TokenPtrSet> follow_of_;
  std::map<TokenPtr, TokenPtrSet> first_of_;
  std::map<ClosurePtr, std::map<TokenPtr, ClosurePtr>> edges_;
};

int main() {
  Grammar simple;

  auto b_blank = std::make_shared<Token>(Token::Type::BLANK, "blank");

  auto t_number = std::make_shared<Token>(Token::Type::Terminator, "number");
  auto t_puls = std::make_shared<Token>(Token::Type::Terminator, "+");
  auto t_comma = std::make_shared<Token>(Token::Type::Terminator, ",");

  auto n_primary =
      std::make_shared<Token>(Token::Type::Nonterminator, "primary");
  auto n_expression =
      std::make_shared<Token>(Token::Type::Nonterminator, "expr");
  auto n_start = std::make_shared<Token>(Token::Type::Nonterminator, "start");

  auto e_ed = std::make_shared<Token>(Token::Type::END, "$");

  simple.start_ = n_start;
  simple.end_ = e_ed;

  simple.nonterminators_ = {n_primary, n_expression, n_start};
  simple.terminators_ = {t_comma, t_number, t_puls, e_ed};

  simple.blank_ = b_blank;

  ProductionPtrVec productions = {
      std::make_shared<Production>(
          n_start, TokenPtrVec{n_expression, t_comma,
                               n_start}), // start -> expr,start | expr
      std::make_shared<Production>(n_start, TokenPtrVec{n_expression}),
      std::make_shared<Production>(
          n_expression,
          TokenPtrVec{n_primary, t_puls,
                      n_primary}), // expr -> primary + primary
      std::make_shared<Production>(n_primary, TokenPtrVec{t_number}),
  };

  simple.productions_ = productions;

  ParserGenerator generator(simple);
  generator.GenerateLALRTable();
}