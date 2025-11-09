// main.cpp — Simple Linux File Explorer (C++17, “normal” version)
// Features: list, cd, create, delete, copy, move/rename, search, view/chmod
#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using std::cout;
using std::cin;
using std::string;

static string cwd() { return fs::current_path().string(); }

void pause_enter() {
    cout << "\nPress Enter to continue...";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void list_files() {
    try {
        cout << "\nListing: " << cwd() << "\n";
        cout << "---------------------------------------------\n";
        for (const auto& e : fs::directory_iterator(fs::current_path())) {
            const auto p = e.path();
            const bool isDir = e.is_directory();
            const bool isReg = e.is_regular_file();
            const auto size = isReg ? std::to_string(e.file_size()) : "-";
            string type = isDir ? "DIR " : (isReg ? "FILE" : "OTHER");
            cout << type << "  " << p.filename().string() << "  (" << size << ")\n";
        }
    } catch (const std::exception& ex) {
        cout << "Error: " << ex.what() << "\n";
    }
}

void change_dir(const string& path) {
    try {
        fs::current_path(path);
        cout << "Now in: " << cwd() << "\n";
    } catch (const std::exception& ex) {
        cout << "cd error: " << ex.what() << "\n";
    }
}

void create_file(const string& name) {
    try {
        if (fs::exists(name)) { cout << "Already exists.\n"; return; }
        std::ofstream f(name);
        if (!f) { cout << "Failed to create file.\n"; return; }
        cout << "Created file: " << name << "\n";
    } catch (const std::exception& ex) {
        cout << "Create error: " << ex.what() << "\n";
    }
}

void delete_entry(const string& path) {
    try {
        if (!fs::exists(path)) { cout << "Not found.\n"; return; }
        if (fs::is_directory(path)) {
            auto n = fs::remove_all(path);
            cout << "Removed directory (and contents): " << path << " (" << n << " items)\n";
        } else {
            fs::remove(path);
            cout << "Removed file: " << path << "\n";
        }
    } catch (const std::exception& ex) {
        cout << "Delete error: " << ex.what() << "\n";
    }
}

void copy_entry(const string& src, const string& dst) {
    try {
        if (!fs::exists(src)) { cout << "Source not found.\n"; return; }
        if (fs::is_directory(src)) {
            fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        } else {
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        }
        cout << "Copied to: " << dst << "\n";
    } catch (const std::exception& ex) {
        cout << "Copy error: " << ex.what() << "\n";
    }
}

void move_rename(const string& src, const string& dst) {
    try {
        fs::rename(src, dst);
        cout << "Moved/Renamed to: " << dst << "\n";
    } catch (const std::exception& ex) {
        cout << "Move/Rename error: " << ex.what() << "\n";
    }
}

void search_name_ci(const string& needle) {
    auto toLower = [](string s){ std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
    string q = toLower(needle);
    size_t hits = 0;
    try {
        for (auto it = fs::recursive_directory_iterator(fs::current_path(), fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it) {
            const auto name = it->path().filename().string();
            if (toLower(name).find(q) != string::npos) {
                cout << it->path().string() << "\n";
                ++hits;
            }
        }
        if (!hits) cout << "No matches.\n";
    } catch (const std::exception& ex) {
        cout << "Search error: " << ex.what() << "\n";
    }
}

void view_perms(const string& path) {
    try {
        if (!fs::exists(path)) { cout << "Not found.\n"; return; }
        auto pr = fs::status(path).permissions();
        auto has = [&](fs::perms p){ return (pr & p) != fs::perms::none; };
        auto bit = [&](bool b, char c){ return b ? c : '-'; };
        string s;
        s += bit(has(fs::perms::owner_read ), 'r');
        s += bit(has(fs::perms::owner_write), 'w');
        s += bit(has(fs::perms::owner_exec), 'x');
        s += bit(has(fs::perms::group_read ), 'r');
        s += bit(has(fs::perms::group_write), 'w');
        s += bit(has(fs::perms::group_exec), 'x');
        s += bit(has(fs::perms::others_read ), 'r');
        s += bit(has(fs::perms::others_write), 'w');
        s += bit(has(fs::perms::others_exec), 'x');
        cout << path << " : " << s << "\n";
    } catch (const std::exception& ex) {
        cout << "Perms error: " << ex.what() << "\n";
    }
}

fs::perms perms_from_octal(const string& oct) {
    // Accept like "755" / "644"
    if (oct.size() != 3 || !std::all_of(oct.begin(), oct.end(), ::isdigit)) return fs::perms::unknown;
    int v = std::stoi(oct, nullptr, 8);
    fs::perms p = fs::perms::none;
    int u = (v >> 6) & 7, g = (v >> 3) & 7, o = v & 7;
    auto set = [&](int bits, fs::perms r, fs::perms w, fs::perms x){
        if (bits & 4) p |= r;
        if (bits & 2) p |= w;
        if (bits & 1) p |= x;
    };
    set(u, fs::perms::owner_read, fs::perms::owner_write, fs::perms::owner_exec);
    set(g, fs::perms::group_read, fs::perms::group_write, fs::perms::group_exec);
    set(o, fs::perms::others_read, fs::perms::others_write, fs::perms::others_exec);
    return p;
}

void chmod_octal(const string& path, const string& oct) {
    try {
        if (!fs::exists(path)) { cout << "Not found.\n"; return; }
        fs::perms p = perms_from_octal(oct);
        if (p == fs::perms::unknown) { cout << "Use octal like 755 or 644.\n"; return; }
        fs::permissions(path, p, fs::perm_options::replace);
        cout << "Permissions set to " << oct << " for " << path << "\n";
    } catch (const std::exception& ex) {
        cout << "chmod error: " << ex.what() << "\n";
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);

    while (true) {
        cout << "\nCurrent Directory: \"" << cwd() << "\"\n";
        cout << "1. List Files\n";
        cout << "2. Change Directory\n";
        cout << "3. Create File\n";
        cout << "4. Delete File/Folder\n";
        cout << "5. Copy (file/folder)\n";
        cout << "6. Move/Rename\n";
        cout << "7. Search (case-insensitive)\n";
        cout << "8. View Permissions\n";
        cout << "9. Change Permissions (octal)\n";
        cout << "10. Exit\n\n";
        cout << "Enter choice: ";

        int ch;
        if (!(cin >> ch)) break;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (ch == 1) {
            list_files(); pause_enter();
        } else if (ch == 2) {
            cout << "Path to cd: ";
            string path; std::getline(cin, path);
            change_dir(path);
        } else if (ch == 3) {
            cout << "Filename: ";
            string name; std::getline(cin, name);
            create_file(name);
        } else if (ch == 4) {
            cout << "Path to delete: ";
            string p; std::getline(cin, p);
            delete_entry(p);
        } else if (ch == 5) {
            cout << "Source: "; string s; std::getline(cin, s);
            cout << "Destination: "; string d; std::getline(cin, d);
            copy_entry(s, d);
        } else if (ch == 6) {
            cout << "Source: "; string s; std::getline(cin, s);
            cout << "Destination (new name/path): "; string d; std::getline(cin, d);
            move_rename(s, d);
        } else if (ch == 7) {
            cout << "Search name contains: ";
            string q; std::getline(cin, q);
            search_name_ci(q); pause_enter();
        } else if (ch == 8) {
            cout << "Path: "; string p; std::getline(cin, p);
            view_perms(p); pause_enter();
        } else if (ch == 9) {
            cout << "Path: "; string p; std::getline(cin, p);
            cout << "Octal (e.g., 755): "; string o; std::getline(cin, o);
            chmod_octal(p, o);
        } else if (ch == 10) {
            cout << "Bye!\n"; break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}
