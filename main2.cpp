#include <iostream>
#include <filesystem>
#include <fstream>
using namespace std;
namespace fs = std::filesystem;

void listDirectory(const fs::path &p) {
    cout << "\n--- Files and Folders in: " << p << " ---\n";
    for (const auto &entry : fs::directory_iterator(p)) {
        cout << (entry.is_directory() ? "[DIR]  " : "FILE   ");
        cout << entry.path().filename().string() << "\n";
    }
}

int main() {
    fs::path current = fs::current_path();
    int choice;
    string name, newname;

    while (true) {
        cout << "\n=============================================\n";
        cout << "           TERMINAL FILE EXPLORER            \n";
        cout << "=============================================\n";
        cout << "Current Directory: " << current << "\n\n";
        cout << "1. List Files\n";
        cout << "2. Change Directory\n";
        cout << "3. Create File\n";
        cout << "4. Delete File\n";
        cout << "5. Rename / Move File\n";
        cout << "6. Search File (Recursive)\n";
        cout << "7. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {

            case 1:
                listDirectory(current);
                break;

            case 2:
                cout << "Enter folder name (.. to go back): ";
                cin >> name;
                if (name == "..") {
                    current = current.parent_path();
                } else if (fs::exists(current / name) && fs::is_directory(current / name)) {
                    current /= name;
                } else {
                    cout << "Folder not found.\n";
                }
                break;

            case 3:
                cout << "Enter new file name: ";
                cin >> name;
                ofstream(current / name);
                cout << "File created.\n";
                break;

            case 4:
                cout << "Enter file name to delete: ";
                cin >> name;
                if (fs::exists(current / name)) {
                    fs::remove(current / name);
                    cout << "File deleted.\n";
                } else {
                    cout << "File not found.\n";
                }
                break;

            case 5:
                cout << "Enter current file name: ";
                cin >> name;
                cout << "Enter new name: ";
                cin >> newname;
                try {
                    fs::rename(current / name, current / newname);
                    cout << "File renamed.\n";
                } catch (...) {
                    cout << "Rename failed. File may not exist.\n";
                }
                break;

            case 6:
                cout << "Enter file name to search: ";
                cin >> name;
                cout << "\nSearching...\n";
                for (auto &entry : fs::recursive_directory_iterator(current)) {
                    if (entry.path().filename() == name)
                        cout << "Found at: " << entry.path() << "\n";
                }
                break;

            case 7:
                cout << "\nExiting File Explorer... Goodbye!\n";
                return 0;

            default:
                cout << "Invalid Option. Try Again.\n";
        }
    }
}
