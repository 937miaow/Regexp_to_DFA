#include "graph.h"

Graph::Graph() : initialState(-1) {}

Graph::~Graph() {}

void Graph::addState(int state)
{
    states.insert(state);
}

void Graph::addEdge(int u, int v, char w)
{
    edges.emplace_back(u, v, w);
    alphabet.insert(w);
    transitions[u][w].insert(v);
}

void Graph::setInitialState(int state)
{
    initialState = state;
    states.insert(state);
}

void Graph::addAcceptState(int state)
{
    acceptStates.insert(state);
    states.insert(state);
}

std::set<int> Graph::getNextStates(int s, char c) const
{
    if (transitions.count(s) && transitions.at(s).count(c))
    {
        return transitions.at(s).at(c);
    }
    return std::set<int>();
}

std::set<int> Graph::getNextStates(const std::set<int> &S, char c) const
{
    std::set<int> result;
    for (int s : S)
    {
        auto nextStates = getNextStates(s, c);
        result.insert(nextStates.begin(), nextStates.end());
    }
    return result;
}

std::set<int> Graph::getAllStates() const
{
    return states;
}

std::set<char> Graph::getAlphabet() const
{
    return alphabet;
}

int Graph::getInitialState() const
{
    return initialState;
}

const std::set<int> &Graph::getAcceptStates() const
{
    return acceptStates;
}

const std::vector<Edge> &Graph::getEdges() const
{
    return edges;
}