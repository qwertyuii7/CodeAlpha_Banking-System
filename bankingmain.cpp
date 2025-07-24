#include <bits/stdc++.h>
#include <ctime>
using namespace std;

class Transaction;

class Account {
public:
    float balance;

    Account() {
        balance = 0.0;
    }

    Account(float bal) {
        balance = bal;
    }

    float getBalance() {
        return balance;
    }

    void deposit(float amount) {
        balance += amount;
    }

    bool withdraw(float amount) {
        if (amount <= balance && amount > 0) {
            balance -= amount;
            return true;
        }
        return false;
    }
};

class Customer {
public:
    int id;
    string name;
    string mobile;
    string pin;
    Account account;

    Customer() {}

    Customer(int id, string name, string mobile, string pin, float balance) {
        this->id = id;
        this->name = name;
        this->mobile = mobile;
        this->pin = pin;
        this->account = Account(balance);
    }

    void saveToFile() {
        ofstream file("users.txt", ios::app);
        file << id << " " << name << " " << mobile << " " << pin << " " << account.getBalance() << endl;
        file.close();
    }

    static bool loadCustomer(int id, Customer &cust) {
        ifstream file("users.txt");
        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            int cid;
            string name, mobile, pin;
            float balance;
            iss >> cid >> name >> mobile >> pin >> balance;
            if (cid == id) {
                cust = Customer(cid, name, mobile, pin, balance);
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    bool verifyPin(string inputPin) {
        return inputPin == pin;
    }

    void updateBalanceInFile() {
        ifstream file("users.txt");
        vector<string> lines;
        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            int cid;
            iss >> cid;
            if (cid == id) {
                ostringstream updatedLine;
                updatedLine << id << " " << name << " " << mobile << " " << pin << " " << account.getBalance();
                lines.push_back(updatedLine.str());
            } else {
                lines.push_back(line);
            }
        }
        file.close();
        ofstream outFile("users.txt");
        for (const auto &l : lines)
            outFile << l << endl;
        outFile.close();
    }
};

class Transaction {
public:
    void addTransaction(int accNo, string type, float amount, string otherParty, float newBalance) {
        string filename = "trans_" + to_string(accNo) + ".txt";
        ofstream file(filename, ios::app);

        time_t now = time(0);
        char *dt = ctime(&now);
        string dateTime(dt);
        dateTime.pop_back(); // remove newline

        file << dateTime << " | " << type << " | " << amount << " | " << otherParty << " | " << newBalance << endl;
        file.close();
    }

    void showTransactionHistory(int accNo, int lastN = 5) {
        string filename = "trans_" + to_string(accNo) + ".txt";
        ifstream file(filename);
        vector<string> lines;
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        cout << "\n--- Last " << min((int)lines.size(), lastN) << " Transactions ---\n";
        for (int i = max(0, (int)lines.size() - lastN); i < lines.size(); ++i) {
            cout << lines[i] << endl;
        }
    }

    bool transferFunds(int sender_id, string sender_pin, int receiver_id, float amount) {
        Customer sender, receiver;
        if (!Customer::loadCustomer(sender_id, sender)) {
            cout << "Sender not found.\n";
            return false;
        }
        if (!sender.verifyPin(sender_pin)) {
            cout << "Invalid PIN.\n";
            return false;
        }
        if (!Customer::loadCustomer(receiver_id, receiver)) {
            cout << "Receiver not found.\n";
            return false;
        }
        if (!sender.account.withdraw(amount)) {
            cout << "Insufficient balance.\n";
            return false;
        }

        receiver.account.deposit(amount);
        sender.updateBalanceInFile();
        receiver.updateBalanceInFile();

        addTransaction(sender_id, "TRANSFER", amount, "TO " + to_string(receiver_id), sender.account.getBalance());
        addTransaction(receiver_id, "RECEIVED", amount, "FROM " + to_string(sender_id), receiver.account.getBalance());

        cout << "Transfer Successful!\n";
        return true;
    }
};
int main() {
    Transaction txn;

    while (true) {
        cout << "\n====== BANK SYSTEM ======" << endl;
        cout << "1. Create Account\n2. Deposit\n3. Withdraw\n4. Transfer Funds\n5. Show Transactions\n6. Exit\n";
        cout << "Enter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            int id;
            string name, mobile, pin;
            float initial;
            cout << "Enter ID: ";
            cin >> id;
            cout << "Enter Name: ";
            cin >> name;
            cout << "Enter Mobile: ";
            cin >> mobile;
            cout << "Set 4-digit PIN: ";
            cin >> pin;
            cout << "Initial Deposit: ";
            cin >> initial;

            Customer cust(id, name, mobile, pin, initial);
            cust.saveToFile();
            txn.addTransaction(id, "DEPOSIT", initial, "Initial", initial);
            cout << "Account Created!\n";
        }

        else if (choice == 2) {
            int id;
            string pin;
            float amount;
            cout << "Enter ID: ";
            cin >> id;
            cout << "Enter PIN: ";
            cin >> pin;
            cout << "Enter Amount: ";
            cin >> amount;

            Customer cust;
            if (!Customer::loadCustomer(id, cust) || !cust.verifyPin(pin)) {
                cout << "Invalid credentials.\n";
                continue;
            }

            cust.account.deposit(amount);
            cust.updateBalanceInFile();
            txn.addTransaction(id, "DEPOSIT", amount, "Self", cust.account.getBalance());
            cout << "Deposited! New Balance: " << cust.account.getBalance() << endl;
        }

        else if (choice == 3) {
            int id;
            string pin;
            float amount;
            cout << "Enter ID: ";
            cin >> id;
            cout << "Enter PIN: ";
            cin >> pin;
            cout << "Enter Amount: ";
            cin >> amount;

            Customer cust;
            if (!Customer::loadCustomer(id, cust) || !cust.verifyPin(pin)) {
                cout << "Invalid credentials.\n";
                continue;
            }

            if (!cust.account.withdraw(amount)) {
                cout << "Insufficient funds.\n";
                continue;
            }

            cust.updateBalanceInFile();
            txn.addTransaction(id, "WITHDRAW", amount, "ATM", cust.account.getBalance());
            cout << "Withdrawn! New Balance: " << cust.account.getBalance() << endl;
        }

        else if (choice == 4) {
            int from, to;
            float amount;
            string pin;
            cout << "Sender ID: ";
            cin >> from;
            cout << "Receiver ID: ";
            cin >> to;
            cout << "Sender PIN: ";
            cin >> pin;
            cout << "Amount: ";
            cin >> amount;

            txn.transferFunds(from, pin, to, amount);
        }

        else if (choice == 5) {
            int id;
            cout << "Enter Account ID: ";
            cin >> id;
            txn.showTransactionHistory(id);
        }

        else if (choice == 6) {
            cout << "Goodbye!\n";
            break;
        }

        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
