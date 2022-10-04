#include <iostream>
#include <omp.h>
#include <vector>
#include <map>
#include <cmath>
#include <utility>
using namespace std;

int main()
{

    int n = 11; // number of nodes
    omp_set_num_threads(n);
    int m = 2 * (n - 1); // number of edges
    int r = 8;           // root vertex;
    vector<vector<int>> adjacencyList = {
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
    pair startingEdge = {r, adjacencyList[r - 1][0]};
    map<pair<int, int>, pair<int, int>> successor, predecessor;
    map<int, pair<int, int>> eulerTour;

// step1: update predecessor and successors
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        int v = i + 1;
        int d = degrees[i];
        
        #pragma omp parallel for
        for (int j = 0; j < d; j++)
        {

            pair p1 = {adjacencyList[i][j], v};
            pair p2 = {v, adjacencyList[i][(j + 1) % (d)]};

            successor[p1] = p2;
            predecessor[p2] = p1;
        }
    }

    // Step2: break the cycle
    int d = degrees[r - 1];
    int u = adjacencyList[r - 1][d - 1];
    successor[{u, r}] = {-1, -1};

    u = adjacencyList[r - 1][0];
    predecessor[{r, u}] = {-1, -1};

    // Step3: label the edges (list ranking)
    pair edge = startingEdge;
    map<pair<int, int>, int> rank;
    int count = 1;
    for (; true;)
    {
        eulerTour[count] = edge;
        rank[edge] = count++;
        edge = successor[edge];
        if (edge.first == -1)
            break;
    }

    // Step4: Identify parents
    map<int, int> parent;
    
    #pragma omp parallel for
    for (int i = 1; i < m; i++)
    {
        pair edge = eulerTour[i];
        int u = edge.first;
        int v = edge.second;

        if (rank[{u, v}] < rank[{v, u}])
            parent[v] = u;
        else
            parent[u] = v;
    }

    cout << "Rooted tree: \n";
    for (auto const &pt : parent)
    {
        int c = pt.first;
        int p = pt.second;

        cout << p << " is parent of " << c << endl;
    }

    // cout << "Euler Tour: ";
    // for (auto const &e: eulerTour) {
    //     int r = e.first;
    //     pair v = e.second;
    //     cout << " (" << v.first << ", " << v.second << "), ";
    // }
    // cout << "\n";

    // cout << "Successors: ";
    // for (auto const &s: successor) {
    //     pair u = s.first;
    //     pair v = s.second;
    //     cout << "(" << u.first << ", " << u.second << "):  " << "(" << v.first << ", " << v.second << ")  \n";
    // }

    // cout << "\nPredecessors: ";
    // for (auto const &p: predecessor) {
    //     pair u = p.first;
    //     pair v = p.second;
    //     cout << "(" << u.first << ", " << u.second << "):  " << "(" << v.first << ", " << v.second << ")  \n";
    // }

    // cout << "\nrank: ";
    // for (auto const &l: rank) {
    //     pair u = l.first;
    //     int lbl = l.second;
    //     cout << "(" << u.first << ", " << u.second << "): " << lbl << " \n";
    // }
}