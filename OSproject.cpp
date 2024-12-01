#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>
#include <sys/wait.h>

using namespace std;

class Account {
private:
    int id;
    double balance;
    mutable mutex acc_mtx;

public:
    Account(int id, double initial_balance)
        : id(id), balance(initial_balance) {
    }

    int getId() const {
        return id;
    }

    double getBalance() const {
        lock_guard<mutex> lock(acc_mtx);
        return balance;
    }

    // directly deposit amount to account
    void deposit(double amount) {
        // lock the Account Mutex
        lock_guard<mutex> lock(acc_mtx);
        // Mutex will auto-unlocked when lock goes out of scope
        balance += amount;
    }

    // directly withdraw amount from account
    bool withdraw(double amount) {
        lock_guard<mutex> lock(acc_mtx);
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }
};

class Bank {
private:
    // Map each customer ID to an Account
    unordered_map<int, Account *> accounts;
    mutex bank_mtx;

public:
    Account *createAccount(int customer_id, double initial_balance) {
        // Account is Created on HEAP, and it's pointer is returned
        lock_guard<mutex> lock(bank_mtx);
        Account *account = new Account(customer_id, initial_balance);
        accounts[customer_id] = account;
        return account;
    }

    Account *getAccount(int account_id) {
        // Return Account pointer if found, else NULL
        lock_guard<mutex> lock(bank_mtx);
        auto it = accounts.find(account_id);
        if (it != accounts.end()) {
            return it->second;
        }
        return NULL;
    }


    // free up account space
    ~Bank() {
        for (auto &pair: accounts) {
            delete pair.second;
        }
    }
};

class Transaction {
private:
    Bank &bank;
    // keep track or running threads
    vector<thread> threads;
    mutex &print_mtx;

public:
    Transaction(Bank &bank, mutex &print_mtx) : bank(bank), print_mtx(print_mtx) {
    }

    void createAccount(int customer_id, double initial_balance) {
        threads.push_back(thread([this, customer_id, initial_balance]() {
            {
                lock_guard<mutex> lock(print_mtx);
                cout << "\033[1;34m::thread " << this_thread::get_id() << " ::\033[0m\n";
                // Mutex will auto-unlocked when lock goes out of scope
            }
            bank.createAccount(customer_id, initial_balance); {
                lock_guard<mutex> lock(print_mtx);
                cout << "Account created for customer " << customer_id
                        << " with initial balance " << initial_balance << "\n";
            }
        }));
    }

    // manage deposit process and thread creation
    void deposit(int account_id, double amount) {
        threads.push_back(thread([this, account_id, amount]() {
            {
                lock_guard<mutex> lock(print_mtx);
                cout << "\033[1;34m::thread " << this_thread::get_id() << " ::\033[0m\n";
            }

            auto account = bank.getAccount(account_id);
            if (account) {
                account->deposit(amount); {
                    lock_guard<mutex> lock(print_mtx);
                    cout << "Deposited " << amount << " to account " << account_id << endl;
                }
            }
            // error handling
            else {
                {
                    lock_guard<mutex> lock(print_mtx);
                    cout << "\033[1;31mAccount " << account_id << " not found\033[0m" << endl;
                }
            }
        }));
    }

    // manage withdraw process inc thread creation
    void withdraw(int account_id, double amount) {
        threads.push_back(thread([this, account_id, amount]() {
            {
                lock_guard<mutex> lock(print_mtx);
                cout << "\033[1;34m::thread " << this_thread::get_id() << " ::\033[0m\n";
            }

            auto account = bank.getAccount(account_id);
            if (account) {
                if (account->withdraw(amount)) {
                    {
                        lock_guard<mutex> lock(print_mtx);
                        cout << "Withdrew " << amount << " from account " << account_id << endl;
                    }
                } else {
                    {
                        lock_guard<mutex> lock(print_mtx);
                        cout << "\033[1;31mInsufficient balance in account " << account_id << "\033[0m" << endl;
                    }
                }
            }
            // error handling
            else {
                {
                    lock_guard<mutex> lock(print_mtx);
                    cout << "\033[1;31mAccount " << account_id << " not found\033[0m" << endl;
                }
            }
        }));
    }

    // manage check balance process inc thread creation
    void checkBalance(int account_id) {
        threads.push_back(thread([this, account_id]() {
            {
                lock_guard<mutex> lock(print_mtx);
                cout << "\033[1;34m::thread " << this_thread::get_id() << " ::\033[0m\n";
            }

            auto account = bank.getAccount(account_id);
            if (account) {
                {
                    lock_guard<mutex> lock(print_mtx);
                    cout << "Balance for account " << account_id << " is " << account->getBalance() << endl;
                }
            } else {
                {
                    lock_guard<mutex> lock(print_mtx);
                    cout << "\033[1;31mAccount " << account_id << " not found\033[0m" << endl;
                }
            }
        }));
    }

    ~Transaction() {
        for (auto &t: threads) {
            // make sure each thread complete its execution before being destroyed
            if (t.joinable()) {
                t.join();
            }
        }
    }
};

void menu(Transaction &transaction, mutex &print_mtx) {
    int choice, account_id, customer_id;
    double amount, initial_balance;

    while (true) {
        {
            lock_guard lock(print_mtx);
            cout << "\n----------------------------------------";
            cout << "\nBanking System Menu:\n";
            cout << "1. Create Account\n";
            cout << "2. Deposit\n";
            cout << "3. Withdraw\n";
            cout << "4. Check Balance\n";
            cout << "5. Exit\n";
            cout << "----------------------------------------\n";
            cout << "Enter your choice > ";
            cin >> choice;
        }
        switch (choice) {
            case 1:
                cout << "Enter customer ID: ";
                cin >> customer_id;
                cout << "Enter initial balance: ";
                cin >> initial_balance;
                transaction.createAccount(customer_id, initial_balance);
                break;
            case 2:
                cout << "Enter account ID: ";
                cin >> account_id;
                cout << "Enter amount to deposit: ";
                cin >> amount;
                transaction.deposit(account_id, amount);
                break;
            case 3:
                cout << "Enter account ID: ";
                cin >> account_id;
                cout << "Enter amount to withdraw: ";
                cin >> amount;
                transaction.withdraw(account_id, amount);
                break;
            case 4:
                cout << "Enter account ID: ";
                cin >> account_id;
                transaction.checkBalance(account_id);
                break;
            case 5:
                return;
            default:
                cout << "\033[1;31mInvalid choice. Please try again.\033[0m\n";
        }
    }
}

int main() {
    Bank bank;
    mutex print_mtx;
    Transaction transaction(bank, print_mtx);

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        menu(transaction, print_mtx);
        exit(0);
    } else if (pid > 0) {
        // Parent process
        wait(NULL); // Wait for the child process to finish
        cout << "Child process finished.\n";
    } else {
        // Fork failed
        perror("fork");
        return 1;
    }

    return 0;
}