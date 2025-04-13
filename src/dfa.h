#ifndef DFA_H
#define DFA_H

#include "graph.h"
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <string>

// DFA构造器类
class DFABuilder
{
public:
    DFABuilder();
    ~DFABuilder();

    // 使用子集构造法从NFA构造DFA
    std::shared_ptr<Graph> buildDFA(const std::shared_ptr<Graph> &nfa);

    // 使用Hopcroft算法最小化DFA
    std::shared_ptr<Graph> minimizeDFA(const std::shared_ptr<Graph> &dfa);

private:
    // 计算ε闭包
    std::set<int> epsilonClosure(const std::shared_ptr<Graph> &nfa, const std::set<int> &states);
    std::set<int> epsilonClosure(const std::shared_ptr<Graph> &nfa, int state);

    // 获取状态集合的转换目标状态集合
    std::set<int> move(const std::shared_ptr<Graph> &nfa, const std::set<int> &states, char symbol);

    // 为状态集合生成唯一的状态ID
    int getStateId(const std::set<int> &states);

    // Hopcroft算法辅助函数
    std::vector<std::set<int>> computeInitialPartition(const std::shared_ptr<Graph> &dfa);
    void refinePartition(const std::shared_ptr<Graph> &dfa, std::vector<std::set<int>> &partition);
    bool canSplit(const std::shared_ptr<Graph> &dfa, const std::set<int> &group,
                  const std::set<int> &splitter, char symbol);

    std::map<std::set<int>, int> stateSetToId; // 状态集合到状态ID的映射
    int stateCounter;                          // 状态计数器
};

#endif // DFA_H