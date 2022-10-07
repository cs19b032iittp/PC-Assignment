#include <iostream>
#include <omp.h>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <utility>
using namespace std;

struct hash_pair
{
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2> &p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);

        if (hash1 != hash2)
        {
            return hash1 ^ hash2;
        }

        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};

int main()
{
    int n = 11;
    int m = 2 * (n - 1);
    int r = 8;
    vector<vector<int>> adjacencyList =
        {
            {2, 10, 11, 8},
            {1},
            {4},
            {7, 3, 8},
            {6, 9, 8},
            {5},
            {4},
            {4, 1, 5},
            {5},
            {1},
            {1}};
    vector<int> degrees = {4, 1, 1, 3, 3, 1, 1, 3, 1, 1, 1};
    int count = 0;
    unordered_map<pair<int, int>, int, hash_pair> hash_map;
    vector<pair<int, int>> edges(m, {-1, -1});

    omp_set_num_threads(n);
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        int v = i + 1;
        int d = degrees[i];
        for (int j = 0; j < d; j++)
        {

            #pragma omp critical
            {
                pair p1 = {adjacencyList[i][j], v};
                edges[count] = p1;
                hash_map[p1] = count++;
            }
        }
    }

    pair startingEdge = {r, adjacencyList[r - 1][0]};
    vector<int> successor(m, -1), predecessor(m, -1);

    // step1: update predecessor and successors
    #pragma omp parallel for shared(count)
    for (int i = 0; i < n; i++)
    {
        int v = i + 1;
        int d = degrees[i];

        #pragma omp parallel for
        for (int j = 0; j < d; j++)
        {
            pair p1 = {adjacencyList[i][j], v};
            pair p2 = {v, adjacencyList[i][(j + 1) % (d)]};

            successor[hash_map[p1]] = hash_map[p2];
            predecessor[hash_map[p2]] = hash_map[p1];
        }
    }

    // step2: break the cycle
    int d = degrees[r - 1];
    int u = adjacencyList[r - 1][d - 1];
    successor[hash_map[{u, r}]] = -1;

    u = adjacencyList[r - 1][0];
    predecessor[hash_map[{r, u}]] = hash_map[{r, u}];

    // step3: label the edges(list ranking on predecessors)
    omp_set_num_threads(m);
    int limit = (int)(log2(m) + 1);
    vector<int> dist(m, -1);

    // initialization of dist[]
    #pragma omp parallel for
    for (int i = 0; i < m; i++)
    {
        if (edges[i] == startingEdge)
        {
            dist[i] = 0;
        }
        else
        {
            dist[i] = 1;
        }
    }

    for (int j = 0; j < limit; j++)
    {
        vector<int> dist2(m, -1);
        vector<int> predecessor2(m, -1);

        #pragma omp parallel shared(dist2, predecessor2)
        {
            #pragma omp for
            for (int i = 0; i < m; i++)
            {
                if (edges[i] != startingEdge)
                {
                    dist2[i] = dist[i] + dist[predecessor[i]];
                    predecessor2[i] = predecessor[predecessor[i]];
                }
            }

            #pragma omp for
            for (int i = 0; i < m; i++)
            {
                if (edges[i] != startingEdge)
                {
                    predecessor[i] = predecessor2[i];
                    dist[i] = dist2[i];
                }
            }
        }
    }

    // step4: Identify the parents
    unordered_map<int, int> parent;
    #pragma omp parallel for
    for (int i = 0; i < m; i++)
    {

        pair edge = edges[i];
        int u = edge.first;
        int v = edge.second;

        if (dist[hash_map[{u, v}]] > dist[hash_map[{v, u}]])
        {
            parent[u] = v;
        }
    }

    vector<int> eulerTour(m, -1);
    for (int i = 0; i < m; i++)
    {
        eulerTour[dist[i]] = hash_map[edges[i]];
    }

    // step5: Assigning the weights
    vector<int> weight(m, 1);
    #pragma omp parallel
    for (int i = 0; i < m; i++)
    {
        pair<int, int> edge = edges[eulerTour[i]];
        int u = edge.first;
        int v = edge.second;

        if (parent[u] == v)
        {
            weight[i] = 0;
        }
    }

    // step6: prefix sum
    vector<int> weight2(m, -1);

    #pragma omp parallel for
    for (int i = 0; i < m; i++)
    {
        weight2[i] = weight[i];
    }

    for (int i = 0; i < limit; i++)
    {
        #pragma omp parallel for
        for (int j = 0; j < m; j++)
        {
            if (j < (int)pow(2, i))
            {
                weight2[j] = weight[j];
            }
            else
            {
                weight2[j] = weight[j] + weight[j - (int)pow(2, i)];
            }
        }

        #pragma omp parallel for
        for (int j = 0; j < m; j++)
        {
            weight[j] = weight2[j];
        }
    }

    vector<int> pre(n, 0);

    #pragma omp parallel 
    {
        #pragma omp for
        for (int i = 0; i < m; i++)
        {
            int u, v;
            u = edges[eulerTour[i]].first;
            v = edges[eulerTour[i]].second;

            if (parent[v] == u)
            {
                pre[v - 1] = weight[i] + 1;
            }
        }
        
    }
    pre[r - 1] = 1;

    // printing preorder traversal
    cout << "Preorder traversal:\n";
    for (int i = 0; i < n; i++)
    {

        cout << (i + 1) << " => " << (pre[i]) << endl;
    }
}