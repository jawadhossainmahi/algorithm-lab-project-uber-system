#pragma once
#include "uber.h"

// ─────────────────────────────────────────────
// TERMINAL UI HELPERS
// ─────────────────────────────────────────────
namespace UI
{

    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[37m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";

    void clearScreen() { cout << "\033[2J\033[1;1H"; }

    void line(const string &c = "─", int n = 60)
    {
        for (int i = 0; i < n; i++)
            cout << c;
        cout << "\n";
    }

    void header(const string &title)
    {
        cout << "\n"
             << CYAN << BOLD;
        line("═");
        int pad = (58 - (int)title.size()) / 2;
        cout << " " << string(max(0, pad), ' ') << title << "\n";
        line("═");
        cout << RESET;
    }

    void subHeader(const string &title)
    {
        cout << "\n"
             << YELLOW << BOLD << " >> " << title << RESET << "\n";
        cout << YELLOW;
        line("─");
        cout << RESET;
    }

    void success(const string &msg) { cout << GREEN << BOLD << " ✔ " << msg << RESET << "\n"; }
    void error(const string &msg) { cout << RED << BOLD << " ✘ " << msg << RESET << "\n"; }
    void info(const string &msg) { cout << CYAN << " ℹ " << msg << RESET << "\n"; }
    void warn(const string &msg) { cout << YELLOW << " ⚠ " << msg << RESET << "\n"; }

    void algoBox(const string &algo, const string &reason)
    {
        cout << "\n";
        cout << CYAN << BOLD << " ┌─────────────────────────────────────────┐\n";
        cout << " │  Algorithm : " << left << setw(27) << algo << "│\n";
        cout << " │  Reason    : " << left << setw(27) << reason << "│\n";
        cout << " └─────────────────────────────────────────┘" << RESET << "\n\n";
    }

    void prompt(const string &msg)
    {
        cout << BOLD << MAGENTA << "\n » " << msg << ": " << RESET;
    }

    void menuItem(int n, const string &label)
    {
        cout << "  " << CYAN << BOLD << "[" << n << "] " << RESET << label << "\n";
    }

    int getInt(const string &p, int lo = INT_MIN, int hi = INT_MAX)
    {
        while (true)
        {
            prompt(p);
            string s;
            getline(cin, s);
            try
            {
                int v = stoi(s);
                if (v >= lo && v <= hi)
                    return v;
                error("Enter a value between " + to_string(lo) + " and " + to_string(hi));
            }
            catch (...)
            {
                error("Invalid number. Try again.");
            }
        }
    }

    double getDouble(const string &p, double lo = 0.0, double hi = 1e9)
    {
        while (true)
        {
            prompt(p);
            string s;
            getline(cin, s);
            try
            {
                double v = stod(s);
                if (v >= lo && v <= hi)
                    return v;
                error("Enter a value between " + to_string(lo) + " and " + to_string(hi));
            }
            catch (...)
            {
                error("Invalid number. Try again.");
            }
        }
    }

    string getString(const string &p)
    {
        prompt(p);
        string s;
        getline(cin, s);
        return s;
    }

    void pause()
    {
        cout << BOLD << "\n  Press ENTER to continue..." << RESET;
        string tmp;
        getline(cin, tmp);
    }

    void banner()
    {
        clearScreen();
        cout << CYAN << BOLD;
        cout << R"(
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║   ██╗   ██╗██████╗ ███████╗██████╗                      ║
║   ██║   ██║██╔══██╗██╔════╝██╔══██╗                     ║
║   ██║   ██║██████╔╝█████╗  ██████╔╝                     ║
║   ██║   ██║██╔══██╗██╔══╝  ██╔══██╗                     ║
║   ╚██████╔╝██████╔╝███████╗██║  ██║                     ║
║    ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝  SYSTEM            ║
║                                                          ║
║   DSA-Powered Ride Management  |  Dijkstra + Bellman    ║
╚══════════════════════════════════════════════════════════╝
)";
        cout << RESET;
    }

    void tableRow(const string &label, const string &value, int width = 22)
    {
        cout << "  " << BOLD << left << setw(width) << label << RESET
             << " : " << value << "\n";
    }

    string stars(double rating)
    {
        string s;
        int full = (int)round(rating);
        for (int i = 0; i < 5; i++)
            s += (i < full ? "★" : "☆");
        return s + " (" + to_string(rating).substr(0, 3) + ")";
    }

} // namespace UI