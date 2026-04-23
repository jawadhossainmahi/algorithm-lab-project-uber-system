# Uber DSA System — C++ Terminal Project

A complete terminal-based Uber ride management system built with C++17,
demonstrating 6 key DSA algorithms on a real-world problem.

---

## Project Files

| File            | Purpose                                      |
|-----------------|----------------------------------------------|
| `main.cpp`      | Entry point, all menu navigation             |
| `uber.h`        | Data structures + all 6 algorithms           |
| `ubersystem.h`  | Core business logic, algorithm integration   |
| `filemanager.h` | Persistent storage via .txt files            |
| `ui.h`          | Terminal UI helpers (colors, prompts, tables)|

---

## Algorithms Used & Where

| Algorithm      | Location in Code         | Use Case in Uber System                          |
|----------------|--------------------------|--------------------------------------------------|
| **Dijkstra**   | `Graph::dijkstra()`      | Find nearest available driver; shortest route    |
| **Bellman-Ford**| `Graph::bellmanFord()`  | Traffic-penalty routing (handles negative edges) |
| **Prim's MST** | `Graph::primMST()`       | Minimum road network to connect all city zones   |
| **Kruskal's**  | `Graph::kruskalMST()`    | Union-Find based MST; compared against Prim's    |
| **LCS**        | `lcs()` / `lcsSequence()`| Find common routes between two customers         |
| **LIS**        | `lis()`                  | Analyze driver efficiency trend over time        |

---

## How to Compile & Run

```bash
g++ -std=c++17 -O2 -o uber main.cpp
./uber
```

---

## Data Files (auto-created on first run)

| File             | Stores                          |
|------------------|---------------------------------|
| `locations.txt`  | City zones with coordinates     |
| `roads.txt`      | Road edges with distances (km)  |
| `customers.txt`  | Customer profiles & ride history|
| `drivers.txt`    | Driver profiles & stats         |
| `rides.txt`      | All completed/active rides      |

---

## Main Features

1. **Book a Ride** — Dijkstra finds nearest available driver, computes optimal route
2. **Customer Management** — Register, view, list customers
3. **Driver Management** — Register, toggle availability, update location
4. **City Map** — View 10 pre-seeded city zones, add new roads
5. **Ride History** — View all rides with fare, distance, duration
6. **Algorithm Analysis** — Interactive showcase of all 6 algorithms
7. **System Statistics** — Revenue, top driver, algorithm summary

---

## City Map (Pre-seeded)

```
0: Airport       1: City Center    2: North Park
3: East Market   4: West Town      5: South Bay
6: Tech Hub      7: University     8: Hospital
9: Shopping Mall
```

Roads connect these zones with weighted edges (km).

---

## Sample Workflow

```
1. Register a Customer (Menu 2 → 1)
2. Register a Driver   (Menu 3 → 1)
3. Book a Ride         (Menu 1)
   → Dijkstra runs automatically to find nearest driver
   → Shortest path displayed with full route
4. Try Algorithm Tools (Menu 6)
   → Dijkstra, Bellman-Ford, Prim's, Kruskal's, LCS, LIS
```

---

## Algorithm Complexity

| Algorithm     | Time         | Space  |
|---------------|--------------|--------|
| Dijkstra      | O((V+E)logV) | O(V)   |
| Bellman-Ford  | O(V×E)       | O(V)   |
| Prim's MST    | O((V+E)logV) | O(V)   |
| Kruskal's MST | O(E log E)   | O(V)   |
| LCS           | O(m×n)       | O(m×n) |
| LIS           | O(n log n)   | O(n)   |# algorithm-lab-project-uber-system
