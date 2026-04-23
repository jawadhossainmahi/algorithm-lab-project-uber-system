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
//  CONSTANTS
// ─────────────────────────────────────────────
const int INF = INT_MAX;
const string CUSTOMERS_FILE  = "customers.txt";
const string DRIVERS_FILE    = "drivers.txt";
const string RIDES_FILE      = "rides.txt";
const string LOCATIONS_FILE  = "locations.txt";
const string ROADS_FILE      = "roads.txt";

// ─────────────────────────────────────────────
//  DATA STRUCTURES
// ─────────────────────────────────────────────
struct Location {
    int id;
    string name;
    double x, y; // coordinates for distance calc
};

struct Road {
    int from, to, weight; // weight = distance in km
};

struct Customer {
    int id;
    string name;
    string phone;
    string email;
    int locationId;
    double totalSpent;
    vector<int> rideHistory; // ride IDs
};

struct Driver {
    int id;
    string name;
    string phone;
    string carModel;
    string licensePlate;
    int locationId;
    bool isAvailable;
    double rating;
    int totalRides;
    double totalEarned;
    vector<int> rideHistory;
};

struct Ride {
    int id;
    int customerId;
    int driverId;
    int pickupLocationId;
    int dropLocationId;
    double fare;
    string status; // PENDING, ACTIVE, COMPLETED, CANCELLED
    string timestamp;
    double distance;
    int duration; // minutes (LIS used on duration sequences)
};

// ─────────────────────────────────────────────
//  GRAPH (for all graph algorithms)
// ─────────────────────────────────────────────
class Graph {
public:
    int V;
    vector<vector<pair<int,int>>> adj; // adj[u] = {v, weight}
    vector<Road> roads;

    Graph(int v) : V(v), adj(v) {}

    void addEdge(int u, int v, int w) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
        roads.push_back({u, v, w});
    }

    // ── DIJKSTRA ──────────────────────────────
    // Returns shortest distances from src to all nodes
    // Used: find nearest available driver
    vector<int> dijkstra(int src, vector<int>& parent) {
        vector<int> dist(V, INF);
        parent.assign(V, -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
        dist[src] = 0;
        pq.push({0, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto [v, w] : adj[u]) {
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }

    // ── BELLMAN-FORD ──────────────────────────
    // Handles negative weights (traffic penalties)
    // Returns false if negative cycle exists
    bool bellmanFord(int src, vector<int>& dist, vector<int>& parent) {
        dist.assign(V, INF);
        parent.assign(V, -1);
        dist[src] = 0;
        for (int i = 0; i < V - 1; i++) {
            for (auto& r : roads) {
                if (dist[r.from] != INF && dist[r.from] + r.weight < dist[r.to]) {
                    dist[r.to] = dist[r.from] + r.weight;
                    parent[r.to] = r.from;
                }
                if (dist[r.to] != INF && dist[r.to] + r.weight < dist[r.from]) {
                    dist[r.from] = dist[r.to] + r.weight;
                    parent[r.from] = r.to;
                }
            }
        }
        // Check negative cycle
        for (auto& r : roads) {
            if (dist[r.from] != INF && dist[r.from] + r.weight < dist[r.to])
                return false;
        }
        return true;
    }

    // ── PRIM'S MST ────────────────────────────
    // Build minimum spanning road network
    // Used: show optimal city road coverage
    vector<pair<int,int>> primMST(int& totalCost) {
        vector<int> key(V, INF), parent(V, -1);
        vector<bool> inMST(V, false);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
        key[0] = 0;
        pq.push({0, 0});
        totalCost = 0;
        vector<pair<int,int>> mstEdges;
        while (!pq.empty()) {
            auto [k, u] = pq.top(); pq.pop();
            if (inMST[u]) continue;
            inMST[u] = true;
            totalCost += k;
            if (parent[u] != -1) mstEdges.push_back({parent[u], u});
            for (auto [v, w] : adj[u]) {
                if (!inMST[v] && w < key[v]) {
                    key[v] = w;
                    parent[v] = u;
                    pq.push({w, v});
                }
            }
        }
        return mstEdges;
    }

    // ── KRUSKAL'S MST ─────────────────────────
    // Union-Find based MST
    // Used: alternative MST / comparison with Prim's
    vector<Road> kruskalMST(int& totalCost) {
        vector<Road> sorted_roads = roads;
        sort(sorted_roads.begin(), sorted_roads.end(),
             [](const Road& a, const Road& b){ return a.weight < b.weight; });
        vector<int> parent(V), rank_(V, 0);
        iota(parent.begin(), parent.end(), 0);
        function<int(int)> find = [&](int x) {
            return parent[x] == x ? x : parent[x] = find(parent[x]);
        };
        auto unite = [&](int a, int b) {
            a = find(a); b = find(b);
            if (a == b) return false;
            if (rank_[a] < rank_[b]) swap(a, b);
            parent[b] = a;
            if (rank_[a] == rank_[b]) rank_[a]++;
            return true;
        };
        vector<Road> mst;
        totalCost = 0;
        for (auto& r : sorted_roads) {
            if (unite(r.from, r.to)) {
                mst.push_back(r);
                totalCost += r.weight;
            }
        }
        return mst;
    }

    // Reconstruct path from parent array
    vector<int> getPath(vector<int>& parent, int dest) {
        vector<int> path;
        for (int at = dest; at != -1; at = parent[at])
            path.push_back(at);
        reverse(path.begin(), path.end());
        return path;
    }
};

// ─────────────────────────────────────────────
//  DP ALGORITHMS (standalone)
// ─────────────────────────────────────────────

// ── LCS ───────────────────────────────────────
// Longest Common Subsequence of two ride history sequences
// Used: find common routes between two customers
int lcs(vector<int>& A, vector<int>& B, vector<vector<int>>& dp) {
    int m = A.size(), n = B.size();
    dp.assign(m+1, vector<int>(n+1, 0));
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            dp[i][j] = (A[i-1] == B[j-1]) ? dp[i-1][j-1]+1
                                            : max(dp[i-1][j], dp[i][j-1]);
    return dp[m][n];
}

vector<int> lcsSequence(vector<int>& A, vector<int>& B, vector<vector<int>>& dp) {
    int i = A.size(), j = B.size();
    vector<int> seq;
    while (i > 0 && j > 0) {
        if (A[i-1] == B[j-1]) { seq.push_back(A[i-1]); i--; j--; }
        else if (dp[i-1][j] > dp[i][j-1]) i--;
        else j--;
    }
    reverse(seq.begin(), seq.end());
    return seq;
}

// ── LIS ───────────────────────────────────────
// Longest Increasing Subsequence of ride durations
// Used: analyze driver's improving ride efficiency over time
int lis(vector<int>& durations, vector<int>& result) {
    int n = durations.size();
    if (n == 0) return 0;
    vector<int> dp(n, 1), parent(n, -1), tails;
    for (int i = 0; i < n; i++) {
        auto it = lower_bound(tails.begin(), tails.end(), durations[i]);
        int pos = it - tails.begin();
        if (it == tails.end()) tails.push_back(durations[i]);
        else tails[pos] = durations[i];
        dp[i] = pos + 1;
        parent[i] = (pos > 0) ? -1 : -1; // simplified
    }
    // Reconstruct
    int maxLen = *max_element(dp.begin(), dp.end());
    int idx = max_element(dp.begin(), dp.end()) - dp.begin();
    result.clear();
    for (int i = idx; i >= 0 && (int)result.size() < maxLen; i--) {
        if (dp[i] == (int)(maxLen - result.size()))
            result.push_back(durations[i]);
    }
    reverse(result.begin(), result.end());
    return maxLen;
}