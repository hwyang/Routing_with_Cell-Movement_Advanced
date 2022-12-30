#pragma once
#include <vector>
namespace steiner_tree
{
template <typename TP> class Queue
{
    std::vector<TP> v;
    using sz_t = std::size_t;
    sz_t p2;
    sz_t mask;
    sz_t S, T;
    void init(int p2sz)
    {
        p2 = (1U << p2sz);
        mask = p2 - 1;
        v.resize(p2);
    }
    sz_t next(sz_t p) const
    {
        return (p + 1) & mask;
    }
    sz_t prev(sz_t p) const
    {
        return (p + p2 - 1) & mask;
    }
    bool full() const
    {
        return next(S) == T;
    }
    void extend()
    {
        sz_t nsz = p2 << 1;
        v.resize(nsz);
        if (S < T)
        {
            for (sz_t i = T; i < p2; ++i)
            {
                v[i + p2] = v[i];
            }
            T += p2;
        }
        p2 = nsz;
        mask = p2 - 1;
    }

  public:
    Queue()
    {
        init(15);
        S = T = 0;
    }
    void push_back(const TP &val)
    {
        if (full())
            extend();
        v[S] = val;
        S = next(S);
    }
    void push_front(const TP &val)
    {
        if (full())
            extend();
        T = prev(T);
        v[T] = val;
    }
    TP &front()
    {
        return v[T];
    }
    void pop_front()
    {
        T = next(T);
    }
    bool empty() const
    {
        return S == T;
    }
};
} // namespace steiner_tree