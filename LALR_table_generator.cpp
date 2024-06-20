#include "LALR_table_generator.h"

namespace siicc {
namespace LALR {

std::string KernelItem::to_string() const {
  std::stringstream ss;
  ss << production_->head_->to_string() << " -> ";
  for (uint32_t i = 0; i < production_->body_.size(); i++) {
    if (i == matched_) {
      ss << "*";
    }
    ss << production_->body_[i]->to_string() << " ";
  }
  if (matched_ == production_->body_.size())
    ss << "*";
  ss << "    : ";
  for (const auto &token : end_with_) {
    ss << token->to_string() << " ";
  }
  ss << "\n";
  return ss.str();
}

std::string NormalItem::to_string(
    std::map<std::string, ProductionPtrVec> &production_of) const {
  const auto &productions = production_of[head_->name_];
  std::stringstream ss;
  for (const auto &production : productions) {
    ss << head_->to_string() << " -> *";
    for (const auto &token : production->body_) {
      ss << token->to_string() << " ";
    }
    ss << "    : ";
    for (const auto &token : end_with_) {
      ss << token->to_string() << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

bool Closure::operator==(const KernelItemVec &other) const {
  if (kernel_items_.size() != other.size())
    return false;
  for (const auto &kernel_item : kernel_items_) {
    if (std::find(other.cbegin(), other.cend(), kernel_item) == other.end()) {
      return false;
    }
  }
  return true;
}

bool Closure::operator==(const Closure &other) const {
  return *this == other.kernel_items_;
}

std::string Closure::to_string(
    std::map<std::string, ProductionPtrVec> &production_of) const {
  std::stringstream ss;
  for (const auto &item : kernel_items_) {
    ss << item.to_string();
  }
  for (const auto &item : normal_items_) {
    ss << item.to_string(production_of);
  }
  return ss.str();
}

std::optional<size_t> Closure::GetNormalItem(TokenPtr token) {
  for (size_t i = 0; i < normal_items_.size(); i++) {
    if (normal_items_[i] == token) {
      return i;
    }
  }
  return std::nullopt;
}

LALRTableGenerator::LALRTableGenerator(const Grammar &grammar) {
  if (grammar.blank_ == nullptr) {
    throw std::invalid_argument("blank not specified.");
  }
  if (grammar.end_ == nullptr) {
    throw std::invalid_argument("end not specified.");
  }
  if (grammar.start_ == nullptr) {
    throw std::invalid_argument("start not specified.");
  }
  
  GrammarPtr new_grammer = std::make_shared<Grammar>();
  *new_grammer = grammar;
  auto new_start = NewNonTerminator("START");


  new_grammer->start_ = new_start;
  new_grammer->nonterminators_.insert(new_start);
  auto new_production =
      std::make_shared<Production>(new_start, TokenPtrVec{grammar.start_});
  new_grammer->productions_.insert(new_grammer->productions_.begin(),
                                   new_production);
  new_grammer->BuildProductionsOf();
  grammar_ = new_grammer;

  uint32_t idx = 0;
  for (auto &production : grammar_->productions_) {
    production->id_ = ++idx;
  }
  idx = 0;
  for (auto &terminator : grammar_->terminators_) {
    terminator->id_ = ++idx;
  }
  idx = 0;
  for (auto &nonterminator : grammar_->nonterminators_) {
    nonterminator->id_ = ++idx;
  }
}

void LALRTableGenerator::GenerateLALRTable() {
  KernelItem begin_closure_kernel_item(
      grammar_->productions_of_[grammar_->start_->name_][0], 0);
  begin_closure_kernel_item.end_with_ = {grammar_->end_};

  auto begin_closure_pair = GetClosure({begin_closure_kernel_item});
  auto &begin_closure = begin_closure_pair.second;

  std::queue<ClosurePtr> queue;
  queue.push(begin_closure);
  while (!queue.empty()) {
    auto closure = queue.front();
    queue.pop();
    auto next_token_set = GetNextTokens(closure);
    for (const auto &token : next_token_set) {
      if (token->type_ == Token::Type::BLANK)
        continue;
      auto next_closure_pair = GetNextClosureFor(token, closure);
      const auto &next_closure = next_closure_pair.second;
      action_[closure][token] = next_closure;
#ifdef DEBUG_MODE
      std::cerr << "------------------------\n";
      std::cerr << closure->to_string(grammar_->productions_of_) << " >> "
                << token->to_string() << " >> \n"
                << next_closure->to_string(grammar_->productions_of_);
      std::cerr << "------------------------\n";
#endif
      if (!next_closure_pair.first) {
        queue.push(next_closure);
      }
    }

    for (const auto &kernel_item : closure->kernel_items_) {
      if (kernel_item.matched_ == kernel_item.production_->body_.size() ||
          (kernel_item.production_->body_.size() == 1 &&
           kernel_item.production_->body_.front()->type_ ==
               Token::Type::BLANK)) {
        for (const auto &token : kernel_item.end_with_) {
          if (token->type_ == Token::Type::BLANK)
            continue;
          if (reduce_[closure].find(token) != reduce_[closure].end()) {
            throw std::invalid_argument(
                std::string("Reduce-Reduce confliction found: ") +
                token->to_string());
          }
          if (action_[closure].find(token) != action_[closure].end()) {
            throw std::invalid_argument(
                std::string("Shift-Reduce confliction found: ") +
                token->to_string());
          }
          reduce_[closure][token] = kernel_item.production_;
        }
      }
    }
  }
}

void LALRTableGenerator::PrintLALRTable() {
  std::cout << std::setw(10) << "name";
  for (const auto &terminator : grammar_->terminators_) {
    std::cout << std::setw(10) << terminator->to_string();
  }
  for (const auto &nonterminator : grammar_->nonterminators_) {
    std::cout << std::setw(10) << nonterminator->to_string();
  }
  std::cout << std::endl;
  for (const auto &closure : closures_) {
    std::cout << std::setw(10) << std::to_string(closure->id_) + "|";
    for (const auto &terminator : grammar_->terminators_) {
      if (action_[closure].find(terminator) != action_[closure].end()) {
        std::cout << std::setw(10)
                  << std::string("s") +
                         std::to_string(action_[closure][terminator]->id_) +
                         "|";
      } else if (reduce_[closure].find(terminator) != reduce_[closure].end()) {
        std::cout << std::setw(10)
                  << std::string("r") +
                         std::to_string(reduce_[closure][terminator]->id_) +
                         "|";
      } else {
        std::cout << std::setw(10) << "|";
      }
    }
    for (const auto &nonterminator : grammar_->nonterminators_) {
      if (action_[closure].find(nonterminator) != action_[closure].end()) {
        std::cout << std::setw(10)
                  << std::to_string(action_[closure][nonterminator]->id_) + "|";
      } else {
        std::cout << std::setw(10) << "|";
      }
    }
    std::cout << "\n";
  }
}

std::pair<bool, ClosurePtr>
LALRTableGenerator::GetClosure(KernelItemVec &&kernel_items) {
  for (uint32_t i = 0; i < closures_.size(); i++) {
    const auto &closure = closures_[i];
    if (*closure == kernel_items) {
      return {true, closure};
    }
  }

  size_t blank_production_count = 0;
  for (const auto &kernel_item : kernel_items) {
    if (kernel_item.production_->body_.front()->type_ == Token::Type::BLANK) {
      blank_production_count++;
      if (blank_production_count > 1) {
        throw std::invalid_argument("Found two blank production in one kernel");
      }
    }
  }

  auto new_closure = std::make_shared<Closure>();
  new_closure->kernel_items_ = std::move(kernel_items);
  new_closure->id_ = closures_.size() + 1;
  std::queue<TokenPtr> queue;
  for (const auto &kernel_item : new_closure->kernel_items_) {
    if (kernel_item.matched_ != kernel_item.production_->body_.size()) {
      const auto &token = kernel_item.production_->body_[kernel_item.matched_];
      auto normal_item_opt = new_closure->GetNormalItem(token);
      size_t idx;
      if (!normal_item_opt.has_value()) {
        new_closure->normal_items_.push_back(NormalItem{token});
        idx = new_closure->normal_items_.size() - 1;
        queue.push(token);
      }
      auto &end_with = new_closure->normal_items_[idx].end_with_;
      if (kernel_item.matched_ + 1 < kernel_item.production_->body_.size()) {
        const auto &first = GetFirstOf(
            kernel_item.production_->body_[kernel_item.matched_ + 1]);
        end_with.insert(first.begin(), first.end());
      } else {
        end_with.insert(kernel_item.end_with_.begin(),
                        kernel_item.end_with_.end());
      }
    }
  }
  while (!queue.empty()) {
    auto token = queue.front();
    queue.pop();
    for (const auto &production : grammar_->productions_of_[token->name_]) {
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
        const auto &old_normal_item =
            new_closure->normal_items_[old_normal_item_idx];
        end_with.insert(old_normal_item.end_with_.begin(),
                        old_normal_item.end_with_.end());
      } else {
        const auto &first = GetFirstOf(production->body_[1]);
        end_with.insert(first.begin(), first.end());
      }
    }
  }
  closures_.emplace_back(new_closure);
  return {false, new_closure};
}

TokenPtrVec LALRTableGenerator::GetNextTokens(const ClosurePtr &closure) {
  TokenPtrVec result;
  for (const auto &kernel_item : closure->kernel_items_) {
    if (kernel_item.matched_ != kernel_item.production_->body_.size()) {
      const auto &token = kernel_item.production_->body_[kernel_item.matched_];
      if (std::find(result.begin(), result.end(), token) == result.end()) {
        result.push_back(token);
      }
    }
  }
  for (const auto &normal_item : closure->normal_items_) {
    for (const auto &production :
         grammar_->productions_of_[normal_item.head_->name_]) {
      if (!production->body_.empty()) {
        const auto &token = production->body_.front();
        if (std::find(result.begin(), result.end(), token) == result.end()) {
          result.push_back(token);
        }
      }
    }
  }
  return result;
}

std::pair<bool, ClosurePtr>
LALRTableGenerator::GetNextClosureFor(const TokenPtr &token,
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
         grammar_->productions_of_[normal_item.head_->name_]) {
      if (!production->body_.empty() && production->body_.front() == token) {
        KernelItem new_item(production, 1);
        new_item.end_with_ = normal_item.end_with_;
        new_closure_kernel.push_back(std::move(new_item));
      }
    }
  }
  return GetClosure(std::move(new_closure_kernel));
}

const TokenPtrSet &LALRTableGenerator::getFollowOf(const TokenPtr &token) {
  auto iter = follow_of_.find(token);
  if (iter != follow_of_.end())
    return iter->second;

  follow_of_[token] = {};
  if (token->type_ != Token::Type::Nonterminator) {
    return follow_of_[token];
  }

  for (const auto &production : grammar_->productions_) {
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

const TokenPtrSet &LALRTableGenerator::GetFirstOf(const TokenPtr &token) {
  auto iter = first_of_.find(token);
  if (iter != first_of_.end())
    return iter->second;

  first_of_[token] = {};
  if (token->type_ != Token::Type::Nonterminator) {
    first_of_[token].insert(token);
    return first_of_[token];
  }

  for (const auto &production : grammar_->productions_of_[token->name_]) {
    bool blank_ntil_the_end = true;
    for (size_t i = 0; i < production->body_.size(); i++) {
      const auto &other_first = GetFirstOf(production->body_[i]);
      first_of_[token].insert(other_first.begin(), other_first.end());
      if (other_first.find(grammar_->blank_) == other_first.end()) {
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
} // namespace LALR
} // namespace siicc