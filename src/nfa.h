#ifndef NFA_H
#define NFA_H

#include "graph.h"
#include <string>
#include <memory>
#include <stack>
#include <map>

// 正则表达式到NFA的转换器
class NFABuilder
{
public:
    NFABuilder();
    ~NFABuilder();

    // 构建NFA
    std::shared_ptr<Graph> buildNFA(const std::string &regex);

private:
    // Thompson构造法的基本构造单元
    std::shared_ptr<Graph> createBasicNFA(char c);
    std::shared_ptr<Graph> createUnionNFA(std::shared_ptr<Graph> nfa1, std::shared_ptr<Graph> nfa2);
    std::shared_ptr<Graph> createConcatNFA(std::shared_ptr<Graph> nfa1, std::shared_ptr<Graph> nfa2);
    std::shared_ptr<Graph> createStarNFA(std::shared_ptr<Graph> nfa);
    std::shared_ptr<Graph> createPlusNFA(std::shared_ptr<Graph> nfa);
    std::shared_ptr<Graph> createOptionalNFA(std::shared_ptr<Graph> nfa);

    // 辅助函数
    bool isOperator(char c) const;
    int getPrecedence(char op) const;
    std::string infixToPostfix(const std::string &infix);
    std::map<int, int> mergeNFA(std::shared_ptr<Graph> &target, const std::shared_ptr<Graph> &source);

    int stateCounter; // 状态计数器，用于生成唯一的状态ID
};

#endif // NFA_H