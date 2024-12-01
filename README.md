# OS_project-Banking_system-
A Simple Banking System Simulation.
# Banking System Project

This project is a simple banking system implemented in C++. It supports creating accounts, depositing money, withdrawing money, and checking account balances. The project uses multithreading and process forking to handle concurrent transactions and user interactions.

## Project Structure

- `OSproject.cpp`: The main source file containing the implementation of the banking system.

## Classes and Functions

### Account

The `Account` class represents a bank account.

- **Constructor**: `Account(int id, double initial_balance)`
- **Methods**:
  - `int getId() const`
  - `double getBalance() const`
  - `void deposit(double amount)`
  - `bool withdraw(double amount)`

### Bank

The `Bank` class manages multiple accounts.

- **Methods**:
  - `Account *createAccount(int customer_id, double initial_balance)`
  - `Account *getAccount(int account_id)`
  - **Destructor**: `~Bank()`

### Transaction

The `Transaction` class handles various banking operations and manages threads for concurrent transactions.

- **Constructor**: `Transaction(Bank &bank, mutex &print_mtx)`
- **Methods**:
  - `void createAccount(int customer_id, double initial_balance)`
  - `void deposit(int account_id, double amount)`
  - `void withdraw(int account_id, double amount)`
  - `void checkBalance(int account_id)`
  - **Destructor**: `~Transaction()`

### Main Function

The `main` function initializes the `Bank` and `Transaction` objects and forks a child process to handle user interactions through the `menu` function.

### Menu Function

The `menu` function provides a user interface for interacting with the banking system. It supports the following operations:
- Create Account
- Deposit
- Withdraw
- Check Balance
- Exit

## How to Build and Run

1. **Build**: Compile the `OSproject.cpp` file using a C++ compiler.
   ```sh
   g++ -o banking_system OSproject.cpp -lpthread


   2.Run: Execute the compled binary
   ./banking_system
