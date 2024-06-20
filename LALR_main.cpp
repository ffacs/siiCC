#include "siicc_EBNF.h"
#include "LALR_EBNF_lexer.h"
#include "LALR_parser_generator.h"
#include "LALR_table_generator.h"

class EBNFGrammarParserGenerator {
public:
    EBNFGrammarParserGenerator(siicc::ASTNodePtr root) : root_(root)  {}
    
    void generate();

private:
    void Visit(siicc::ASTNodePtr node) {        
        for (const auto& child : node->children_) {
            if (child->type_ == siicc::ASTNode::Type::NODE_START) {
                throw std::invalid_argument("Error node start");
            } else if (child->type_ == siicc::ASTNode::Type::LEAF_Blank) {
                throw std::invalid_argument("Error node Blank");                
            } 
            if (child->type_ == siicc::ASTNode::Type::NODE_Production) {
                CreateGrammarFromProduction(child);
                return;
            } 
            Visit(child);
        }
    }
    
    siicc::LALR::TokenPtr GetTerminator(const std::string& name) {
        //
        throw "Not impl yet.";
    }
    
    void CreateGrammarFromProduction(siicc::ASTNodePtr production_node) {
        if (production_node->children_.at(0)->type_ != siicc::ASTNode::Type::LEAF_nonterminator) {
            throw std::invalid_argument("First child of production is not nonterminator.");
        }
    }

    siicc::ASTNodePtr root_;
    siicc::LALR::GrammarPtr grammar_;
    siicc::LALR::TokenPtrSet terminators_;
    siicc::LALR::TokenPtrSet nonterminators_;
    siicc::LALR::ProductionPtrVec productions_;
    siicc::LALR::TokenPtr blank_;
    siicc::LALR::TokenPtr start_;
};

int main(int argc, char**argv) {
    if (argc <= 1) {
            throw std::invalid_argument("No file specified.");
    }
    for (int i = 1; i < argc; i++) {
        std::ifstream is(argv[i]);
        auto lexer = std::make_shared<siicc::BNFLexer>(is);
        auto result = Parse(lexer);
    }
}