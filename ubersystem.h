#pragma once
#include "uber.h"
#include "filemanager.h"
#include "ui.h"

// ─────────────────────────────────────────────
//  UBER SYSTEM ENGINE
// ─────────────────────────────────────────────
class UberSystem
{
    vector<Location> locations;
    vector<Driver> drivers;
    vector<Customer> customers;
    vector<Ride> rides;
    Graph *graph;

    int nextCustomerId = 1;
    int nextDriverId = 1;
    int nextRideId = 1;

public:
    UberSystem() : graph(nullptr)
    {
        loadAll();
        buildGraph();
    }
    ~UberSystem() { delete graph; }

    // ── LOAD & SAVE ────────────────────────────
    void loadAll()
    {
        locations = FileManager::loadLocations();
        drivers = FileManager::loadDrivers();
        customers = FileManager::loadCustomers();
        rides = FileManager::loadRides();

        for (auto &c : customers)
            nextCustomerId = max(nextCustomerId, c.id + 1);
        for (auto &d : drivers)
            nextDriverId = max(nextDriverId, d.id + 1);
        for (auto &r : rides)
            nextRideId = max(nextRideId, r.id + 1);

        // Seed default city map if empty
        if (locations.empty())
            seedCityMap();
    }

    void saveAll()
    {
        FileManager::saveLocations(locations);
        FileManager::saveDrivers(drivers);
        FileManager::saveCustomers(customers);
        FileManager::saveRides(rides);
    }

    void seedCityMap()
    {
        // 10 city zones
        locations = {
            {0, "Airport", 0, 0},
            {1, "City Center", 3, 4},
            {2, "North Park", 1, 8},
            {3, "East Market", 7, 3},
            {4, "West Town", -4, 5},
            {5, "South Bay", 2, -5},
            {6, "Tech Hub", 9, 9},
            {7, "University", -2, 9},
            {8, "Hospital", 5, 1},
            {9, "Shopping Mall", 6, 7},
        };
        FileManager::saveLocations(locations);

        // Roads (undirected, km)
        vector<Road> roads = {
            {0, 1, 7}, {0, 5, 8}, {0, 8, 5}, {1, 2, 6}, {1, 3, 5}, {1, 4, 7}, {1, 8, 4}, {1, 9, 6}, {2, 4, 5}, {2, 7, 4}, {3, 6, 5}, {3, 8, 3}, {3, 9, 4}, {4, 7, 4}, {5, 8, 6}, {6, 9, 3}, {7, 2, 4}, {9, 6, 3}};
        FileManager::saveRoads(roads);
    }

    void buildGraph()
    {
        delete graph;
        graph = new Graph((int)locations.size());
        auto roads = FileManager::loadRoads();
        for (auto &r : roads)
            graph->addEdge(r.from, r.to, r.weight);
    }

    // ── HELPERS ────────────────────────────────
    Location *findLocation(int id)
    {
        for (auto &l : locations)
            if (l.id == id)
                return &l;
        return nullptr;
    }
    Customer *findCustomer(int id)
    {
        for (auto &c : customers)
            if (c.id == id)
                return &c;
        return nullptr;
    }
    Driver *findDriver(int id)
    {
        for (auto &d : drivers)
            if (d.id == id)
                return &d;
        return nullptr;
    }
    Ride *findRide(int id)
    {
        for (auto &r : rides)
            if (r.id == id)
                return &r;
        return nullptr;
    }

    string locationName(int id)
    {
        auto *l = findLocation(id);
        return l ? l->name : "Unknown";
    }

    // ══════════════════════════════════════════
    //  CUSTOMER MANAGEMENT
    // ══════════════════════════════════════════
    void registerCustomer()
    {
        UI::subHeader("Register New Customer");
        Customer c;
        c.id = nextCustomerId++;
        c.name = UI::getString("Full Name");
        c.phone = UI::getString("Phone Number");
        c.email = UI::getString("Email");

        showLocations();
        c.locationId = UI::getInt("Your Location ID", 0, (int)locations.size() - 1);
        c.totalSpent = 0;

        customers.push_back(c);
        FileManager::saveCustomers(customers);
        UI::success("Customer registered! Your ID: " + to_string(c.id));
    }

    void viewCustomer()
    {
        int id = UI::getInt("Enter Customer ID");
        auto *c = findCustomer(id);
        if (!c)
        {
            UI::error("Customer not found.");
            return;
        }

        UI::subHeader("Customer Profile");
        UI::tableRow("ID", to_string(c->id));
        UI::tableRow("Name", c->name);
        UI::tableRow("Phone", c->phone);
        UI::tableRow("Email", c->email);
        UI::tableRow("Location", locationName(c->locationId));
        UI::tableRow("Total Rides", to_string(c->rideHistory.size()));
        UI::tableRow("Total Spent", "$" + to_string(c->totalSpent).substr(0, 6));
    }

    void listCustomers()
    {
        UI::subHeader("All Customers");
        cout << "  " << left << setw(5) << "ID"
             << setw(20) << "Name"
             << setw(15) << "Phone"
             << setw(18) << "Location"
             << setw(8) << "Rides" << "\n";
        UI::line();
        for (auto &c : customers)
        {
            cout << "  " << left << setw(5) << c.id
                 << setw(20) << c.name
                 << setw(15) << c.phone
                 << setw(18) << locationName(c.locationId)
                 << setw(8) << c.rideHistory.size() << "\n";
        }
    }

    // ══════════════════════════════════════════
    //  DRIVER MANAGEMENT
    // ══════════════════════════════════════════
    void registerDriver()
    {
        UI::subHeader("Register New Driver");
        Driver d;
        d.id = nextDriverId++;
        d.name = UI::getString("Full Name");
        d.phone = UI::getString("Phone Number");
        d.carModel = UI::getString("Car Model");
        d.licensePlate = UI::getString("License Plate");
        d.rating = 5.0;
        d.totalRides = 0;
        d.totalEarned = 0;
        d.isAvailable = true;

        showLocations();
        d.locationId = UI::getInt("Current Location ID", 0, (int)locations.size() - 1);

        drivers.push_back(d);
        FileManager::saveDrivers(drivers);
        UI::success("Driver registered! Your ID: " + to_string(d.id));
    }

    void viewDriver()
    {
        int id = UI::getInt("Enter Driver ID");
        auto *d = findDriver(id);
        if (!d)
        {
            UI::error("Driver not found.");
            return;
        }

        UI::subHeader("Driver Profile");
        UI::tableRow("ID", to_string(d->id));
        UI::tableRow("Name", d->name);
        UI::tableRow("Phone", d->phone);
        UI::tableRow("Car", d->carModel);
        UI::tableRow("Plate", d->licensePlate);
        UI::tableRow("Location", locationName(d->locationId));
        UI::tableRow("Status", d->isAvailable ? "✔ Available" : "✘ On Ride");
        UI::tableRow("Rating", UI::stars(d->rating));
        UI::tableRow("Total Rides", to_string(d->totalRides));
        UI::tableRow("Earned", "$" + to_string(d->totalEarned).substr(0, 7));
    }

    void listDrivers()
    {
        UI::subHeader("All Drivers");
        cout << "  " << left << setw(5) << "ID"
             << setw(20) << "Name"
             << setw(15) << "Car"
             << setw(18) << "Location"
             << setw(12) << "Status"
             << setw(8) << "Rating" << "\n";
        UI::line();
        for (auto &d : drivers)
        {
            string status = d.isAvailable ? "\033[32mAvail\033[0m" : "\033[31mBusy \033[0m";
            cout << "  " << left << setw(5) << d.id
                 << setw(20) << d.name
                 << setw(15) << d.carModel
                 << setw(18) << locationName(d.locationId)
                 << status << "       "
                 << fixed << setprecision(1) << d.rating << "\n";
        }
    }

    void toggleDriverAvailability()
    {
        int id = UI::getInt("Enter Driver ID");
        auto *d = findDriver(id);
        if (!d)
        {
            UI::error("Driver not found.");
            return;
        }
        d->isAvailable = !d->isAvailable;
        FileManager::saveDrivers(drivers);
        UI::success("Driver " + d->name + " is now " + (d->isAvailable ? "AVAILABLE" : "UNAVAILABLE"));
    }

    // ══════════════════════════════════════════
    //  LOCATION MAP
    // ══════════════════════════════════════════
    void showLocations()
    {
        UI::subHeader("City Locations");
        cout << "  " << left << setw(5) << "ID" << setw(20) << "Name"
             << setw(10) << "X" << setw(10) << "Y" << "\n";
        UI::line();
        for (auto &l : locations)
            cout << "  " << left << setw(5) << l.id << setw(20) << l.name
                 << setw(10) << l.x << setw(10) << l.y << "\n";
    }

    // Add new road to the city graph
    void addRoad()
    {
        UI::subHeader("Add New Road");
        showLocations();
        int from = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int to = UI::getInt("To Location ID", 0, (int)locations.size() - 1);
        int weight = UI::getInt("Distance (km)", 1, 999);

        auto roads = FileManager::loadRoads();
        roads.push_back({from, to, weight});
        FileManager::saveRoads(roads);
        graph->addEdge(from, to, weight);
        UI::success("Road added: " + locationName(from) + " ↔ " + locationName(to) + " (" + to_string(weight) + " km)");
    }

    // ══════════════════════════════════════════
    //  BOOK A RIDE  (core feature)
    //  Uses: DIJKSTRA to find nearest driver
    // ══════════════════════════════════════════
    void bookRide()
    {
        UI::subHeader("Book a Ride");

        int custId = UI::getInt("Your Customer ID");
        auto *cust = findCustomer(custId);
        if (!cust)
        {
            UI::error("Customer not found.");
            return;
        }

        showLocations();
        int dropId = UI::getInt("Drop-off Location ID", 0, (int)locations.size() - 1);
        if (dropId == cust->locationId)
        {
            UI::warn("Drop-off same as pickup!");
            return;
        }

        // ── DIJKSTRA: shortest path from customer to all nodes ──
        cout << "\n"
             << UI::CYAN << "  [DIJKSTRA] Finding nearest available driver..." << UI::RESET << "\n";
        vector<int> parentCustomer;
        vector<int> distFromCustomer = graph->dijkstra(cust->locationId, parentCustomer);

        // Find best available driver
        int bestDriverId = -1, bestDist = INF;
        vector<int> bestDriverParent;
        for (auto &d : drivers)
        {
            if (!d.isAvailable)
                continue;
            int dist = distFromCustomer[d.locationId];
            if (dist < bestDist)
            {
                bestDist = dist;
                bestDriverId = d.id;
            }
        }

        if (bestDriverId == -1)
        {
            UI::error("No available drivers right now. Try again later.");
            return;
        }

        auto *driver = findDriver(bestDriverId);

        // ── DIJKSTRA: shortest path customer → drop ──
        vector<int> parentDrop;
        vector<int> distToDrop = graph->dijkstra(cust->locationId, parentDrop);
        int rideDist = distToDrop[dropId];
        vector<int> ridePath = graph->getPath(parentDrop, dropId);

        // Display ride details
        UI::subHeader("Ride Details");
        UI::tableRow("Customer", cust->name);
        UI::tableRow("Driver", driver->name + " (" + driver->carModel + ")");
        UI::tableRow("Driver Loc", locationName(driver->locationId));
        UI::tableRow("Driver ETA", to_string(bestDist) + " km away");
        UI::tableRow("Pickup", locationName(cust->locationId));
        UI::tableRow("Drop-off", locationName(dropId));
        UI::tableRow("Distance", to_string(rideDist) + " km");

        // Show route
        cout << "\n  " << UI::BOLD << "Route: " << UI::RESET;
        for (int i = 0; i < (int)ridePath.size(); i++)
        {
            cout << UI::CYAN << locationName(ridePath[i]) << UI::RESET;
            if (i + 1 < (int)ridePath.size())
                cout << " → ";
        }
        cout << "\n";

        // Fare calculation
        double fare = 2.0 + rideDist * 1.5;         // base + per km
        int duration = rideDist * 3 + bestDist * 2; // minutes estimate
        UI::tableRow("Estimated Fare", "$" + to_string(fare).substr(0, 5));
        UI::tableRow("Est. Duration", to_string(duration) + " min");

        string confirm = UI::getString("Confirm booking? (y/n)");
        if (confirm != "y" && confirm != "Y")
        {
            UI::warn("Booking cancelled.");
            return;
        }

        // Create ride
        Ride ride;
        ride.id = nextRideId++;
        ride.customerId = custId;
        ride.driverId = bestDriverId;
        ride.pickupLocationId = cust->locationId;
        ride.dropLocationId = dropId;
        ride.fare = fare;
        ride.status = "COMPLETED";
        ride.timestamp = FileManager::currentTimestamp();
        ride.distance = rideDist;
        ride.duration = duration;

        // Update customer
        cust->rideHistory.push_back(ride.id);
        cust->totalSpent += fare;
        cust->locationId = dropId;

        // Update driver
        driver->isAvailable = false;
        driver->rideHistory.push_back(ride.id);
        driver->totalRides++;
        driver->totalEarned += fare * 0.75;
        driver->locationId = dropId;

        // Rate driver
        int stars = UI::getInt("Rate your driver (1-5)", 1, 5);
        driver->rating = (driver->rating * (driver->totalRides - 1) + stars) / driver->totalRides;
        driver->isAvailable = true;

        rides.push_back(ride);
        saveAll();

        UI::success("Ride completed! Fare: $" + to_string(fare).substr(0, 5));
        UI::success("Thank you, " + cust->name + "!");
    }

    // ══════════════════════════════════════════
    //  BELLMAN-FORD: Route with traffic penalties
    // ══════════════════════════════════════════
    void bellmanFordRoute()
    {
        UI::subHeader("Bellman-Ford: Traffic-Aware Shortest Route");
        UI::info("Bellman-Ford handles negative weights (traffic discounts/delays)");

        showLocations();
        int src = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int dest = UI::getInt("To Location ID", 0, (int)locations.size() - 1);

        vector<int> dist, parent;
        bool ok = graph->bellmanFord(src, dist, parent);

        if (!ok)
        {
            UI::error("Negative cycle detected in road network!");
            return;
        }

        UI::subHeader("Results");
        if (dist[dest] == INF)
        {
            UI::warn("No path found from " + locationName(src) + " to " + locationName(dest));
        }
        else
        {
            vector<int> path = graph->getPath(parent, dest);
            UI::tableRow("From", locationName(src));
            UI::tableRow("To", locationName(dest));
            UI::tableRow("Distance", to_string(dist[dest]) + " km");
            cout << "\n  " << UI::BOLD << "Route: " << UI::RESET;
            for (int i = 0; i < (int)path.size(); i++)
            {
                cout << UI::YELLOW << locationName(path[i]) << UI::RESET;
                if (i + 1 < (int)path.size())
                    cout << " → ";
            }
            cout << "\n";

            // Show all distances
            cout << "\n  " << UI::BOLD << "Distances from " << locationName(src) << ":\n"
                 << UI::RESET;
            for (auto &l : locations)
            {
                string d = (dist[l.id] == INF) ? "∞" : to_string(dist[l.id]) + " km";
                cout << "    " << left << setw(20) << l.name << d << "\n";
            }
        }
    }

    // ══════════════════════════════════════════
    //  PRIM'S MST: Optimal Road Network
    // ══════════════════════════════════════════
    void primsMST()
    {
        UI::subHeader("Prim's Algorithm: Minimum Spanning Road Network");
        UI::info("Shows the minimum roads needed to connect all city zones");

        int totalCost;
        auto mstEdges = graph->primMST(totalCost);

        cout << "\n  " << UI::BOLD << "MST Edges (Prim's):\n"
             << UI::RESET;
        cout << "  " << left << setw(20) << "From" << setw(20) << "To" << "Weight\n";
        UI::line();

        auto roads = FileManager::loadRoads();
        for (auto &[u, v] : mstEdges)
        {
            int w = 0;
            for (auto &r : roads)
                if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                    w = r.weight;
            cout << "  " << left << setw(20) << locationName(u)
                 << setw(20) << locationName(v) << w << " km\n";
        }
        UI::line();
        UI::info("Total MST cost: " + to_string(totalCost) + " km");
        UI::info("Minimum roads to connect all " + to_string(locations.size()) + " city zones.");
    }

    // ══════════════════════════════════════════
    //  KRUSKAL'S MST
    // ══════════════════════════════════════════
    void kruskalMST()
    {
        UI::subHeader("Kruskal's Algorithm: Minimum Spanning Road Network");
        UI::info("Union-Find based MST — results should match Prim's");

        int totalCost;
        auto mst = graph->kruskalMST(totalCost);

        cout << "\n  " << UI::BOLD << "MST Edges (Kruskal's):\n"
             << UI::RESET;
        cout << "  " << left << setw(20) << "From" << setw(20) << "To" << "Weight\n";
        UI::line();
        for (auto &r : mst)
        {
            cout << "  " << left << setw(20) << locationName(r.from)
                 << setw(20) << locationName(r.to) << r.weight << " km\n";
        }
        UI::line();
        UI::info("Total MST cost: " + to_string(totalCost) + " km");
    }

    // ══════════════════════════════════════════
    //  LCS: Common Route Analysis
    // ══════════════════════════════════════════
    void lcsAnalysis()
    {
        UI::subHeader("LCS: Common Routes Between Two Customers");
        UI::info("Finds longest common subsequence of location IDs visited");

        int id1 = UI::getInt("Customer 1 ID");
        int id2 = UI::getInt("Customer 2 ID");

        auto *c1 = findCustomer(id1);
        auto *c2 = findCustomer(id2);
        if (!c1 || !c2)
        {
            UI::error("One or both customers not found.");
            return;
        }

        // Build location visit sequences from ride histories
        auto getLocSeq = [&](Customer *c)
        {
            vector<int> seq;
            for (int rid : c->rideHistory)
            {
                auto *r = findRide(rid);
                if (r)
                {
                    seq.push_back(r->pickupLocationId);
                    seq.push_back(r->dropLocationId);
                }
            }
            return seq;
        };

        vector<int> seq1 = getLocSeq(c1);
        vector<int> seq2 = getLocSeq(c2);

        if (seq1.empty() || seq2.empty())
        {
            UI::warn("One or both customers have no ride history.");
            return;
        }

        vector<vector<int>> dp;
        int len = lcs(seq1, seq2, dp);
        vector<int> common = lcsSequence(seq1, seq2, dp);

        UI::tableRow(c1->name + " route", "");
        for (int id : seq1)
            cout << "    " << locationName(id) << "\n";
        UI::tableRow(c2->name + " route", "");
        for (int id : seq2)
            cout << "    " << locationName(id) << "\n";

        cout << "\n  " << UI::BOLD << UI::GREEN << "Common Route (LCS length=" << len << "):\n"
             << UI::RESET;
        for (int id : common)
            cout << "    " << UI::CYAN << locationName(id) << UI::RESET << "\n";

        if (common.empty())
            UI::info("No common routes found.");
        else
            UI::info("These customers frequently travel through common zones — carpooling opportunity!");
    }

    // ══════════════════════════════════════════
    //  LIS: Driver Efficiency Trend
    // ══════════════════════════════════════════
    void lisAnalysis()
    {
        UI::subHeader("LIS: Driver Efficiency Analysis");
        UI::info("Finds longest increasing subsequence of ride durations");
        UI::info("A decreasing trend = driver getting faster (more efficient)");

        int drvId = UI::getInt("Driver ID");
        auto *d = findDriver(drvId);
        if (!d)
        {
            UI::error("Driver not found.");
            return;
        }

        vector<int> durations;
        for (int rid : d->rideHistory)
        {
            auto *r = findRide(rid);
            if (r)
                durations.push_back(r->duration);
        }

        if (durations.size() < 2)
        {
            UI::warn("Driver needs at least 2 rides for LIS analysis.");
            return;
        }

        cout << "\n  " << UI::BOLD << "Ride durations: " << UI::RESET;
        for (int x : durations)
            cout << x << " ";
        cout << "\n";

        vector<int> lisResult;
        int len = lis(durations, lisResult);

        cout << "  " << UI::BOLD << UI::GREEN << "LIS (longest increasing streak): length=" << len << "\n"
             << UI::RESET;
        cout << "  Sequence: ";
        for (int x : lisResult)
            cout << x << " ";
        cout << "\n";

        // Analysis
        double avg = 0;
        for (int x : durations)
            avg += x;
        avg /= durations.size();

        UI::tableRow("Total Rides", to_string(durations.size()));
        UI::tableRow("Avg Duration", to_string((int)avg) + " min");
        UI::tableRow("LIS Length", to_string(len));

        if (len <= (int)durations.size() / 2)
            UI::success("Driver is becoming more efficient over time!");
        else
            UI::warn("Driver's ride durations are trending upward.");
    }

    // ══════════════════════════════════════════
    //  DIJKSTRA: Full shortest path display
    // ══════════════════════════════════════════
    void dijkstraRoute()
    {
        UI::subHeader("Dijkstra: Shortest Path Finder");
        showLocations();
        int src = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int dest = UI::getInt("To Location ID", 0, (int)locations.size() - 1);

        vector<int> parent;
        vector<int> dist = graph->dijkstra(src, parent);

        if (dist[dest] == INF)
        {
            UI::error("No path found.");
            return;
        }

        vector<int> path = graph->getPath(parent, dest);
        UI::tableRow("Shortest Distance", to_string(dist[dest]) + " km");
        cout << "\n  " << UI::BOLD << "Path: " << UI::RESET;
        for (int i = 0; i < (int)path.size(); i++)
        {
            cout << UI::GREEN << locationName(path[i]) << UI::RESET;
            if (i + 1 < (int)path.size())
                cout << " → ";
        }
        cout << "\n\n  " << UI::BOLD << "All distances from " << locationName(src) << ":\n";
        for (auto &l : locations)
        {
            cout << "    " << left << setw(20) << l.name;
            if (dist[l.id] == INF)
                cout << "∞\n";
            else
                cout << dist[l.id] << " km\n";
        }
    }

    // ══════════════════════════════════════════
    //  RIDE HISTORY
    // ══════════════════════════════════════════
    void viewRideHistory()
    {
        UI::subHeader("Ride History");
        if (rides.empty())
        {
            UI::warn("No rides recorded yet.");
            return;
        }

        cout << "  " << left << setw(5) << "ID"
             << setw(15) << "Customer"
             << setw(15) << "Driver"
             << setw(15) << "Pickup"
             << setw(15) << "Drop"
             << setw(8) << "Fare"
             << setw(12) << "Status" << "\n";
        UI::line();

        for (auto &r : rides)
        {
            auto *cust = findCustomer(r.customerId);
            auto *drv = findDriver(r.driverId);
            cout << "  " << left << setw(5) << r.id
                 << setw(15) << (cust ? cust->name : "?")
                 << setw(15) << (drv ? drv->name : "?")
                 << setw(15) << locationName(r.pickupLocationId)
                 << setw(15) << locationName(r.dropLocationId)
                 << "$" << setw(7) << fixed << setprecision(2) << r.fare
                 << r.status << "\n";
        }
    }

    void viewRideDetails()
    {
        int id = UI::getInt("Enter Ride ID");
        auto *r = findRide(id);
        if (!r)
        {
            UI::error("Ride not found.");
            return;
        }

        auto *cust = findCustomer(r->customerId);
        auto *drv = findDriver(r->driverId);

        UI::subHeader("Ride Details #" + to_string(id));
        UI::tableRow("Customer", cust ? cust->name : "?");
        UI::tableRow("Driver", drv ? drv->name : "?");
        UI::tableRow("Pickup", locationName(r->pickupLocationId));
        UI::tableRow("Drop-off", locationName(r->dropLocationId));
        UI::tableRow("Distance", to_string(r->distance) + " km");
        UI::tableRow("Duration", to_string(r->duration) + " min");
        UI::tableRow("Fare", "$" + to_string(r->fare).substr(0, 6));
        UI::tableRow("Status", r->status);
        UI::tableRow("Timestamp", r->timestamp);
    }

    // ══════════════════════════════════════════
    //  STATS & REPORTS
    // ══════════════════════════════════════════
    void systemStats()
    {
        UI::subHeader("System Statistics");
        UI::tableRow("Total Customers", to_string(customers.size()));
        UI::tableRow("Total Drivers", to_string(drivers.size()));
        UI::tableRow("Total Rides", to_string(rides.size()));

        int available = 0;
        double totalFare = 0;
        for (auto &d : drivers)
            if (d.isAvailable)
                available++;
        for (auto &r : rides)
            totalFare += r.fare;

        UI::tableRow("Available Drivers", to_string(available));
        UI::tableRow("Total Revenue", "$" + to_string(totalFare).substr(0, 8));
        UI::tableRow("City Locations", to_string(locations.size()));
        UI::tableRow("City Roads", to_string(graph->roads.size()));

        // Top driver
        if (!drivers.empty())
        {
            auto topD = *max_element(drivers.begin(), drivers.end(),
                                     [](const Driver &a, const Driver &b)
                                     { return a.totalRides < b.totalRides; });
            UI::tableRow("Top Driver", topD.name + " (" + to_string(topD.totalRides) + " rides)");
        }

        // Algorithms info
        UI::subHeader("Algorithms in Use");
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "Dijkstra      — Nearest driver & shortest route\n";
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "Bellman-Ford  — Traffic-aware routing\n";
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "Prim's MST    — Optimal city road network\n";
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "Kruskal's MST — Union-Find road coverage\n";
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "LCS           — Common route analysis\n";
        cout << "  " << UI::CYAN << "✔ " << UI::RESET << "LIS           — Driver efficiency trend\n";
    }

    // ══════════════════════════════════════════
    //  UPDATE DRIVER LOCATION
    // ══════════════════════════════════════════
    void updateDriverLocation()
    {
        int id = UI::getInt("Driver ID");
        auto *d = findDriver(id);
        if (!d)
        {
            UI::error("Driver not found.");
            return;
        }
        showLocations();
        d->locationId = UI::getInt("New Location ID", 0, (int)locations.size() - 1);
        FileManager::saveDrivers(drivers);
        UI::success("Driver location updated to " + locationName(d->locationId));
    }

    // ══════════════════════════════════════════
    //  COMPARE BOTH MSTs
    // ══════════════════════════════════════════
    void compareMSTs()
    {
        UI::subHeader("MST Comparison: Prim's vs Kruskal's");

        int costP, costK;
        auto primsEdges = graph->primMST(costP);
        auto kruskalEdges = graph->kruskalMST(costK);

        cout << "\n  " << UI::BOLD << left << setw(35) << "PRIM'S MST"
             << "KRUSKAL'S MST\n"
             << UI::RESET;
        UI::line("─", 70);

        auto roads = FileManager::loadRoads();
        int maxRows = max(primsEdges.size(), kruskalEdges.size());
        for (int i = 0; i < maxRows; i++)
        {
            string left_col = "   ---", right_col = "   ---";
            if (i < (int)primsEdges.size())
            {
                auto [u, v] = primsEdges[i];
                int w = 0;
                for (auto &r : roads)
                    if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                        w = r.weight;
                left_col = locationName(u) + "↔" + locationName(v) + "(" + to_string(w) + ")";
            }
            if (i < (int)kruskalEdges.size())
            {
                auto &r = kruskalEdges[i];
                right_col = locationName(r.from) + "↔" + locationName(r.to) + "(" + to_string(r.weight) + ")";
            }
            cout << "  " << left << setw(35) << left_col << right_col << "\n";
        }
        UI::line("─", 70);
        cout << "  " << left << setw(35)
             << ("Total: " + to_string(costP) + " km")
             << ("Total: " + to_string(costK) + " km") << "\n";
        UI::info("Both algorithms produce equal total MST cost (they find the same optimal network).");
    }
};