#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <set>
#include "graph.h"
#include "nfa.h"
#include "dfa.h"

// 辅助函数：从正则表达式中提取字母表
std::set<char> extract_alphabet_from_regexp(const std::string &regexp)
{
    std::set<char> alphabet;
    for (char c : regexp)
    {
        if (isalnum(c) && c != '$' && c != '*' && c != '(' && c != ')')
        {
            alphabet.insert(c);
        }
    }
    return alphabet;
}

// 辅助函数：获取完整的字母表
std::set<char> get_full_alphabet(const std::shared_ptr<Graph> &graph, const std::string &regexp, bool include_epsilon = false)
{
    // 从正则表达式中获取字母表
    auto alphabet = extract_alphabet_from_regexp(regexp);

    // 从图中获取字母表
    for (char c : graph->getAlphabet())
    {
        if (include_epsilon || c != '$')
        {
            alphabet.insert(c);
        }
    }
    return alphabet;
}

// 辅助函数：打印状态转换表
void print_transition_table(const std::shared_ptr<Graph> &graph, const std::string &title, const std::string &regexp, bool is_nfa = false)
{
    std::cout << title << ":\n";

    // 获取所有状态和完整的字母表
    auto states = graph->getAllStates();
    auto alphabet = get_full_alphabet(graph, regexp, is_nfa);

    // 计算每列的宽度
    size_t state_width = 6; // "State" 的长度
    for (int state : states)
    {
        state_width = std::max(state_width, std::to_string(state).length());
    }
    state_width += 2; // 添加一些padding

    // 打印表头
    std::cout << std::setw(state_width) << "State";
    for (char c : alphabet)
    {
        if (c == '$')
        {
            std::cout << std::setw(8) << "ε"; // 使用ε符号代替$
        }
        else
        {
            std::cout << std::setw(8) << c;
        }
    }
    std::cout << "  Accept?\n";

    // 打印分隔线
    for (size_t i = 0; i < state_width + alphabet.size() * 8 + 8; ++i)
    {
        std::cout << "-";
    }
    std::cout << "\n";

    // 打印每个状态的转换
    for (int state : states)
    {
        // 打印状态号
        std::cout << std::setw(state_width) << state;

        // 打印每个输入符号的转换
        for (char c : alphabet)
        {
            auto next_states = graph->getNextStates(state, c);
            std::stringstream ss;
            bool first = true;
            for (int next : next_states)
            {
                if (!first)
                    ss << ",";
                ss << next;
                first = false;
            }
            std::cout << std::setw(8) << (ss.str().empty() ? "-" : ss.str());
        }

        // 打印是否为接受状态和初始状态
        std::cout << "  " << (graph->getAcceptStates().count(state) > 0 ? "Yes" : "No");
        if (state == graph->getInitialState())
        {
            std::cout << " (Initial)";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <regexp>" << std::endl;
        return 1;
    }

    std::string regexp = argv[1];

    try
    {
        // 构建NFA
        NFABuilder nfa_builder;
        auto nfa = nfa_builder.buildNFA(regexp);
        print_transition_table(nfa, "NFA状态转换表", regexp, true); // true表示这是NFA

        // 转换为DFA
        DFABuilder dfa_builder;
        auto dfa = dfa_builder.buildDFA(nfa);
        print_transition_table(dfa, "DFA状态转换表", regexp);

        // 最小化DFA
        auto min_dfa = dfa_builder.minimizeDFA(dfa);
        print_transition_table(min_dfa, "最小化DFA状态转换表", regexp);

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}