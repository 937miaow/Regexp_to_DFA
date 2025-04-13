#include "dfa.h"
#include <queue>
#include <stack>
#include <algorithm>

#define EPSILON_CHAR '$' // 使用$作为epsilon转换的符号

DFABuilder::DFABuilder() : stateCounter(0) {}

DFABuilder::~DFABuilder() {}

std::shared_ptr<Graph> DFABuilder::buildDFA(const std::shared_ptr<Graph> &nfa)
{
    // 重置状态计数器
    stateCounter = 0;
    stateSetToId.clear();

    auto dfa = std::make_shared<Graph>();
    std::queue<std::set<int>> unprocessedStates;
    std::set<std::set<int>> processedStates;

    // 计算初始状态的ε闭包
    std::set<int> initialState = epsilonClosure(nfa, nfa->getInitialState());
    int initialStateId = getStateId(initialState);

    dfa->addState(initialStateId);
    dfa->setInitialState(initialStateId);
    unprocessedStates.push(initialState);
    processedStates.insert(initialState);

    // 如果初始状态包含NFA的接受状态，则将其设为DFA的接受状态
    for (int state : initialState)
    {
        if (nfa->getAcceptStates().count(state))
        {
            dfa->addAcceptState(initialStateId);
            break;
        }
    }

    // 获取完整的字母表（不包括epsilon）
    std::set<char> fullAlphabet = nfa->getAlphabet();
    fullAlphabet.erase(EPSILON_CHAR);

    // 处理所有未处理的状态
    while (!unprocessedStates.empty())
    {
        std::set<int> currentStates = unprocessedStates.front();
        unprocessedStates.pop();
        int currentStateId = getStateId(currentStates);

        // 对于每个输入符号
        for (char symbol : fullAlphabet)
        {
            // 计算转换后的状态集合
            std::set<int> nextStates = epsilonClosure(nfa, move(nfa, currentStates, symbol));

            if (nextStates.empty())
                continue;

            int nextStateId = getStateId(nextStates);

            // 如果是新状态，添加到DFA中
            if (processedStates.find(nextStates) == processedStates.end())
            {
                dfa->addState(nextStateId);

                // 检查是否包含接受状态
                for (int state : nextStates)
                {
                    if (nfa->getAcceptStates().count(state))
                    {
                        dfa->addAcceptState(nextStateId);
                        break;
                    }
                }

                unprocessedStates.push(nextStates);
                processedStates.insert(nextStates);
            }

            // 添加转换边
            dfa->addEdge(currentStateId, nextStateId, symbol);
        }
    }

    return dfa;
}

std::shared_ptr<Graph> DFABuilder::minimizeDFA(const std::shared_ptr<Graph> &dfa)
{
    // 计算初始划分
    std::vector<std::set<int>> partition = computeInitialPartition(dfa);

    // 细化划分直到不能再细化
    refinePartition(dfa, partition);

    // 构建最小化DFA
    auto minDfa = std::make_shared<Graph>();
    std::map<int, int> oldToNewState;
    int newStateId = 0;

    // 为每个等价类分配新的状态ID
    for (const auto &group : partition)
    {
        for (int state : group)
        {
            oldToNewState[state] = newStateId;
        }
        minDfa->addState(newStateId);

        // 如果组包含原DFA的接受状态，则新状态也是接受状态
        for (int state : group)
        {
            if (dfa->getAcceptStates().count(state))
            {
                minDfa->addAcceptState(newStateId);
                break;
            }
        }

        // 如果组包含原DFA的初始状态，则新状态也是初始状态
        for (int state : group)
        {
            if (state == dfa->getInitialState())
            {
                minDfa->setInitialState(newStateId);
                break;
            }
        }

        newStateId++;
    }

    // 添加转换边
    for (const auto &group : partition)
    {
        int representativeState = *group.begin();
        int fromState = oldToNewState[representativeState];

        for (char symbol : dfa->getAlphabet())
        {
            auto nextStates = dfa->getNextStates(representativeState, symbol);
            if (!nextStates.empty())
            {
                int toState = oldToNewState[*nextStates.begin()];
                minDfa->addEdge(fromState, toState, symbol);
            }
        }
    }

    return minDfa;
}

std::set<int> DFABuilder::epsilonClosure(const std::shared_ptr<Graph> &nfa, int state)
{
    std::set<int> closure{state};
    std::stack<int> stack;
    stack.push(state);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        auto nextStates = nfa->getNextStates(current, EPSILON_CHAR);
        for (int next : nextStates)
        {
            if (closure.insert(next).second)
            {
                stack.push(next);
            }
        }
    }

    return closure;
}

std::set<int> DFABuilder::epsilonClosure(const std::shared_ptr<Graph> &nfa, const std::set<int> &states)
{
    std::set<int> closure;
    for (int state : states)
    {
        auto stateClosure = epsilonClosure(nfa, state);
        closure.insert(stateClosure.begin(), stateClosure.end());
    }
    return closure;
}

std::set<int> DFABuilder::move(const std::shared_ptr<Graph> &nfa, const std::set<int> &states, char symbol)
{
    std::set<int> result;
    for (int state : states)
    {
        auto nextStates = nfa->getNextStates(state, symbol);
        result.insert(nextStates.begin(), nextStates.end());
    }
    return result;
}

int DFABuilder::getStateId(const std::set<int> &states)
{
    auto it = stateSetToId.find(states);
    if (it != stateSetToId.end())
    {
        return it->second;
    }
    int newId = stateCounter++;
    stateSetToId[states] = newId;
    return newId;
}

std::vector<std::set<int>> DFABuilder::computeInitialPartition(const std::shared_ptr<Graph> &dfa)
{
    std::vector<std::set<int>> partition;
    std::set<int> acceptStates = dfa->getAcceptStates();
    std::set<int> nonAcceptStates;

    // 将状态分为接受状态和非接受状态两组
    for (int state : dfa->getAllStates())
    {
        if (acceptStates.count(state))
        {
            continue;
        }
        nonAcceptStates.insert(state);
    }

    if (!acceptStates.empty())
    {
        partition.push_back(acceptStates);
    }
    if (!nonAcceptStates.empty())
    {
        partition.push_back(nonAcceptStates);
    }

    return partition;
}

void DFABuilder::refinePartition(const std::shared_ptr<Graph> &dfa, std::vector<std::set<int>> &partition)
{
    bool changed;
    do
    {
        changed = false;
        std::vector<std::set<int>> newPartition;

        // 对每个组尝试进行分割
        for (const auto &group : partition)
        {
            if (group.size() <= 1)
            {
                newPartition.push_back(group);
                continue;
            }

            bool split = false;
            // 尝试用其他组和输入符号分割当前组
            for (const auto &splitter : partition)
            {
                for (char symbol : dfa->getAlphabet())
                {
                    if (canSplit(dfa, group, splitter, symbol))
                    {
                        std::set<int> group1, group2;

                        // 根据转换目标将状态分为两组
                        for (int state : group)
                        {
                            auto nextStates = dfa->getNextStates(state, symbol);
                            bool goesToSplitter = false;
                            if (!nextStates.empty())
                            {
                                int nextState = *nextStates.begin();
                                goesToSplitter = splitter.count(nextState) > 0;
                            }
                            if (goesToSplitter)
                            {
                                group1.insert(state);
                            }
                            else
                            {
                                group2.insert(state);
                            }
                        }

                        newPartition.push_back(group1);
                        newPartition.push_back(group2);
                        split = true;
                        changed = true;
                        break;
                    }
                }
                if (split)
                    break;
            }

            if (!split)
            {
                newPartition.push_back(group);
            }
        }

        partition = newPartition;
    } while (changed);
}

bool DFABuilder::canSplit(const std::shared_ptr<Graph> &dfa, const std::set<int> &group,
                          const std::set<int> &splitter, char symbol)
{
    bool foundInSplitter = false;
    bool foundOutSplitter = false;

    for (int state : group)
    {
        auto nextStates = dfa->getNextStates(state, symbol);
        if (nextStates.empty())
            continue;

        int nextState = *nextStates.begin();
        if (splitter.count(nextState))
        {
            foundInSplitter = true;
        }
        else
        {
            foundOutSplitter = true;
        }

        if (foundInSplitter && foundOutSplitter)
        {
            return true;
        }
    }

    return false;
}