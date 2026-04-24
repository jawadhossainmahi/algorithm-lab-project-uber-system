#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <climits>
#include <cmath>
#include <iomanip>
#include <ctime>
#include <functional>
#include <numeric>
using namespace std;

// ─────────────────────────────────────────────
// CONSTANTS
// ─────────────────────────────────────────────
const double INF_D = 1e18;
const int INF = INT_MAX;

const string CUSTOMERS_FILE = "customers.txt";
const string DRIVERS_FILE = "drivers.txt";
const string RIDES_FILE = "rides.txt";
const string LOCATIONS_FILE = "locations.txt";
const string ROADS_FILE = "roads.txt";
const string CONFIG_FILE = "config.txt";

// ─────────────────────────────────────────────
// DATA STRUCTURES
// ─────────────────────────────────────────────

struct Location
{
    int id;
    string name;
    double x, y; // 2D coordinates used to calculate real distance
};

// Road stores TWO separate values:
//   distance = Euclidean km between the two nodes (always positive, used for fare & Dijkstra)
//   weight   = traffic penalty value (can be negative = shortcut; used for Bellman-Ford)
struct Road
{
    int from, to;
    double distance; // computed from x,y  (>= 0 always)
    int weight;      // traffic weight     (can be negative)
};

struct Customer
{
    int id;
    string name, phone, email;
    int locationId;
    double totalSpent;
    vector<int> rideHistory;
};

struct Driver
{
    int id;
    string name, phone, carModel, licensePlate;
    int locationId;
    bool isAvailable;
    double rating;
    int totalRides;
    double totalEarned;
    vector<int> rideHistory;
};

struct Ride
{
    int id;
    int customerId, driverId;
    int pickupLocationId, dropLocationId;
    double fare;
    string status;
    string timestamp;
    double distance; // Euclidean km
    int duration;
    string algorithm; // "DIJKSTRA" or "BELLMAN-FORD"
};

// Fare config saved once on first run
struct FareConfig
{
    double baseFare;  // fixed charge per ride
    double perKmRate; // charge per km
    double driverCut; // fraction driver earns  (e.g. 0.75)
};

// ─────────────────────────────────────────────
// GRAPH
// ─────────────────────────────────────────────
class Graph
{
public:
    int V;
    // adj for Dijkstra: {neighbor, distance_km}
    vector<vector<pair<int, double>>> adjDist;
    // full road list for Bellman-Ford (uses weight)
    vector<Road> roads;

    Graph(int v) : V(v), adjDist(v) {}

    void addEdge(const Road &r)
    {
        adjDist[r.from].push_back({r.to, r.distance});
        adjDist[r.to].push_back({r.from, r.distance});
        roads.push_back(r);
    }

    // ── DIJKSTRA ─────────────────────────────────────────────────
    // Uses DISTANCE (Euclidean km, always positive).
    // Returns shortest distances from src to all nodes.
    // parent[] lets us reconstruct the path.
    vector<double> dijkstra(int src, vector<int> &parent)
    {
        vector<double> dist(V, INF_D);
        parent.assign(V, -1);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
        dist[src] = 0;
        pq.push({0.0, src});

        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();
            if (d > dist[u])
                continue;
            for (auto [v, w] : adjDist[u])
            {
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }

    // ── BELLMAN-FORD ──────────────────────────────────────────────
    // Uses WEIGHT (traffic value, CAN be negative).
    // Returns false if a negative cycle is detected.
    // dist[] is in weight units; to get actual km use the path then
    // sum distances along the same edges.
    bool bellmanFord(int src, vector<double> &dist, vector<int> &parent)
    {
        dist.assign(V, INF_D);
        parent.assign(V, -1);
        dist[src] = 0;

        for (int i = 0; i < V - 1; i++)
        {
            bool updated = false;
            for (auto &r : roads)
            {
                // forward direction
                if (dist[r.from] < INF_D && dist[r.from] + r.weight < dist[r.to])
                {
                    dist[r.to] = dist[r.from] + r.weight;
                    parent[r.to] = r.from;
                    updated = true;
                }
                // backward direction (undirected)
                if (dist[r.to] < INF_D && dist[r.to] + r.weight < dist[r.from])
                {
                    dist[r.from] = dist[r.to] + r.weight;
                    parent[r.from] = r.to;
                    updated = true;
                }
            }
            if (!updated)
                break;
        }

        // Negative cycle check
        for (auto &r : roads)
        {
            if (dist[r.from] < INF_D && dist[r.from] + r.weight < dist[r.to])
                return false;
            if (dist[r.to] < INF_D && dist[r.to] + r.weight < dist[r.from])
                return false;
        }
        return true;
    }

    // Reconstruct path from parent array
    vector<int> getPath(vector<int> &parent, int dest)
    {
        vector<int> path;
        for (int at = dest; at != -1; at = parent[at])
            path.push_back(at);
        reverse(path.begin(), path.end());
        return path;
    }

    // Sum Euclidean distance along a path
    double pathDistance(const vector<int> &path, const vector<Location> &locs)
    {
        double total = 0;
        for (int i = 0; i + 1 < (int)path.size(); i++)
        {
            double dx = locs[path[i]].x - locs[path[i + 1]].x;
            double dy = locs[path[i]].y - locs[path[i + 1]].y;
            total += sqrt(dx * dx + dy * dy);
        }
        return total;
    }

    // Sum weight along a path (for Bellman-Ford display)
    int pathWeight(const vector<int> &path)
    {
        int total = 0;
        for (int i = 0; i + 1 < (int)path.size(); i++)
        {
            int u = path[i], v = path[i + 1];
            for (auto &r : roads)
            {
                if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                {
                    total += r.weight;
                    break;
                }
            }
        }
        return total;
    }

    // ── PRIM'S MST ────────────────────────────────────────────────
    vector<pair<int, int>> primMST(double &totalCost)
    {
        vector<double> key(V, INF_D);
        vector<int> parent(V, -1);
        vector<bool> inMST(V, false);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
        key[0] = 0;
        pq.push({0.0, 0});
        totalCost = 0;
        vector<pair<int, int>> mstEdges;

        while (!pq.empty())
        {
            auto [k, u] = pq.top();
            pq.pop();
            if (inMST[u])
                continue;
            inMST[u] = true;
            totalCost += k;
            if (parent[u] != -1)
                mstEdges.push_back({parent[u], u});
            for (auto [v, w] : adjDist[u])
            {
                if (!inMST[v] && w < key[v])
                {
                    key[v] = w;
                    parent[v] = u;
                    pq.push({w, v});
                }
            }
        }
        return mstEdges;
    }

    // ── KRUSKAL'S MST ─────────────────────────────────────────────
    vector<Road> kruskalMST(double &totalCost)
    {
        vector<Road> sorted_roads = roads;
        sort(sorted_roads.begin(), sorted_roads.end(),
             [](const Road &a, const Road &b)
             { return a.distance < b.distance; });

        vector<int> par(V);
        vector<int> rnk(V, 0);
        iota(par.begin(), par.end(), 0);
        function<int(int)> find = [&](int x)
        {
            return par[x] == x ? x : par[x] = find(par[x]);
        };
        auto unite = [&](int a, int b)
        {
            a = find(a);
            b = find(b);
            if (a == b)
                return false;
            if (rnk[a] < rnk[b])
                swap(a, b);
            par[b] = a;
            if (rnk[a] == rnk[b])
                rnk[a]++;
            return true;
        };

        vector<Road> mst;
        totalCost = 0;
        for (auto &r : sorted_roads)
        {
            if (unite(r.from, r.to))
            {
                mst.push_back(r);
                totalCost += r.distance;
            }
        }
        return mst;
    }
};

// ─────────────────────────────────────────────
// DP ALGORITHMS
// ─────────────────────────────────────────────

// LCS — Longest Common Subsequence of ride-location sequences
int lcs(vector<int> &A, vector<int> &B, vector<vector<int>> &dp)
{
    int m = A.size(), n = B.size();
    dp.assign(m + 1, vector<int>(n + 1, 0));
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            dp[i][j] = (A[i - 1] == B[j - 1]) ? dp[i - 1][j - 1] + 1
                                              : max(dp[i - 1][j], dp[i][j - 1]);
    return dp[m][n];
}

vector<int> lcsSequence(vector<int> &A, vector<int> &B, vector<vector<int>> &dp)
{
    int i = A.size(), j = B.size();
    vector<int> seq;
    while (i > 0 && j > 0)
    {
        if (A[i - 1] == B[j - 1])
        {
            seq.push_back(A[i - 1]);
            i--;
            j--;
        }
        else if (dp[i - 1][j] > dp[i][j - 1])
            i--;
        else
            j--;
    }
    reverse(seq.begin(), seq.end());
    return seq;
}

// LIS — Longest Increasing Subsequence of ride durations
int lis(vector<int> &durations, vector<int> &result)
{
    int n = durations.size();
    if (n == 0)
        return 0;

    vector<int> dp(n, 1), par(n, -1), tails, tailIdx;
    for (int i = 0; i < n; i++)
    {
        auto it = lower_bound(tails.begin(), tails.end(), durations[i]);
        int pos = it - tails.begin();
        if (it == tails.end())
        {
            tails.push_back(durations[i]);
            tailIdx.push_back(i);
        }
        else
        {
            tails[pos] = durations[i];
            tailIdx[pos] = i;
        }
        dp[i] = pos + 1;
        if (pos > 0)
            par[i] = tailIdx[pos - 1];
    }

    int maxLen = *max_element(dp.begin(), dp.end());
    int idx = max_element(dp.begin(), dp.end()) - dp.begin();
    result.clear();
    for (int at = idx; at != -1; at = par[at])
        result.push_back(durations[at]);
    reverse(result.begin(), result.end());
    return maxLen;
}