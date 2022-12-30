#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
namespace steiner_tree
{
template <class CostTy> struct edge
{
    const uint32_t v1, v2;
    const CostTy cost;
    edge(uint32_t v1, uint32_t v2, const CostTy &cost) : v1(v1), v2(v2), cost(cost)
    {
    }
    uint32_t getDual(uint32_t v) const
    {
        assert(v == v1 || v == v2);
        if (v == v1)
            return v2;
        else
            return v1;
    }
};
template <class CostTy> class UndirectedGraph
{
    /*
        Undirected Graph use 0-base
    */
    std::vector<edge<CostTy>> edges;
    std::vector<std::vector<uint32_t>> adj_lists;
    CostTy costSum;

  public:
    UndirectedGraph() : edges(), adj_lists(), costSum(0)
    {
    }
    UndirectedGraph(UndirectedGraph &&other)
        : edges(std::move(other.edges)), adj_lists(std::move(other.adj_lists)), costSum(other.costSum)
    {
    }
    UndirectedGraph &operator=(const UndirectedGraph &other)
    {
        edges = other.edges;
        adj_lists = other.adj_lists;
        costSum = other.costSum;
        return *this;
    }
    UndirectedGraph &operator=(UndirectedGraph &&other)
    {
        edges = std::move(other.edges);
        adj_lists = std::move(other.adj_lists);
        costSum = other.costSum;
        return *this;
    }
    void clear()
    {
        edges.clear();
        adj_lists.clear();
        costSum = 0;
    }
    void memoryClear()
    {
        std::vector<edge<CostTy>>().swap(edges);
        std::vector<std::vector<uint32_t>>().swap(adj_lists);
        costSum = 0;
    }
    void setVertexNum(size_t num)
    {
        adj_lists.resize(num);
    }
    size_t addEdge(uint32_t v1, uint32_t v2, const CostTy &cost)
    {
        assert(!(cost < 0));
        if (v1 > v2)
            std::swap(v1, v2);
        while (adj_lists.size() <= v2)
            adj_lists.emplace_back();
        size_t eid = edges.size();
        edges.emplace_back(v1, v2, cost);
        adj_lists[v1].emplace_back(eid);
        adj_lists[v2].emplace_back(eid);
        costSum += cost;
        return eid;
    }
    const std::vector<uint32_t> &getAdjacencyList(uint32_t v) const
    {
        return adj_lists.at(v);
    }
    const edge<CostTy> &getEdge(uint32_t eid) const
    {
        return edges.at(eid);
    }
    const std::vector<edge<CostTy>> &getEdges() const
    {
        return edges;
    }
    size_t getEdgeNum() const
    {
        return edges.size();
    }
    size_t getVertexNum() const
    {
        return adj_lists.size();
    }
    CostTy getEdgeCosts() const
    {
        return costSum;
    }
};
} // namespace steiner_tree