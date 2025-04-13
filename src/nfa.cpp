#include "nfa.h"
#include <stack>
#include <algorithm>
#include <map>

#define EPSILON_CHAR '$' // 使用$作为epsilon转换的符号

NFABuilder::NFABuilder() : stateCounter(0) {}

NFABuilder::~NFABuilder() {}

std::shared_ptr<Graph> NFABuilder::buildNFA(const std::string &regex)
{
    // 重置状态计数器
    stateCounter = 0;

    // 先构建NFA
    std::string postfix = infixToPostfix(regex);
    std::stack<std::shared_ptr<Graph>> nfaStack;

    for (char c : postfix)
    {
        if (!isOperator(c))
        {
            nfaStack.push(createBasicNFA(c));
        }
        else
        {
            switch (c)
            {
            case '|':
            {
                auto nfa2 = nfaStack.top();
                nfaStack.pop();
                auto nfa1 = nfaStack.top();
                nfaStack.pop();
                nfaStack.push(createUnionNFA(nfa1, nfa2));
                break;
            }
            case '.':
            { // 连接运算符
                auto nfa2 = nfaStack.top();
                nfaStack.pop();
                auto nfa1 = nfaStack.top();
                nfaStack.pop();
                nfaStack.push(createConcatNFA(nfa1, nfa2));
                break;
            }
            case '*':
            {
                auto nfa = nfaStack.top();
                nfaStack.pop();
                nfaStack.push(createStarNFA(nfa));
                break;
            }
            case '+':
            {
                auto nfa = nfaStack.top();
                nfaStack.pop();
                nfaStack.push(createPlusNFA(nfa));
                break;
            }
            case '?':
            {
                auto nfa = nfaStack.top();
                nfaStack.pop();
                nfaStack.push(createOptionalNFA(nfa));
                break;
            }
            }
        }
    }

    // 获取构建好的NFA
    auto result = nfaStack.top();

    // 重新映射所有状态，确保从0开始
    auto remappedNFA = std::make_shared<Graph>();
    std::map<int, int> stateMap;
    int newCounter = 0;

    // 创建新的状态映射
    for (int oldState : result->getAllStates())
    {
        stateMap[oldState] = newCounter++;
        remappedNFA->addState(stateMap[oldState]);
    }

    // 复制所有边
    for (const Edge &edge : result->getEdges())
    {
        remappedNFA->addEdge(stateMap[edge.u], stateMap[edge.v], edge.w);
    }

    // 设置初始状态和接受状态
    remappedNFA->setInitialState(stateMap[result->getInitialState()]);
    for (int acceptState : result->getAcceptStates())
    {
        remappedNFA->addAcceptState(stateMap[acceptState]);
    }

    return remappedNFA;
}

std::shared_ptr<Graph> NFABuilder::createBasicNFA(char c)
{
    auto nfa = std::make_shared<Graph>();
    int start = stateCounter++;
    int end = stateCounter++;

    nfa->addState(start);
    nfa->addState(end);
    nfa->setInitialState(start);
    nfa->addAcceptState(end);
    nfa->addEdge(start, end, c);

    return nfa;
}

std::shared_ptr<Graph> NFABuilder::createUnionNFA(std::shared_ptr<Graph> nfa1, std::shared_ptr<Graph> nfa2)
{
    auto result = std::make_shared<Graph>();
    int start = stateCounter++;
    int end = stateCounter++;

    result->addState(start);
    result->addState(end);
    result->setInitialState(start);
    result->addAcceptState(end);

    // 合并两个NFA并获取状态映射
    auto map1 = mergeNFA(result, nfa1);
    auto map2 = mergeNFA(result, nfa2);

    // 使用映射后的状态添加ε转换
    result->addEdge(start, map1[nfa1->getInitialState()], EPSILON_CHAR);
    result->addEdge(start, map2[nfa2->getInitialState()], EPSILON_CHAR);

    for (int acceptState : nfa1->getAcceptStates())
    {
        result->addEdge(map1[acceptState], end, EPSILON_CHAR);
    }
    for (int acceptState : nfa2->getAcceptStates())
    {
        result->addEdge(map2[acceptState], end, EPSILON_CHAR);
    }

    return result;
}

std::shared_ptr<Graph> NFABuilder::createConcatNFA(std::shared_ptr<Graph> nfa1, std::shared_ptr<Graph> nfa2)
{
    auto result = std::make_shared<Graph>();

    // 合并两个NFA并获取状态映射
    auto map1 = mergeNFA(result, nfa1);
    auto map2 = mergeNFA(result, nfa2);

    result->setInitialState(map1[nfa1->getInitialState()]);

    // 使用映射后的状态连接nfa1的接受状态到nfa2的初始状态
    for (int acceptState : nfa1->getAcceptStates())
    {
        result->addEdge(map1[acceptState], map2[nfa2->getInitialState()], EPSILON_CHAR);
    }

    // 设置nfa2的映射后的接受状态为新NFA的接受状态
    for (int acceptState : nfa2->getAcceptStates())
    {
        result->addAcceptState(map2[acceptState]);
    }

    return result;
}

std::shared_ptr<Graph> NFABuilder::createStarNFA(std::shared_ptr<Graph> nfa)
{
    auto result = std::make_shared<Graph>();
    int start = stateCounter++;
    int end = stateCounter++;

    result->addState(start);
    result->addState(end);
    result->setInitialState(start);
    result->addAcceptState(end);

    // 合并原NFA并获取状态映射
    auto map = mergeNFA(result, nfa);

    // 使用映射后的状态添加ε转换
    result->addEdge(start, end, EPSILON_CHAR);
    result->addEdge(start, map[nfa->getInitialState()], EPSILON_CHAR);

    for (int acceptState : nfa->getAcceptStates())
    {
        result->addEdge(map[acceptState], end, EPSILON_CHAR);
        result->addEdge(map[acceptState], map[nfa->getInitialState()], EPSILON_CHAR);
    }

    return result;
}

std::shared_ptr<Graph> NFABuilder::createPlusNFA(std::shared_ptr<Graph> nfa)
{
    auto result = std::make_shared<Graph>();
    int start = stateCounter++;
    int end = stateCounter++;

    result->addState(start);
    result->addState(end);
    result->setInitialState(start);
    result->addAcceptState(end);

    // 合并原NFA并获取状态映射
    auto map = mergeNFA(result, nfa);

    // 使用映射后的状态添加ε转换
    result->addEdge(start, map[nfa->getInitialState()], EPSILON_CHAR);

    for (int acceptState : nfa->getAcceptStates())
    {
        result->addEdge(map[acceptState], end, EPSILON_CHAR);
        result->addEdge(map[acceptState], map[nfa->getInitialState()], EPSILON_CHAR);
    }

    return result;
}

std::shared_ptr<Graph> NFABuilder::createOptionalNFA(std::shared_ptr<Graph> nfa)
{
    auto result = std::make_shared<Graph>();
    int start = stateCounter++;
    int end = stateCounter++;

    result->addState(start);
    result->addState(end);
    result->setInitialState(start);
    result->addAcceptState(end);

    // 合并原NFA并获取状态映射
    auto map = mergeNFA(result, nfa);

    // 使用映射后的状态添加ε转换
    result->addEdge(start, end, EPSILON_CHAR);
    result->addEdge(start, map[nfa->getInitialState()], EPSILON_CHAR);

    for (int acceptState : nfa->getAcceptStates())
    {
        result->addEdge(map[acceptState], end, EPSILON_CHAR);
    }

    return result;
}

bool NFABuilder::isOperator(char c) const
{
    return c == '|' || c == '.' || c == '*' || c == '+' || c == '?';
}

int NFABuilder::getPrecedence(char op) const
{
    switch (op)
    {
    case '*':
    case '+':
    case '?':
        return 3;
    case '.':
        return 2;
    case '|':
        return 1;
    default:
        return 0;
    }
}

std::string NFABuilder::infixToPostfix(const std::string &infix)
{
    std::string postfix;
    std::stack<char> operators;
    bool lastWasOperand = false;

    for (size_t i = 0; i < infix.length(); i++)
    {
        char c = infix[i];

        if (c == '\\' && i + 1 < infix.length())
        {
            postfix += infix[++i];
            lastWasOperand = true;
            continue;
        }

        if (!isOperator(c) && c != '(' && c != ')')
        {
            if (lastWasOperand)
            {
                // 插入显式连接运算符
                while (!operators.empty() && operators.top() != '(' &&
                       getPrecedence(operators.top()) >= getPrecedence('.'))
                {
                    postfix += operators.top();
                    operators.pop();
                }
                operators.push('.');
            }
            postfix += c;
            lastWasOperand = true;
        }
        else if (c == '(')
        {
            if (lastWasOperand)
            {
                operators.push('.');
            }
            operators.push(c);
            lastWasOperand = false;
        }
        else if (c == ')')
        {
            while (!operators.empty() && operators.top() != '(')
            {
                postfix += operators.top();
                operators.pop();
            }
            if (!operators.empty())
            {
                operators.pop(); // 弹出'('
            }
            lastWasOperand = true;
        }
        else
        {
            while (!operators.empty() && operators.top() != '(' &&
                   getPrecedence(operators.top()) >= getPrecedence(c))
            {
                postfix += operators.top();
                operators.pop();
            }
            operators.push(c);
            lastWasOperand = false;
        }
    }

    while (!operators.empty())
    {
        postfix += operators.top();
        operators.pop();
    }

    return postfix;
}

std::map<int, int> NFABuilder::mergeNFA(std::shared_ptr<Graph> &target, const std::shared_ptr<Graph> &source)
{
    // 创建状态映射
    std::map<int, int> stateMap;

    // 为源NFA中的每个状态创建新的状态编号
    for (int oldState : source->getAllStates())
    {
        int newState = stateCounter++;
        stateMap[oldState] = newState;
        target->addState(newState);
    }

    // 使用新的状态编号添加边
    for (const Edge &edge : source->getEdges())
    {
        target->addEdge(stateMap[edge.u], stateMap[edge.v], edge.w);
    }

    // 更新接受状态
    for (int oldAcceptState : source->getAcceptStates())
    {
        target->addAcceptState(stateMap[oldAcceptState]);
    }

    return stateMap;
}