# Cherrington BankSim

A command-line banking simulator built in C++ using object-oriented programming principles. BankSim allows users to create and manage bank accounts, simulate employee and customer roles, and handle persistent data using JSON files.

## Features

- Account creation with username, password, and initial deposit
- SHA-256 password hashing for secure user authentication
- Secure login for both employees and customers
- Deposit, withdrawal, and transfer functionality
- Employee view of all customer accounts
- Persistent storage of account and user data via JSON
- Clean, modular code with proper encapsulation and inheritance
- Full-field encryption using SHA-256: all usernames, passwords, account numbers, balances, and owner names are hashed for secure storage

## Structure

### Classes

- `Account`: Handles basic account operations like deposit, withdrawal, and balance tracking
- `User`: Abstract base class for shared user attributes like username and password; includes SHA-256 hashing for credentials
- `Customer`: Inherits from `User`, linked to a specific bank account
- `Employee`: Inherits from `User`, can view all accounts
- `Bank`: Manages all customer and employee data, account management, and file I/O

## File Organization

- `main.cpp`: Contains all class definitions and the program's main logic
- `vaults/bank.json`: Stores SHA-256 hashed account data
- `vaults/customers.json`: Stores hashed customer login data
- `vaults/employees.json`: Stores hashed employee login data

## Usage

1. Compile:
   ```
   g++ main.cpp -std=c++17 -o bankSim
   ```

2. Run:
   ```
   ./bankSim
   ```

3. Follow on-screen prompts to:
   - Create a new account (with encrypted fields)
   - Log in as a customer or employee (verified via hashed credentials)
   - Perform account actions (deposit, withdraw, transfer)
   - View accounts (employee only; encrypted data until login)

## Future Enhancements

- Admin role with extended permissions
- Transaction history logs
- UI improvements (e.g. color formatting)

## Author

Cody Cherrington