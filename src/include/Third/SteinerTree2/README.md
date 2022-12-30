# steiner-tree-2-approximation
steiner tree 2-approximation O(E + V log V)

Paper: [A faster approximation algorithm for the Steiner problem in graphs](https://www.sciencedirect.com/science/article/pii/002001908890066X)

Step 4,5 of Algorithm H in this paper is unnecessary after replace step 1 by the paper's method, so I removed it.

## Example usage
```cpp
#include "Solver.hpp"
#include "Structure.hpp"
#include <iostream>
#include <unordered_set>
using namespace std;
int main()
{
    size_t n, m;
    cin >> n >> m;
    steiner_tree::UndirectedGraph<double> G;
    G.setVertexNum(n);
    while (m--)
    {
        size_t v1, v2, cost;
        cin >> v1 >> v2 >> cost;
        G.addEdge(v1, v2, cost);
    }
    size_t terminalNum;
    cin >> terminalNum;
    unordered_set<size_t> terminals;
    while (terminalNum--)
    {
        size_t terminal;
        cin >> terminal;
        terminals.emplace(terminal);
    }
    steiner_tree::Solver<double> solver(G);
    auto res = solver.solve(terminals);
    cout << res->size() << '\n';
    for (auto eid : *res)
    {
        cout << G.getEdge(eid).v1 << ' ' << G.getEdge(eid).v2 << ' ' << G.getEdge(eid).cost << '\n';
    }
    return 0;
}
```

### example input 1
```
15 34
0 3 8
0 4 2
0 5 3
0 2 3
0 1 6
1 2 2
1 7 3
2 5 1
2 7 3
3 6 6
4 6 2
4 8 2
4 5 1
5 8 7
5 10 3
5 9 2
5 12 4
5 7 4
6 8 1
6 11 3
7 8 5
7 12 2
7 14 2
8 11 1
8 13 2
8 10 1
9 10 1
9 12 1
10 13 1
10 14 3
11 13 1
12 13 3
12 14 1
13 14 4
9
0 1 4 6 8 10 12 13 14
```
### example output 1
```
11
6 8 1
8 10 1
10 13 1
12 14 1
0 4 2
4 6 2
9 10 1
9 12 1
2 5 1
4 5 1
1 2 2
```