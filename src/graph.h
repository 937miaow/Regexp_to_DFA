#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_map>
#include <set>
#include <string>

struct Edge
{
    int u;  // 起始状态
    int v;  // 目标状态
    char w; // 转换字符

    Edge(int _u, int _v, char _w) : u(_u), v(_v), w(_w) {}
    bool operator==(const Edge &other) const
    {
        return u == other.u && v == other.v && w == other.w;
    }
};

class Graph
{
public:
    Graph();
    ~Graph();

    // 添加状态
    void addState(int state);
    // 添加边（转换）
    void addEdge(int u, int v, char w);
    // 设置初始状态
    void setInitialState(int state);
    // 添加接受状态
    void addAcceptState(int state);
    // 获取从状态s出发，接受字符c的所有目标状态
    std::set<int> getNextStates(int s, char c) const;
    // 获取从状态集合S出发，接受字符c的所有目标状态
    std::set<int> getNextStates(const std::set<int> &S, char c) const;
    // 获取所有状态
    std::set<int> getAllStates() const;
    // 获取所有转换字符
    std::set<char> getAlphabet() const;
    // 获取初始状态
    int getInitialState() const;
    // 获取所有接受状态
    const std::set<int> &getAcceptStates() const;
    // 获取所有边
    const std::vector<Edge> &getEdges() const;

private:
    std::set<int> states;       // 所有状态
    std::vector<Edge> edges;    // 所有边
    std::set<char> alphabet;    // 字母表
    int initialState;           // 初始状态
    std::set<int> acceptStates; // 接受状态集合
    std::unordered_map<int,
                       std::unordered_map<char, std::set<int>>>
        transitions; // 转换函数
};

#endif // GRAPH_H