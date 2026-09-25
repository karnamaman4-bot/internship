#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <limits>

using namespace std;

// Class representing a Book
class Book {
private:
    int id;
    string title;
    string author;
    int totalCopies;
    int availableCopies;

public:
    Book() : id(0), totalCopies(0), availableCopies(0) {}
    Book(int bId, string t, string a, int copies)
        : id(bId), title(t), author(a), totalCopies(copies), availableCopies(copies) {}

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getAvailableCopies() const { return availableCopies; }

    bool issueBook() {
        if (availableCopies > 0) {
            availableCopies--;
            return true;
        }
        return false;
    }

    void returnBook() {
        if (availableCopies < totalCopies) {
            availableCopies++;
        }
    }

    void display() const {
        cout << left << setw(8) << id 
             << setw(30) << title 
             << setw(25) << author 
             << setw(12) << availableCopies 
             << setw(10) << totalCopies << "\n";
    }

    string serialize() const {
        return to_string(id) + "|" + title + "|" + author + "|" + to_string(totalCopies) + "|" + to_string(availableCopies);
    }

    static Book deserialize(const string& line) {
        stringstream ss(line);
        string idStr, t, a, totalStr, availStr;
        getline(ss, idStr, '|');
        getline(ss, t, '|');
        getline(ss, a, '|');
        getline(ss, totalStr, '|');
        getline(ss, availStr, '|');

        Book b;
        b.id = stoi(idStr);
        b.title = t;
        b.author = a;
        b.totalCopies = stoi(totalStr);
        b.availableCopies = stoi(availStr);
        return b;
    }
};

// Class representing a Library Member
class Member {
private:
    int memberId;
    string name;

public:
    Member() : memberId(0) {}
    Member(int mId, string n) : memberId(mId), name(n) {}

    int getMemberId() const { return memberId; }
    string getName() const { return name; }

    void display() const {
        cout << left << setw(12) << memberId << setw(30) << name << "\n";
    }

    string serialize() const {
        return to_string(memberId) + "|" + name;
    }

    static Member deserialize(const string& line) {
        stringstream ss(line);
        string idStr, n;
        getline(ss, idStr, '|');
        getline(ss, n, '|');
        return Member(stoi(idStr), n);
    }
};

// Main Library Management System Class
class LibrarySystem {
private:
    vector<Book> books;
    vector<Member> members;
    const string booksFile = "books.txt";
    const string membersFile = "members.txt";

    void loadData() {
        books.clear();
        members.clear();

        ifstream bFile(booksFile);
        string line;
        while (bFile && getline(bFile, line)) {
            if (!line.empty()) books.push_back(Book::deserialize(line));
        }
        bFile.close();

        ifstream mFile(membersFile);
        while (mFile && getline(mFile, line)) {
            if (!line.empty()) members.push_back(Member::deserialize(line));
        }
        mFile.close();
    }

    void saveData() {
        ofstream bFile(booksFile, ios::trunc);
        for (const auto& b : books) bFile << b.serialize() << "\n";
        bFile.close();

        ofstream mFile(membersFile, ios::trunc);
        for (const auto& m : members) mFile << m.serialize() << "\n";
        mFile.close();
    }

    Book* findBook(int id) {
        for (auto& b : books) {
            if (b.getId() == id) return &b;
        }
        return nullptr;
    }

    Member* findMember(int id) {
        for (auto& m : members) {
            if (m.getMemberId() == id) return &m;
        }
        return nullptr;
    }

public:
    LibrarySystem() { loadData(); }
    ~LibrarySystem() { saveData(); }

    void addBook() {
        int id, copies;
        string title, author;

        cout << "\n--- ADD NEW BOOK ---\n";
        cout << "Enter Book ID: ";
        cin >> id;
        if (findBook(id) != nullptr) {
            cout << " [!] Error: Book ID already exists!\n";
            return;
        }
        cin.ignore();
        cout << "Enter Title: ";
        getline(cin, title);
        cout << "Enter Author: ";
        getline(cin, author);
        cout << "Enter Quantity: ";
        cin >> copies;

        books.push_back(Book(id, title, author, copies));
        saveData();
        cout << " [✓] Book added successfully!\n";
    }

    void registerMember() {
        int id;
        string name;

        cout << "\n--- REGISTER MEMBER ---\n";
        cout << "Enter Member ID: ";
        cin >> id;
        if (findMember(id) != nullptr) {
            cout << " [!] Error: Member ID already exists!\n";
            return;
        }
        cin.ignore();
        cout << "Enter Member Name: ";
        getline(cin, name);

        members.push_back(Member(id, name));
        saveData();
        cout << " [✓] Member registered successfully!\n";
    }

    void issueBook() {
        int bookId, memberId;
        cout << "\n--- ISSUE BOOK ---\n";
        cout << "Enter Member ID: ";
        cin >> memberId;
        Member* m = findMember(memberId);
        if (!m) {
            cout << " [!] Member not found.\n";
            return;
        }

        cout << "Enter Book ID: ";
        cin >> bookId;
        Book* b = findBook(bookId);
        if (!b) {
            cout << " [!] Book not found.\n";
            return;
        }

        if (b->issueBook()) {
            saveData();
            cout << " [✓] Book '" << b->getTitle() << "' issued to " << m->getName() << "!\n";
        } else {
            cout << " [!] Out of stock! No available copies.\n";
        }
    }

    void returnBook() {
        int bookId;
        cout << "\n--- RETURN BOOK ---\n";
        cout << "Enter Book ID: ";
        cin >> bookId;

        Book* b = findBook(bookId);
        if (!b) {
            cout << " [!] Book not found.\n";
            return;
        }

        b->returnBook();
        saveData();
        cout << " [✓] Book '" << b->getTitle() << "' returned successfully!\n";
    }

    void searchBook() {
        int choice;
        cout << "\n--- SEARCH BOOK ---\n";
        cout << "1. Search by Title\n2. Search by Author\nChoice: ";
        cin >> choice;
        cin.ignore();

        string query;
        cout << "Enter search term: ";
        getline(cin, query);

        transform(query.begin(), query.end(), query.begin(), ::tolower);

        cout << "\n" << left << setw(8) << "ID" << setw(30) << "Title" << setw(25) << "Author" << setw(12) << "Available" << setw(10) << "Total" << "\n";
        cout << string(85, '-') << "\n";

        bool found = false;
        for (const auto& b : books) {
            string field = (choice == 1) ? b.getTitle() : b.getAuthor();
            transform(field.begin(), field.end(), field.begin(), ::tolower);

            if (field.find(query) != string::npos) {
                b.display();
                found = true;
            }
        }
        if (!found) cout << "No matching books found.\n";
    }

    void displayAllBooks() {
        if (books.empty()) {
            cout << "\n [!] No books available in library.\n";
            return;
        }
        cout << "\n=====================================================================================\n";
        cout << left << setw(8) << "ID" << setw(30) << "Title" << setw(25) << "Author" << setw(12) << "Available" << setw(10) << "Total" << "\n";
        cout << "=====================================================================================\n";
        for (const auto& b : books) b.display();
        cout << "=====================================================================================\n";
    }
};

int main() {
    LibrarySystem library;
    int choice;

    do {
        cout << "\n=========================================\n";
        cout << "       LIBRARY MANAGEMENT SYSTEM         \n";
        cout << "=========================================\n";
        cout << " 1. Add New Book\n";
        cout << " 2. Register New Member\n";
        cout << " 3. Issue Book\n";
        cout << " 4. Return Book\n";
        cout << " 5. Search Book (Title / Author)\n";
        cout << " 6. Display All Books\n";
        cout << " 7. Exit\n";
        cout << "=========================================\n";
        cout << "Select Option (1-7): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: library.addBook(); break;
            case 2: library.registerMember(); break;
            case 3: library.issueBook(); break;
            case 4: library.returnBook(); break;
            case 5: library.searchBook(); break;
            case 6: library.displayAllBooks(); break;
            case 7: cout << "\nExiting system. Goodbye!\n"; break;
            default: cout << " [!] Invalid selection.\n";
        }
    } while (choice != 7);

    return 0;
}
