#pragma once
#include "uber.h"

// ─────────────────────────────────────────────
// FILE I/O MANAGER
// ─────────────────────────────────────────────
class FileManager {
public:

    // ── CONFIG (fare settings) ─────────────────
    static void saveConfig(const FareConfig& cfg) {
        ofstream f(CONFIG_FILE);
        f << fixed << setprecision(4)
          << cfg.baseFare   << "\n"
          << cfg.perKmRate  << "\n"
          << cfg.driverCut  << "\n";
    }

    // Returns false if config file doesn't exist yet (first run)
    static bool loadConfig(FareConfig& cfg) {
        ifstream f(CONFIG_FILE);
        if (!f) return false;
        f >> cfg.baseFare >> cfg.perKmRate >> cfg.driverCut;
        return (bool)f;
    }

    // ── LOCATIONS ─────────────────────────────
    static void saveLocations(const vector<Location>& locs) {
        ofstream f(LOCATIONS_FILE);
        for (auto& l : locs)
            f << l.id << "|" << l.name << "|"
              << fixed << setprecision(4) << l.x << "|" << l.y << "\n";
    }

    static vector<Location> loadLocations() {
        vector<Location> locs;
        ifstream f(LOCATIONS_FILE);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Location l;
            string tok;
            getline(ss, tok, '|'); l.id = stoi(tok);
            getline(ss, l.name,  '|');
            getline(ss, tok,     '|'); l.x = stod(tok);
            getline(ss, tok,     '|'); l.y = stod(tok);
            locs.push_back(l);
        }
        return locs;
    }

    // ── ROADS ─────────────────────────────────
    // Format: from|to|distance|weight
    static void saveRoads(const vector<Road>& roads) {
        ofstream f(ROADS_FILE);
        for (auto& r : roads)
            f << r.from << "|" << r.to << "|"
              << fixed << setprecision(4) << r.distance << "|"
              << r.weight << "\n";
    }

    static vector<Road> loadRoads() {
        vector<Road> roads;
        ifstream f(ROADS_FILE);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Road r;
            string tok;
            getline(ss, tok, '|'); r.from     = stoi(tok);
            getline(ss, tok, '|'); r.to       = stoi(tok);
            getline(ss, tok, '|'); r.distance = stod(tok);
            getline(ss, tok, '|'); r.weight   = stoi(tok);
            roads.push_back(r);
        }
        return roads;
    }

    // ── CUSTOMERS ─────────────────────────────
    static void saveCustomers(const vector<Customer>& custs) {
        ofstream f(CUSTOMERS_FILE);
        for (auto& c : custs) {
            f << c.id << "|" << c.name << "|" << c.phone << "|"
              << c.email << "|" << c.locationId << "|"
              << fixed << setprecision(4) << c.totalSpent << "|";
            for (int i = 0; i < (int)c.rideHistory.size(); i++)
                f << c.rideHistory[i] << (i+1 < (int)c.rideHistory.size() ? "," : "");
            f << "\n";
        }
    }

    static vector<Customer> loadCustomers() {
        vector<Customer> custs;
        ifstream f(CUSTOMERS_FILE);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Customer c;
            string tok;
            getline(ss, tok,      '|'); c.id         = stoi(tok);
            getline(ss, c.name,   '|');
            getline(ss, c.phone,  '|');
            getline(ss, c.email,  '|');
            getline(ss, tok,      '|'); c.locationId = stoi(tok);
            getline(ss, tok,      '|'); c.totalSpent = stod(tok);
            getline(ss, tok);
            if (!tok.empty()) {
                istringstream hist(tok);
                string rid;
                while (getline(hist, rid, ','))
                    if (!rid.empty()) c.rideHistory.push_back(stoi(rid));
            }
            custs.push_back(c);
        }
        return custs;
    }

    // ── DRIVERS ───────────────────────────────
    static void saveDrivers(const vector<Driver>& drvs) {
        ofstream f(DRIVERS_FILE);
        for (auto& d : drvs) {
            f << d.id << "|" << d.name << "|" << d.phone << "|"
              << d.carModel << "|" << d.licensePlate << "|"
              << d.locationId << "|" << d.isAvailable << "|"
              << fixed << setprecision(4)
              << d.rating << "|" << d.totalRides << "|" << d.totalEarned << "|";
            for (int i = 0; i < (int)d.rideHistory.size(); i++)
                f << d.rideHistory[i] << (i+1 < (int)d.rideHistory.size() ? "," : "");
            f << "\n";
        }
    }

    static vector<Driver> loadDrivers() {
        vector<Driver> drvs;
        ifstream f(DRIVERS_FILE);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Driver d;
            string tok;
            getline(ss, tok,            '|'); d.id          = stoi(tok);
            getline(ss, d.name,         '|');
            getline(ss, d.phone,        '|');
            getline(ss, d.carModel,     '|');
            getline(ss, d.licensePlate, '|');
            getline(ss, tok,            '|'); d.locationId  = stoi(tok);
            getline(ss, tok,            '|'); d.isAvailable = stoi(tok);
            getline(ss, tok,            '|'); d.rating      = stod(tok);
            getline(ss, tok,            '|'); d.totalRides  = stoi(tok);
            getline(ss, tok,            '|'); d.totalEarned = stod(tok);
            getline(ss, tok);
            if (!tok.empty()) {
                istringstream hist(tok);
                string rid;
                while (getline(hist, rid, ','))
                    if (!rid.empty()) d.rideHistory.push_back(stoi(rid));
            }
            drvs.push_back(d);
        }
        return drvs;
    }

    // ── RIDES ─────────────────────────────────
    static void saveRides(const vector<Ride>& rides) {
        ofstream f(RIDES_FILE);
        for (auto& r : rides)
            f << r.id << "|" << r.customerId << "|" << r.driverId << "|"
              << r.pickupLocationId << "|" << r.dropLocationId << "|"
              << fixed << setprecision(4) << r.fare << "|"
              << r.status << "|" << r.timestamp << "|"
              << r.distance << "|" << r.duration << "|"
              << r.algorithm << "\n";
    }

    static vector<Ride> loadRides() {
        vector<Ride> rides;
        ifstream f(RIDES_FILE);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Ride r;
            string tok;
            getline(ss, tok,        '|'); r.id               = stoi(tok);
            getline(ss, tok,        '|'); r.customerId        = stoi(tok);
            getline(ss, tok,        '|'); r.driverId          = stoi(tok);
            getline(ss, tok,        '|'); r.pickupLocationId  = stoi(tok);
            getline(ss, tok,        '|'); r.dropLocationId    = stoi(tok);
            getline(ss, tok,        '|'); r.fare              = stod(tok);
            getline(ss, r.status,   '|');
            getline(ss, r.timestamp,'|');
            getline(ss, tok,        '|'); r.distance          = stod(tok);
            getline(ss, tok,        '|'); r.duration          = stoi(tok);
            getline(ss, r.algorithm);
            rides.push_back(r);
        }
        return rides;
    }

    static string currentTimestamp() {
        time_t now = time(0);
        char buf[30];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }
};