#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "DisjoinSet.hpp"
#include "Queue.hpp"
#include "Structure.hpp"
namespace steiner_tree
{
template <class CostTy> class Solver
{
    const size_t INVLID;
    const CostTy INF;

    const UndirectedGraph<CostTy> &G;

    std::vector<CostTy> dist;
    std::vector<size_t> prev_eid;
    std::vector<size_t> index;

    std::vector<std::tuple<CostTy, size_t>> CrossEdge;
    std::vector<size_t> SelectKEdge;

    DisjoinSet ds;

    // Propageta Wavefront in the whole graph constructed by boundary
    void SPFA(const std::unordered_set<size_t> &terminals)
    {
        auto N = G.getVertexNum();
        std::vector<bool> inqueue(N, false);
        Queue<size_t> q;

        dist.clear();
        dist.resize(N, INF);

        prev_eid.clear();
        prev_eid.resize(N, INVLID);

        index.clear();
        index.resize(N, INVLID);

        for (auto v : terminals)
        {
            dist.at(v) = 0;
            index.at(v) = v;
            q.push_back(v);
            inqueue.at(v) = true;
        }
        while (!q.empty())
        {
            auto v = q.front();
            q.pop_front();
            inqueue[v] = false;
            for (auto eid : G.getAdjacencyList(v)) // for neighbor of v
            {
                const auto &edge = G.getEdge(eid);
                const auto dual = edge.getDual(v);
                if (dist[dual] > dist[v] + edge.cost)
                {
                    dist[dual] = dist[v] + edge.cost; // relax edge cost
                    prev_eid[dual] = eid;
                    index[dual] = index[v];
                    if (!inqueue[dual])
                    {
                        if (!q.empty() && dist[dual] <= dist[q.front()])
                            q.push_front(dual);
                        else
                            q.push_back(dual);
                        inqueue[dual] = true;
                    }
                }
            }
        }
    }

    // CrossEdge: the edge that connecting two pins
    void calculateCrossEdge()
    {
        CrossEdge.clear();
        size_t eid = 0;
        for (const auto &edge : G.getEdges())
        {   
            // If this edge's two connecting vertex's index is different
            if (index[edge.v1] != index[edge.v2])
            {
                if (index[edge.v1] != INVLID && index[edge.v2] != INVLID)
                {   
                    // Connecting wavefront intersection point
                    CrossEdge.emplace_back(dist[edge.v1] + dist[edge.v2] + edge.cost, eid); 
                }
            }
            ++eid;
        }
    }

    bool Kruskal(const std::unordered_set<size_t> &terminals)
    {
        SelectKEdge.clear();
        // sort crossedge in non-descending order
        std::sort(CrossEdge.begin(), CrossEdge.end());
        // Disjoint Set
        // Init set of each pin as containing itself only
        ds.init(G.getVertexNum(), terminals);
        size_t unionCnt = 0;
        for (const auto &TUS : CrossEdge)
        {
            size_t eid;
            std::tie(std::ignore, eid) = TUS;
            const auto &edge = G.getEdge(eid);
            // if in different set
            if (!ds.same(index[edge.v1], index[edge.v2]))
            {
                SelectKEdge.emplace_back(eid);
                // union two disjoint set
                ds.Union(index[edge.v1], index[edge.v2]);
                ++unionCnt;
            }
        }
        // all pins is in the same set
        return terminals.size() == unionCnt + 1;
    }

    std::shared_ptr<std::vector<size_t>> edgeRecover()
    {
        std::vector<bool> used(G.getEdgeNum(), false);
        auto resultEdge = std::make_shared<std::vector<size_t>>();
        for (auto eid : SelectKEdge)
        {
            const auto &edge = G.getEdge(eid);
            resultEdge->emplace_back(eid);
            for (int t = 0; t < 2; ++t)
            {
                size_t v = (t & 1) ? edge.v1 : edge.v2;
                while (v != index[v] && !used[prev_eid[v]])
                {
                    used[prev_eid[v]] = true;
                    const auto &prev_edge = G.getEdge(prev_eid[v]);
                    resultEdge->emplace_back(prev_eid[v]);
                    v = prev_edge.getDual(v);
                }
            }
        }
        return resultEdge;
    }

  public:
    Solver(const UndirectedGraph<CostTy> &G, const CostTy INF = std::numeric_limits<CostTy>::max() / 2 - 1)
        : INVLID(std::numeric_limits<size_t>::max()), INF(INF), G(G)
    {
        assert(G.getEdgeCosts() < INF);
    }
    std::shared_ptr<std::vector<size_t>> solve(const std::unordered_set<size_t> &terminals)
    {
        SPFA(terminals);
        calculateCrossEdge();
        // check all pins can be connected by cross edge
        if (!Kruskal(terminals))
            return nullptr;
        return edgeRecover();
    }
};
} // namespace steiner_tree