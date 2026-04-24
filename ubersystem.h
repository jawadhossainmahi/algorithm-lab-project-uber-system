#pragma once
#include "uber.h"
#include "filemanager.h"
#include "ui.h"

// ─────────────────────────────────────────────
// UBER SYSTEM ENGINE
// ─────────────────────────────────────────────
class UberSystem
{
    vector<Location> locations;
    vector<Driver> drivers;
    vector<Customer> customers;
    vector<Ride> rides;
    Graph *graph;
    FareConfig fare;

    int nextCustomerId = 1;
    int nextDriverId = 1;
    int nextRideId = 1;

public:
    UberSystem() : graph(nullptr)
    {
        firstRunSetup(); // ask for fare config if missing
        loadAll();
        buildGraph();
    }
    ~UberSystem() { delete graph; }

    // ══════════════════════════════════════════
    // FIRST-RUN SETUP
    // Asks for fare config once; saved to config.txt
    // ══════════════════════════════════════════
    void firstRunSetup()
    {
        if (FileManager::loadConfig(fare))
            return; // already configured

        UI::clearScreen();
        cout << UI::CYAN << UI::BOLD;
        cout << "\n╔══════════════════════════════════════════╗\n";
        cout << "║      FIRST-TIME SETUP — FARE CONFIG      ║\n";
        cout << "╚══════════════════════════════════════════╝\n"
             << UI::RESET;
        cout << "\n  This setup runs only ONCE and saves to config.txt.\n";
        cout << "  You can change these any time from the Settings menu.\n\n";

        fare.baseFare = UI::getDouble("Base fare per ride (e.g. 2.50)", 0.0, 1000.0);
        fare.perKmRate = UI::getDouble("Charge per km (e.g. 1.50)", 0.0, 1000.0);
        fare.driverCut = UI::getDouble("Driver's cut fraction 0-1 (e.g. 0.75)", 0.0, 1.0);

        FileManager::saveConfig(fare);
        UI::success("Configuration saved to config.txt!");
        UI::pause();
    }

    void editFareConfig()
    {
        UI::subHeader("Edit Fare Configuration");
        UI::tableRow("Current Base Fare", "$" + to_string(fare.baseFare).substr(0, 6));
        UI::tableRow("Current Per-Km Rate", "$" + to_string(fare.perKmRate).substr(0, 6));
        UI::tableRow("Current Driver Cut", to_string(fare.driverCut * 100).substr(0, 5) + "%");
        cout << "\n";

        fare.baseFare = UI::getDouble("New base fare per ride", 0.0, 1000.0);
        fare.perKmRate = UI::getDouble("New charge per km", 0.0, 1000.0);
        fare.driverCut = UI::getDouble("New driver cut (0-1 fraction)", 0.0, 1.0);

        FileManager::saveConfig(fare);
        UI::success("Fare configuration updated and saved!");
    }

    // ── LOAD & SAVE ───────────────────────────
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

    // ── SEED DEFAULT MAP ──────────────────────
    // Locations have (x,y) coords.
    // distance = Euclidean from those coords.
    // weight   = traffic integer value (can be negative = faster shortcut lane).
    void seedCityMap()
    {
        locations = {
            {0, "Airport", 0.0, 0.0},
            {1, "City Center", 3.0, 4.0},
            {2, "North Park", 1.0, 8.0},
            {3, "East Market", 7.0, 3.0},
            {4, "West Town", -4.0, 5.0},
            {5, "South Bay", 2.0, -5.0},
            {6, "Tech Hub", 9.0, 9.0},
            {7, "University", -2.0, 9.0},
            {8, "Hospital", 5.0, 1.0},
            {9, "Shopping Mall", 6.0, 7.0},
        };
        FileManager::saveLocations(locations);

        // Build roads: distance = Euclidean; weight = traffic integer
        // Negative weight = road with traffic advantage (express lane etc.)
        auto euclidean = [&](int a, int b) -> double
        {
            double dx = locations[a].x - locations[b].x;
            double dy = locations[a].y - locations[b].y;
            return sqrt(dx * dx + dy * dy);
        };

        // {from, to, weight}  — distance computed automatically
        vector<tuple<int, int, int>> rawRoads = {
            {0, 1, 7},
            {0, 5, 8},
            {0, 8, 5},
            {1, 2, 6},
            {1, 3, 5},
            {1, 4, 7},
            {1, 8, 4},
            {1, 9, 6},
            {2, 4, 5},
            {2, 7, 4},
            {3, 6, 5},
            {3, 8, 3},
            {3, 9, -2}, // negative: express lane East Market ↔ Shopping Mall
            {4, 7, 4},
            {5, 8, 6},
            {6, 9, -1}, // negative: express lane Tech Hub ↔ Shopping Mall
            {7, 2, 4},
            {9, 6, 3},
        };

        vector<Road> roads;
        for (auto &[f, t, w] : rawRoads)
            roads.push_back({f, t, euclidean(f, t), w});

        FileManager::saveRoads(roads);
    }

    void buildGraph()
    {
        delete graph;
        graph = new Graph((int)locations.size());
        auto roads = FileManager::loadRoads();
        for (auto &r : roads)
            graph->addEdge(r);
    }

    // ── HELPERS ───────────────────────────────
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

    // Euclidean distance between two location IDs
    double euclidDist(int a, int b)
    {
        auto *la = findLocation(a);
        auto *lb = findLocation(b);
        if (!la || !lb)
            return 0;
        double dx = la->x - lb->x, dy = la->y - lb->y;
        return sqrt(dx * dx + dy * dy);
    }

    // ══════════════════════════════════════════
    // CUSTOMER MANAGEMENT
    // ══════════════════════════════════════════
    void registerCustomer()
    {
        UI::subHeader("Register New Customer");
        Customer c;
        c.id = nextCustomerId++;
        c.name = UI::getString("Full Name");
        c.phone = UI::getString("Phone Number");
        c.email = UI::getString("Email");
        c.totalSpent = 0;
        showLocations();
        c.locationId = UI::getInt("Your Location ID", 0, (int)locations.size() - 1);
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
        UI::tableRow("Total Spent", "$" + to_string(c->totalSpent).substr(0, 7));
    }

    void listCustomers()
    {
        UI::subHeader("All Customers");
        cout << "  " << left << setw(5) << "ID" << setw(20) << "Name"
             << setw(15) << "Phone" << setw(18) << "Location" << "Rides\n";
        UI::line();
        for (auto &c : customers)
            cout << "  " << left << setw(5) << c.id << setw(20) << c.name
                 << setw(15) << c.phone << setw(18) << locationName(c.locationId)
                 << c.rideHistory.size() << "\n";
    }

    // ══════════════════════════════════════════
    // DRIVER MANAGEMENT
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
        UI::tableRow("Earned", "$" + to_string(d->totalEarned).substr(0, 8));
    }

    void listDrivers()
    {
        UI::subHeader("All Drivers");
        cout << "  " << left << setw(5) << "ID" << setw(20) << "Name"
             << setw(15) << "Car" << setw(18) << "Location"
             << setw(12) << "Status" << "Rating\n";
        UI::line();
        for (auto &d : drivers)
        {
            string status = d.isAvailable ? "\033[32mAvail\033[0m" : "\033[31mBusy \033[0m";
            cout << "  " << left << setw(5) << d.id << setw(20) << d.name
                 << setw(15) << d.carModel << setw(18) << locationName(d.locationId)
                 << status << "  "
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
        UI::success("Driver " + d->name + " is now " +
                    (d->isAvailable ? "AVAILABLE" : "UNAVAILABLE"));
    }

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
    // LOCATION MAP
    // ══════════════════════════════════════════
    void showLocations()
    {
        UI::subHeader("City Locations  (x,y = 2D coordinates)");
        cout << "  " << left << setw(5) << "ID" << setw(20) << "Name"
             << setw(10) << "X" << setw(10) << "Y" << "\n";
        UI::line();
        for (auto &l : locations)
            cout << "  " << left << setw(5) << l.id << setw(20) << l.name
                 << setw(10) << l.x << setw(10) << l.y << "\n";
    }

    void showRoads()
    {
        UI::subHeader("City Roads  (distance = Euclidean km | weight = traffic value)");
        cout << "  " << left << setw(5) << "ID"
             << setw(18) << "From" << setw(18) << "To"
             << setw(14) << "Dist (km)" << "Weight\n";
        UI::line();
        auto roads = FileManager::loadRoads();
        for (int i = 0; i < (int)roads.size(); i++)
        {
            auto &r = roads[i];
            string wstr = to_string(r.weight);
            if (r.weight < 0)
                wstr = UI::GREEN + wstr + " (negative!)" + UI::RESET;
            cout << "  " << left << setw(5) << i
                 << setw(18) << locationName(r.from)
                 << setw(18) << locationName(r.to)
                 << setw(14) << fixed << setprecision(2) << r.distance
                 << wstr << "\n";
        }
    }

    void addRoad()
    {
        UI::subHeader("Add New Road");
        showLocations();
        int from = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int to = UI::getInt("To Location ID", 0, (int)locations.size() - 1);

        // Distance auto-computed from x,y coords
        double dist = euclidDist(from, to);
        cout << "\n";
        UI::info("Auto-computed Euclidean distance: " +
                 to_string(dist).substr(0, 6) + " km");

        cout << "\n  Traffic weight is used by Bellman-Ford (can be negative).\n"
             << "  Negative weight = express/priority lane (faster route).\n"
             << "  Positive weight = congested road (higher cost).\n\n";

        int weight = UI::getInt("Traffic weight (e.g. 5 or -3)", -999, 999);

        Road r{from, to, dist, weight};
        auto roads = FileManager::loadRoads();
        roads.push_back(r);
        FileManager::saveRoads(roads);
        graph->addEdge(r);

        UI::success("Road added: " + locationName(from) + " ↔ " + locationName(to));
        cout << "  " << UI::CYAN
             << "Distance: " << fixed << setprecision(2) << dist << " km"
             << "  |  Weight: " << weight
             << UI::RESET << "\n";
    }

    // ══════════════════════════════════════════
    // BOOK A RIDE (core feature)
    // ══════════════════════════════════════════
    // → Ask user: avoid traffic? (Dijkstra) or with traffic? (Bellman-Ford)
    // → Distance for fare = Euclidean (from x,y)
    // → Weight = traffic value, used only in Bellman-Ford path selection
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

        // ── Algorithm choice ─────────────────────
        cout << "\n"
             << UI::BOLD << "  Route preference:\n"
             << UI::RESET
             << "  " << UI::CYAN << "[1]" << UI::RESET
             << " Avoid traffic  → Dijkstra  (uses road DISTANCE, optimal km route)\n"
             << "  " << UI::CYAN << "[2]" << UI::RESET
             << " With traffic   → Bellman-Ford  (uses road WEIGHT, handles negative-weight lanes)\n\n";

        int algoChoice = UI::getInt("Your choice", 1, 2);
        bool avoidTraffic = (algoChoice == 1);
        string algoName = avoidTraffic ? "DIJKSTRA" : "BELLMAN-FORD";

        // ── Find nearest driver using Dijkstra (always distance-based) ──
        cout << "\n";
        UI::algoBox("DIJKSTRA", "Finding nearest available driver by distance");

        vector<int> parentCustomer;
        auto distFromCustomer = graph->dijkstra(cust->locationId, parentCustomer);

        int bestDriverId = -1;
        double bestDist = INF_D;
        for (auto &d : drivers)
        {
            if (!d.isAvailable)
                continue;
            if (distFromCustomer[d.locationId] < bestDist)
            {
                bestDist = distFromCustomer[d.locationId];
                bestDriverId = d.id;
            }
        }

        if (bestDriverId == -1)
        {
            UI::error("No available drivers right now. Try again later.");
            return;
        }
        auto *driver = findDriver(bestDriverId);

        // ── Find route: customer → drop ──────────
        cout << "\n";
        vector<int> pathNodes;

        if (avoidTraffic)
        {
            // ── DIJKSTRA ─────────────────────────
            UI::algoBox("DIJKSTRA", "Shortest path by Euclidean distance");
            vector<int> parentDrop;
            auto distToDrop = graph->dijkstra(cust->locationId, parentDrop);
            if (distToDrop[dropId] >= INF_D)
            {
                UI::error("No path found to destination.");
                return;
            }
            pathNodes = graph->getPath(parentDrop, dropId);
            // routeWeightOrDist = distToDrop[dropId]; // unused
        }
        else
        {
            // ── BELLMAN-FORD ──────────────────────
            UI::algoBox("BELLMAN-FORD", "Optimal path by traffic weight (negative weights OK)");
            vector<double> bfDist;
            vector<int> bfParent;
            bool ok = graph->bellmanFord(cust->locationId, bfDist, bfParent);
            if (!ok)
            {
                UI::error("Negative cycle detected! Cannot use Bellman-Ford safely.");
                UI::warn("Falling back to Dijkstra for safety.");
                vector<int> parentDrop;
                auto distToDrop = graph->dijkstra(cust->locationId, parentDrop);
                pathNodes = graph->getPath(parentDrop, dropId);
                algoName = "DIJKSTRA(fallback)";
            }
            else
            {
                if (bfDist[dropId] >= INF_D)
                {
                    UI::error("No path found to destination.");
                    return;
                }
                pathNodes = graph->getPath(bfParent, dropId);
                // routeWeightOrDist = bfDist[dropId]; // unused
            }
        }

        // ── Euclidean distance along path (for fare) ──
        double rideDistKm = graph->pathDistance(pathNodes, locations);
        int pathWeight = graph->pathWeight(pathNodes);

        // ── Terminal: show route clearly ──────────
        UI::subHeader("Route Information");
        cout << "  " << UI::BOLD << "Algorithm Used : " << UI::RESET
             << UI::CYAN << algoName << UI::RESET << "\n";
        cout << "  " << UI::BOLD << "Route          : " << UI::RESET;
        for (int i = 0; i < (int)pathNodes.size(); i++)
        {
            cout << UI::CYAN << locationName(pathNodes[i]) << UI::RESET;
            if (i + 1 < (int)pathNodes.size())
                cout << " → ";
        }
        cout << "\n\n";

        // Show per-segment breakdown
        cout << "  " << UI::BOLD << "Segment Breakdown:\n"
             << UI::RESET;
        cout << "  " << left << setw(18) << "From" << setw(18) << "To"
             << setw(14) << "Dist (km)" << setw(10) << "Weight" << "\n";
        UI::line("─", 60);
        for (int i = 0; i + 1 < (int)pathNodes.size(); i++)
        {
            int u = pathNodes[i], v = pathNodes[i + 1];
            double segDist = euclidDist(u, v);
            int segW = 0;
            for (auto &r : graph->roads)
                if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                {
                    segW = r.weight;
                    break;
                }
            string wStr = to_string(segW);
            if (segW < 0)
                wStr = UI::GREEN + wStr + " ◀ express" + UI::RESET;
            cout << "  " << left << setw(18) << locationName(u)
                 << setw(18) << locationName(v)
                 << setw(14) << fixed << setprecision(2) << segDist
                 << wStr << "\n";
        }
        UI::line("─", 60);
        cout << "  " << UI::BOLD
             << "Total Euclidean distance : " << fixed << setprecision(2)
             << rideDistKm << " km\n"
             << UI::RESET;
        cout << "  " << UI::BOLD
             << "Total traffic weight     : " << pathWeight
             << (avoidTraffic ? "  (not used for routing)" : "  (used for path selection)")
             << "\n"
             << UI::RESET;

        // ── Fare ──────────────────────────────────
        double rideFare = fare.baseFare + rideDistKm * fare.perKmRate;
        int duration = (int)(rideDistKm * 3.0 + bestDist * 2.0);

        cout << "\n";
        UI::subHeader("Ride Details");
        UI::tableRow("Customer", cust->name);
        UI::tableRow("Driver", driver->name + " (" + driver->carModel + ")");
        UI::tableRow("Driver at", locationName(driver->locationId));
        UI::tableRow("Driver ETA", to_string(bestDist).substr(0, 5) + " km away");
        UI::tableRow("Pickup", locationName(cust->locationId));
        UI::tableRow("Drop-off", locationName(dropId));
        UI::tableRow("Distance", to_string(rideDistKm).substr(0, 6) + " km  (Euclidean from x,y)");
        UI::tableRow("Traffic wt", to_string(pathWeight) + "  (Bellman-Ford weight sum)");
        UI::tableRow("Algorithm", algoName);
        UI::tableRow("Base fare", "$" + to_string(fare.baseFare).substr(0, 6));
        UI::tableRow("Per-km rate", "$" + to_string(fare.perKmRate).substr(0, 6) + "/km");
        UI::tableRow("Est. Fare", "$" + to_string(rideFare).substr(0, 7));
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
        ride.fare = rideFare;
        ride.status = "COMPLETED";
        ride.timestamp = FileManager::currentTimestamp();
        ride.distance = rideDistKm;
        ride.duration = duration;
        ride.algorithm = algoName;

        cust->rideHistory.push_back(ride.id);
        cust->totalSpent += rideFare;
        cust->locationId = dropId;

        driver->isAvailable = false;
        driver->rideHistory.push_back(ride.id);
        driver->totalRides++;
        driver->totalEarned += rideFare * fare.driverCut;
        driver->locationId = dropId;

        int stars = UI::getInt("Rate your driver (1-5)", 1, 5);
        driver->rating = (driver->rating * (driver->totalRides - 1) + stars) / driver->totalRides;
        driver->isAvailable = true;

        rides.push_back(ride);
        saveAll();

        UI::success("Ride completed!  Fare: $" + to_string(rideFare).substr(0, 7) +
                    "  |  Algorithm: " + algoName);
        UI::success("Thank you, " + cust->name + "!");
    }

    // ══════════════════════════════════════════
    // DIJKSTRA ROUTE (standalone analysis)
    // ══════════════════════════════════════════
    void dijkstraRoute()
    {
        UI::subHeader("Dijkstra: Shortest Path (by Euclidean Distance)");
        UI::info("Uses road DISTANCE (from x,y coords). Always positive. No negatives.");
        showLocations();
        int src = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int dest = UI::getInt("To Location ID", 0, (int)locations.size() - 1);

        vector<int> parent;
        auto dist = graph->dijkstra(src, parent);

        if (dist[dest] >= INF_D)
        {
            UI::error("No path found.");
            return;
        }

        vector<int> path = graph->getPath(parent, dest);
        double pathDist = graph->pathDistance(path, locations);

        UI::algoBox("DIJKSTRA", "Shortest by Euclidean distance");
        cout << "  " << UI::BOLD << "Path: " << UI::RESET;
        for (int i = 0; i < (int)path.size(); i++)
        {
            cout << UI::GREEN << locationName(path[i]) << UI::RESET;
            if (i + 1 < (int)path.size())
                cout << " → ";
        }
        cout << "\n\n";

        // Segment table
        cout << "  " << left << setw(18) << "From" << setw(18) << "To"
             << setw(12) << "Dist (km)" << "Weight\n";
        UI::line("─", 60);
        for (int i = 0; i + 1 < (int)path.size(); i++)
        {
            int u = path[i], v = path[i + 1];
            double sd = euclidDist(u, v);
            int sw = 0;
            for (auto &r : graph->roads)
                if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                {
                    sw = r.weight;
                    break;
                }
            cout << "  " << left << setw(18) << locationName(u)
                 << setw(18) << locationName(v)
                 << setw(12) << fixed << setprecision(2) << sd
                 << sw << "\n";
        }
        UI::line("─", 60);
        UI::tableRow("Total distance", to_string(pathDist).substr(0, 6) + " km");

        cout << "\n  " << UI::BOLD << "All distances from " << locationName(src) << ":\n"
             << UI::RESET;
        for (auto &l : locations)
        {
            string ds = (dist[l.id] >= INF_D) ? "∞"
                                              : to_string(dist[l.id]).substr(0, 6) + " km";
            cout << "  " << left << setw(20) << l.name << ds << "\n";
        }
    }

    // ══════════════════════════════════════════
    // BELLMAN-FORD ROUTE (standalone analysis)
    // ══════════════════════════════════════════
    void bellmanFordRoute()
    {
        UI::subHeader("Bellman-Ford: Traffic-Aware Shortest Route");
        UI::info("Uses road WEIGHT (traffic value). CAN handle negative weights.");
        UI::info("Negative weight = express lane / traffic advantage.");
        showLocations();
        int src = UI::getInt("From Location ID", 0, (int)locations.size() - 1);
        int dest = UI::getInt("To Location ID", 0, (int)locations.size() - 1);

        vector<double> bfDist;
        vector<int> bfParent;
        bool ok = graph->bellmanFord(src, bfDist, bfParent);

        if (!ok)
        {
            UI::error("Negative cycle detected in road network!");
            return;
        }

        if (bfDist[dest] >= INF_D)
        {
            UI::warn("No path found from " + locationName(src) + " to " + locationName(dest));
            return;
        }

        vector<int> path = graph->getPath(bfParent, dest);
        double pathDist = graph->pathDistance(path, locations);
        int pathWt = graph->pathWeight(path);

        UI::algoBox("BELLMAN-FORD", "Optimal path minimising traffic weight");
        cout << "  " << UI::BOLD << "Path: " << UI::RESET;
        for (int i = 0; i < (int)path.size(); i++)
        {
            cout << UI::YELLOW << locationName(path[i]) << UI::RESET;
            if (i + 1 < (int)path.size())
                cout << " → ";
        }
        cout << "\n\n";

        // Segment table
        cout << "  " << left << setw(18) << "From" << setw(18) << "To"
             << setw(14) << "Dist (km)" << "Weight\n";
        UI::line("─", 65);
        for (int i = 0; i + 1 < (int)path.size(); i++)
        {
            int u = path[i], v = path[i + 1];
            double sd = euclidDist(u, v);
            int sw = 0;
            for (auto &r : graph->roads)
                if ((r.from == u && r.to == v) || (r.from == v && r.to == u))
                {
                    sw = r.weight;
                    break;
                }
            string wStr = to_string(sw);
            if (sw < 0)
                wStr = UI::GREEN + wStr + " ◀ express lane" + UI::RESET;
            cout << "  " << left << setw(18) << locationName(u)
                 << setw(18) << locationName(v)
                 << setw(14) << fixed << setprecision(2) << sd
                 << wStr << "\n";
        }
        UI::line("─", 65);
        cout << "  Total Euclidean distance : " << fixed << setprecision(2) << pathDist << " km\n";
        cout << "  Total traffic weight     : " << pathWt
             << (pathWt < 0 ? "  (negative = used express lanes)" : "") << "\n";

        cout << "\n  " << UI::BOLD << "All weight-distances from " << locationName(src) << ":\n"
             << UI::RESET;
        for (auto &l : locations)
        {
            string ds = (bfDist[l.id] >= INF_D) ? "∞"
                                                : to_string((int)bfDist[l.id]) + " wt";
            cout << "  " << left << setw(20) << l.name << ds << "\n";
        }
    }

    // ══════════════════════════════════════════
    // PRIM'S MST
    // ══════════════════════════════════════════
    void primsMST()
    {
        UI::subHeader("Prim's Algorithm: Minimum Spanning Road Network");
        UI::info("Uses DISTANCE (Euclidean km). Negative weights not applicable here.");
        double totalCost;
        auto mstEdges = graph->primMST(totalCost);
        cout << "\n  " << UI::BOLD << "MST Edges (Prim's):\n"
             << UI::RESET;
        cout << "  " << left << setw(20) << "From" << setw(20) << "To" << "Distance\n";
        UI::line();
        for (auto &[u, v] : mstEdges)
        {
            double d = euclidDist(u, v);
            cout << "  " << left << setw(20) << locationName(u)
                 << setw(20) << locationName(v)
                 << fixed << setprecision(2) << d << " km\n";
        }
        UI::line();
        UI::info("Total MST cost: " + to_string(totalCost).substr(0, 7) + " km");
    }

    // ══════════════════════════════════════════
    // KRUSKAL'S MST
    // ══════════════════════════════════════════
    void kruskalMST()
    {
        UI::subHeader("Kruskal's Algorithm: Minimum Spanning Road Network");
        UI::info("Uses DISTANCE (Euclidean km). Union-Find based.");
        double totalCost;
        auto mst = graph->kruskalMST(totalCost);
        cout << "\n  " << UI::BOLD << "MST Edges (Kruskal's):\n"
             << UI::RESET;
        cout << "  " << left << setw(20) << "From" << setw(20) << "To" << "Distance\n";
        UI::line();
        for (auto &r : mst)
            cout << "  " << left << setw(20) << locationName(r.from)
                 << setw(20) << locationName(r.to)
                 << fixed << setprecision(2) << r.distance << " km\n";
        UI::line();
        UI::info("Total MST cost: " + to_string(totalCost).substr(0, 7) + " km");
    }

    void compareMSTs()
    {
        UI::subHeader("MST Comparison: Prim's vs Kruskal's");
        double costP, costK;
        auto primsEdges = graph->primMST(costP);
        auto kruskalEdges = graph->kruskalMST(costK);

        cout << "\n  " << UI::BOLD << left << setw(38) << "PRIM'S MST"
             << "KRUSKAL'S MST\n"
             << UI::RESET;
        UI::line("─", 76);

        int maxRows = max(primsEdges.size(), kruskalEdges.size());
        for (int i = 0; i < maxRows; i++)
        {
            string lc = "  ---", rc = "  ---";
            if (i < (int)primsEdges.size())
            {
                auto [u, v] = primsEdges[i];
                lc = locationName(u) + " ↔ " + locationName(v) +
                     " (" + to_string(euclidDist(u, v)).substr(0, 4) + "km)";
            }
            if (i < (int)kruskalEdges.size())
            {
                auto &r = kruskalEdges[i];
                rc = locationName(r.from) + " ↔ " + locationName(r.to) +
                     " (" + to_string(r.distance).substr(0, 4) + "km)";
            }
            cout << "  " << left << setw(38) << lc << rc << "\n";
        }
        UI::line("─", 76);
        cout << "  " << left << setw(38)
             << ("Total: " + to_string(costP).substr(0, 6) + " km")
             << ("Total: " + to_string(costK).substr(0, 6) + " km") << "\n";
        UI::info("Both algorithms find the same optimal total MST cost.");
    }

    // ══════════════════════════════════════════
    // LCS
    // ══════════════════════════════════════════
    void lcsAnalysis()
    {
        UI::subHeader("LCS: Common Routes Between Two Customers");
        int id1 = UI::getInt("Customer 1 ID");
        int id2 = UI::getInt("Customer 2 ID");
        auto *c1 = findCustomer(id1);
        auto *c2 = findCustomer(id2);
        if (!c1 || !c2)
        {
            UI::error("One or both customers not found.");
            return;
        }

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

        auto seq1 = getLocSeq(c1), seq2 = getLocSeq(c2);
        if (seq1.empty() || seq2.empty())
        {
            UI::warn("One or both customers have no ride history.");
            return;
        }

        vector<vector<int>> dp;
        int len = lcs(seq1, seq2, dp);
        auto common = lcsSequence(seq1, seq2, dp);

        cout << "\n  " << c1->name << "'s route: ";
        for (int id : seq1)
            cout << locationName(id) << " → ";
        cout << "\n";
        cout << "  " << c2->name << "'s route: ";
        for (int id : seq2)
            cout << locationName(id) << " → ";
        cout << "\n\n";

        cout << "  " << UI::BOLD << UI::GREEN << "Common Route (LCS length=" << len << "):\n"
             << UI::RESET;
        for (int id : common)
            cout << "  " << UI::CYAN << locationName(id) << UI::RESET << "\n";
        if (common.empty())
            UI::info("No common routes found.");
        else
            UI::info("Carpooling opportunity detected!");
    }

    // ══════════════════════════════════════════
    // LIS
    // ══════════════════════════════════════════
    void lisAnalysis()
    {
        UI::subHeader("LIS: Driver Efficiency Analysis");
        UI::info("Longest Increasing Subsequence of ride durations.");
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
            UI::warn("Driver needs at least 2 rides.");
            return;
        }

        cout << "\n  Ride durations: ";
        for (int x : durations)
            cout << x << " ";
        cout << "\n";

        vector<int> lisResult;
        int len = lis(durations, lisResult);

        cout << "  " << UI::BOLD << UI::GREEN << "LIS length=" << len << ": " << UI::RESET;
        for (int x : lisResult)
            cout << x << " ";
        cout << "\n";

        double avg = 0;
        for (int x : durations)
            avg += x;
        avg /= durations.size();
        UI::tableRow("Total Rides", to_string(durations.size()));
        UI::tableRow("Avg Duration", to_string((int)avg) + " min");
        UI::tableRow("LIS Length", to_string(len));
        if (len <= (int)durations.size() / 2)
            UI::success("Driver efficiency improving!");
        else
            UI::warn("Ride durations trending upward.");
    }

    // ══════════════════════════════════════════
    // RIDE HISTORY
    // ══════════════════════════════════════════
    void viewRideHistory()
    {
        UI::subHeader("Ride History");
        if (rides.empty())
        {
            UI::warn("No rides recorded yet.");
            return;
        }
        cout << "  " << left << setw(5) << "ID" << setw(14) << "Customer"
             << setw(14) << "Driver" << setw(14) << "Pickup"
             << setw(14) << "Drop" << setw(10) << "Dist(km)"
             << setw(9) << "Fare" << setw(16) << "Algorithm" << "Status\n";
        UI::line("─", 96);
        for (auto &r : rides)
        {
            auto *cust = findCustomer(r.customerId);
            auto *drv = findDriver(r.driverId);
            cout << "  " << left << setw(5) << r.id
                 << setw(14) << (cust ? cust->name : "?")
                 << setw(14) << (drv ? drv->name : "?")
                 << setw(14) << locationName(r.pickupLocationId)
                 << setw(14) << locationName(r.dropLocationId)
                 << setw(10) << fixed << setprecision(2) << r.distance
                 << "$" << setw(8) << setprecision(2) << r.fare
                 << setw(16) << r.algorithm
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
        UI::tableRow("Distance", to_string(r->distance).substr(0, 6) + " km (Euclidean)");
        UI::tableRow("Duration", to_string(r->duration) + " min");
        UI::tableRow("Fare", "$" + to_string(r->fare).substr(0, 7));
        UI::tableRow("Algorithm", r->algorithm);
        UI::tableRow("Status", r->status);
        UI::tableRow("Timestamp", r->timestamp);
    }

    // ══════════════════════════════════════════
    // SYSTEM STATS
    // ══════════════════════════════════════════
    void systemStats()
    {
        UI::subHeader("System Statistics");
        UI::tableRow("Total Customers", to_string(customers.size()));
        UI::tableRow("Total Drivers", to_string(drivers.size()));
        UI::tableRow("Total Rides", to_string(rides.size()));
        int avail = 0;
        double totalFare = 0;
        for (auto &d : drivers)
            if (d.isAvailable)
                avail++;
        for (auto &r : rides)
            totalFare += r.fare;
        UI::tableRow("Available Drivers", to_string(avail));
        UI::tableRow("Total Revenue", "$" + to_string(totalFare).substr(0, 9));
        UI::tableRow("City Locations", to_string(locations.size()));
        UI::tableRow("City Roads", to_string(graph->roads.size()));
        UI::tableRow("Base Fare", "$" + to_string(fare.baseFare).substr(0, 6));
        UI::tableRow("Per-Km Rate", "$" + to_string(fare.perKmRate).substr(0, 6));
        UI::tableRow("Driver Cut", to_string(fare.driverCut * 100).substr(0, 5) + "%");

        if (!drivers.empty())
        {
            auto topD = *max_element(drivers.begin(), drivers.end(),
                                     [](const Driver &a, const Driver &b)
                                     { return a.totalRides < b.totalRides; });
            UI::tableRow("Top Driver", topD.name + " (" + to_string(topD.totalRides) + " rides)");
        }

        int dijk = 0, bf = 0;
        for (auto &r : rides)
        {
            if (r.algorithm.find("DIJKSTRA") != string::npos)
                dijk++;
            else
                bf++;
        }
        UI::subHeader("Algorithm Usage");
        UI::tableRow("Dijkstra rides", to_string(dijk));
        UI::tableRow("Bellman-Ford rides", to_string(bf));

        cout << "\n";
        UI::info("Dijkstra   — path by Euclidean distance (no negatives)");
        UI::info("Bellman-Ford — path by traffic weight   (negatives OK)");
        UI::info("Distance for fare ALWAYS uses Euclidean x,y calculation");
    }
};