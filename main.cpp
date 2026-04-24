#include "ubersystem.h"

// ─────────────────────────────────────────────
// MENUS
// ─────────────────────────────────────────────

void customerMenu(UberSystem& sys) {
    while (true) {
        UI::header("CUSTOMER MANAGEMENT");
        UI::menuItem(1, "Register New Customer");
        UI::menuItem(2, "View Customer Profile");
        UI::menuItem(3, "List All Customers");
        UI::menuItem(0, "Back");
        int ch = UI::getInt("Choice", 0, 3);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.registerCustomer(); break;
            case 2: sys.viewCustomer();     break;
            case 3: sys.listCustomers();    break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

void driverMenu(UberSystem& sys) {
    while (true) {
        UI::header("DRIVER MANAGEMENT");
        UI::menuItem(1, "Register New Driver");
        UI::menuItem(2, "View Driver Profile");
        UI::menuItem(3, "List All Drivers");
        UI::menuItem(4, "Toggle Availability");
        UI::menuItem(5, "Update Driver Location");
        UI::menuItem(0, "Back");
        int ch = UI::getInt("Choice", 0, 5);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.registerDriver();           break;
            case 2: sys.viewDriver();               break;
            case 3: sys.listDrivers();              break;
            case 4: sys.toggleDriverAvailability(); break;
            case 5: sys.updateDriverLocation();     break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

void algorithmMenu(UberSystem& sys) {
    while (true) {
        UI::header("ALGORITHM ANALYSIS TOOLS");
        cout << "\n  " << UI::BOLD << "Graph Algorithms:\n" << UI::RESET;
        UI::menuItem(1, "Dijkstra  — Shortest path by DISTANCE (Euclidean km)");
        UI::menuItem(2, "Bellman-Ford — Shortest path by WEIGHT (handles negatives)");
        UI::menuItem(3, "Prim's MST — Minimum road network");
        UI::menuItem(4, "Kruskal's MST — Minimum road network (Union-Find)");
        UI::menuItem(5, "Compare MSTs");
        cout << "\n  " << UI::BOLD << "Dynamic Programming:\n" << UI::RESET;
        UI::menuItem(6, "LCS Analysis — Common routes between customers");
        UI::menuItem(7, "LIS Analysis — Driver efficiency trend");
        UI::menuItem(0, "Back");

        int ch = UI::getInt("Choice", 0, 7);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.dijkstraRoute();  break;
            case 2: sys.bellmanFordRoute(); break;
            case 3: sys.primsMST();       break;
            case 4: sys.kruskalMST();     break;
            case 5: sys.compareMSTs();    break;
            case 6: sys.lcsAnalysis();    break;
            case 7: sys.lisAnalysis();    break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

void mapMenu(UberSystem& sys) {
    while (true) {
        UI::header("CITY MAP");
        UI::menuItem(1, "View All Locations  (with x,y coordinates)");
        UI::menuItem(2, "View All Roads      (distance + traffic weight)");
        UI::menuItem(3, "Add New Road");
        UI::menuItem(0, "Back");
        int ch = UI::getInt("Choice", 0, 3);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.showLocations(); break;
            case 2: sys.showRoads();     break;
            case 3: sys.addRoad();       break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

void ridesMenu(UberSystem& sys) {
    while (true) {
        UI::header("RIDES");
        UI::menuItem(1, "View All Rides");
        UI::menuItem(2, "View Ride Details");
        UI::menuItem(0, "Back");
        int ch = UI::getInt("Choice", 0, 2);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.viewRideHistory(); break;
            case 2: sys.viewRideDetails(); break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

void settingsMenu(UberSystem& sys) {
    while (true) {
        UI::header("SETTINGS");
        UI::menuItem(1, "Edit Fare Configuration  (base fare, per-km rate, driver cut)");
        UI::menuItem(0, "Back");
        int ch = UI::getInt("Choice", 0, 1);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.editFareConfig(); break;
            case 0: return;
        }
        UI::pause(); UI::clearScreen();
    }
}

// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main() {
    UberSystem sys;   // first-run setup happens inside constructor

    while (true) {
        UI::banner();
        cout << "\n";
        UI::menuItem(1, "Book a Ride  (choose Dijkstra or Bellman-Ford)");
        UI::menuItem(2, "Customer Management");
        UI::menuItem(3, "Driver Management");
        UI::menuItem(4, "City Map & Roads");
        UI::menuItem(5, "Ride History");
        UI::menuItem(6, "Algorithm Analysis  (DSA Showcase)");
        UI::menuItem(7, "System Statistics");
        UI::menuItem(8, "Settings  (fare config)");
        UI::menuItem(0, "Exit");
        cout << "\n";

        int ch = UI::getInt("Choose an option", 0, 8);
        UI::clearScreen();
        switch (ch) {
            case 1: sys.bookRide();     UI::pause(); UI::clearScreen(); break;
            case 2: customerMenu(sys);  break;
            case 3: driverMenu(sys);    break;
            case 4: mapMenu(sys);       break;
            case 5: ridesMenu(sys);     break;
            case 6: algorithmMenu(sys); break;
            case 7: sys.systemStats();  UI::pause(); UI::clearScreen(); break;
            case 8: settingsMenu(sys);  break;
            case 0:
                UI::header("GOODBYE");
                cout << "\n  " << UI::CYAN
                     << "Thank you for using Uber DSA System!\n\n" << UI::RESET;
                return 0;
        }
    }
}