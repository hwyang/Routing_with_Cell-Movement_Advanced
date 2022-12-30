#pragma once
#include <algorithm>
#include <unordered_set>
#include <vector>
/*
    DisjoinSet use 0-base
*/
namespace steiner_tree
{
class DisjoinSet
{
    size_t sz;
    std::vector<size_t> dis;
    std::vector<size_t> sum;

  public:
    DisjoinSet(size_t s = 0)
    {
        init(s);
    }
    void init(size_t s, const std::unordered_set<size_t> &special = {})
    {
        sz = s;
        dis.resize(sz);
        sum.resize(sz);
        if (special.size())
        {
            for (auto i : special)
            {
                dis[i] = i;
                sum[i] = 1;
            }
        }
        else
        {
            for (size_t i = 0; i < sz; ++i)
            {
                dis[i] = i;
                sum[i] = 1;
            }
        }
    }
    size_t find(size_t p)
    {
        if (dis[p] == p)
            return p;
        return dis[p] = find(dis[p]);
    }
    bool same(size_t a, size_t b)
    {
        return find(a) == find(b);
    }
    void Union(size_t a, size_t b)
    {
        if (!same(a, b))
        {
            if (sum[dis[a]] < sum[dis[b]])
                std::swap(a, b);
            sum[dis[a]] += sum[dis[b]];
            dis[dis[b]] = dis[a];
        }
    }
    size_t size(size_t p)
    {
        return sum[find(p)];
    }
};
} // namespace steiner_tree