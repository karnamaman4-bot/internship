#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <limits>

using namespace std;

// Class representing individual transactions
class Transaction {
public:
    string type;
    double amount;
    double balanceAfter;

    Transaction(string t, double amt, double bal) 
        : type(t), amount(amt), balanceAfter(bal) {}

    string serialize() const {
        return type + "," + to_string(amount) + "," + to_string(balanceAfter);
    }

    static Transaction deserialize(const string& line) {
        stringstream ss(line);
        string t, amtStr, balStr;
        getline(ss, t, ',');
        getline(ss, amtStr, ',');
        getline(ss, balStr, ',');
        return Transaction(t, stod(amtStr), stod(balStr));
    }
};

// Class representing a Bank Account
class BankAccount {
private:
    int accountNumber;
    string accountHolderName;
    double balance;
    vector<Transaction> history;

public:
    BankAccount() : accountNumber(0), balance(0.0) {}

    BankAccount(int accNum, string name, double initialBalance) 
        : accountNumber(accNum), accountHolderName(name), balance(initialBalance) {
        if (initialBalance > 0) {
            history.push_back(Transaction("INITIAL DEPOSIT", initialBalance, balance));
        }
    }

    int getAccountNumber() const { return accountNumber; }
    string getAccountHolderName() const { return accountHolderName; }
    double getBalance() const { return balance; }

    void deposit(double amount) {
        if (amount <= 0) {
            cout << "\n [!] Error: Deposit amount must be greater than zero.\n";
            return;
        }
        balance += amount;
        history.push_back(Transaction("DEPOSIT", amount, balance));
        cout << "\n [✓] Successfully deposited $" << fixed << setprecision(2) << amount;
        cout << ". New Balance: $" << balance << "\n";
    }

    bool withdraw(double amount) {
        if (amount <= 0) {
            cout << "\n [!] Error: Withdrawal amount must be greater than zero.\n";
            return false;
        }
        if (amount > balance) {
            cout << "\n [!] Insufficient Funds! Current Balance: $" << fixed << setprecision(2) << balance << "\n";
            return false;
        }
        balance -= amount;
        history.push_back(Transaction("WITHDRAWAL", amount, balance));
        cout << "\n [✓] Successfully withdrew $" << fixed << setprecision(2) << amount;
        cout << ". Remaining Balance: $" << balance << "\n";
        return true;
    }

    void displayAccountInfo() const {
        cout << "\n=========================================\n";
        cout << "           ACCOUNT DETAILS               \n";
        cout << "=========================================\n";
        cout << " Account Number : " << accountNumber << "\n";
        cout << " Holder Name    : " << accountHolderName << "\n";
        cout << " Current Balance: $" << fixed << setprecision(2) << balance << "\n";
        cout << "=========================================\n";
    }

    void displayTransactionHistory() const {
        cout << "\n-----------------------------------------------------\n";
        cout << left << setw(20) << "Type" << setw(15) << "Amount ($)" << setw(15) << "Balance ($)" << "\n";
        cout << "-----------------------------------------------------\n";
        if (history.empty()) {
            cout << " No transaction history available.\n";
        } else {
            for (const auto& tx : history) {
                cout << left << setw(20) << tx.type 
                     << setw(15) << fixed << setprecision(2) << tx.amount 
                     << setw(15) << tx.balanceAfter << "\n";
            }
        }
        cout << "-----------------------------------------------------\n";
    }

    // Save account state to file format: accNum|name|balance|tx1;tx2;...
    string serialize() const {
        stringstream ss;
        ss << accountNumber << "|" << accountHolderName << "|" << balance << "|";
        for (size_t i = 0; i < history.size(); ++i) {
            ss << history[i].serialize();
            if (i < history.size() - 1) ss << ";";
        }
        return ss.str();
    }

    // Load account state from file string
    static BankAccount deserialize(const string& line) {
        stringstream ss(line);
        string accNumStr, name, balStr, historyStr;

        getline(ss, accNumStr, '|');
        getline(ss, name, '|');
        getline(ss, balStr, '|');
        getline(ss, historyStr, '|');

        BankAccount acc;
        acc.accountNumber = stoi(accNumStr);
        acc.accountHolderName = name;
        acc.balance = stod(balStr);

        if (!historyStr.empty()) {
            stringstream txStream(historyStr);
            string txLine;
            while (getline(txStream, txLine, ';')) {
                if (!txLine.empty()) {
                    acc.history.push_back(Transaction::deserialize(txLine));
                }
            }
        }
        return acc;
    }
};

// Bank Manager class handling system operations and persistent storage
class BankSystem {
private:
    vector<BankAccount> accounts;
    const string filename = "bank_data.txt";

    void loadAccountsFromFile() {
        accounts.clear();
        ifstream file(filename);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                accounts.push_back(BankAccount::deserialize(line));
            }
        }
        file.close();
    }

    void saveAccountsToFile() {
        ofstream file(filename, ios::trunc);
        for (const auto& acc : accounts) {
            file << acc.serialize() << "\n";
        }
        file.close();
    }

    BankAccount* findAccount(int accNum) {
        for (auto& acc : accounts) {
            if (acc.getAccountNumber() == accNum) {
                return &acc;
            }
        }
        return nullptr;
    }

public:
    BankSystem() {
        loadAccountsFromFile();
    }

    ~BankSystem() {
        saveAccountsToFile();
    }

    void createAccount() {
        int accNum;
        string name;
        double initialDeposit;

        cout << "\n--- CREATE NEW ACCOUNT ---\n";
        cout << "Enter Account Number: ";
        while (!(cin >> accNum)) {
            cout << "Invalid input. Enter a valid numerical Account Number: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        if (findAccount(accNum) != nullptr) {
            cout << " [!] Error: Account with ID " << accNum << " already exists!\n";
            return;
        }

        cout << "Enter Account Holder Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Initial Deposit Amount ($): ";
        while (!(cin >> initialDeposit) || initialDeposit < 0) {
            cout << "Invalid input. Enter a non-negative amount ($): ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        BankAccount newAcc(accNum, name, initialDeposit);
        accounts.push_back(newAcc);
        saveAccountsToFile();

        cout << "\n [✓] Account created successfully for " << name << "!\n";
    }

    void depositMoney() {
        int accNum;
        double amount;
        cout << "\nEnter Account Number: ";
        cin >> accNum;

        BankAccount* acc = findAccount(accNum);
        if (!acc) {
            cout << " [!] Account not found.\n";
            return;
        }

        cout << "Enter Deposit Amount ($): ";
        cin >> amount;
        acc->deposit(amount);
        saveAccountsToFile();
    }

    void withdrawMoney() {
        int accNum;
        double amount;
        cout << "\nEnter Account Number: ";
        cin >> accNum;

        BankAccount* acc = findAccount(accNum);
        if (!acc) {
            cout << " [!] Account not found.\n";
            return;
        }

        cout << "Enter Withdrawal Amount ($): ";
        cin >> amount;
        if (acc->withdraw(amount)) {
            saveAccountsToFile();
        }
    }

    void checkBalance() {
        int accNum;
        cout << "\nEnter Account Number: ";
        cin >> accNum;

        BankAccount* acc = findAccount(accNum);
        if (!acc) {
            cout << " [!] Account not found.\n";
            return;
        }

        acc->displayAccountInfo();
        acc->displayTransactionHistory();
    }

    void displayAllAccounts() {
        if (accounts.empty()) {
            cout << "\n [!] No records found in the system.\n";
            return;
        }

        cout << "\n=========================================================\n";
        cout << "                ALL CUSTOMER ACCOUNTS                    \n";
        cout << "=========================================================\n";
        cout << left << setw(15) << "Acc No." << setw(25) << "Holder Name" << setw(15) << "Balance ($)" << "\n";
        cout << "---------------------------------------------------------\n";

        for (const auto& acc : accounts) {
            cout << left << setw(15) << acc.getAccountNumber() 
                 << setw(25) << acc.getAccountHolderName() 
                 << setw(15) << fixed << setprecision(2) << acc.getBalance() << "\n";
        }
        cout << "=========================================================\n";
    }
};

int main() {
    BankSystem bank;
    int choice;

    do {
        cout << "\n=========================================\n";
        cout << "     ADVANCED BANK MANAGEMENT SYSTEM     \n";
        cout << "=========================================\n";
        cout << " 1. Create New Account\n";
        cout << " 2. Deposit Money\n";
        cout << " 3. Withdraw Money\n";
        cout << " 4. Balance Enquiry & Statement\n";
        cout << " 5. Display All Accounts\n";
        cout << " 6. Exit\n";
        cout << "=========================================\n";
        cout << "Select Choice (1-6): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << " [!] Invalid input! Please enter a number between 1 and 6.\n";
            continue;
        }

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.depositMoney(); break;
            case 3: bank.withdrawMoney(); break;
            case 4: bank.checkBalance(); break;
            case 5: bank.displayAllAccounts(); break;
            case 6: cout << "\nThank you for using our Banking System!\n"; break;
            default: cout << " [!] Invalid option. Please try again.\n";
        }
    } while (choice != 6);

    return 0;
}
