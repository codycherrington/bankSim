# Cherrington BankSim

A command-line banking simulator built in C++ using object-oriented programming principles. BankSim allows users to create and manage bank accounts, simulate employee and customer roles, and handle persistent data using JSON files.

## Features

- Customer-managed account creation
- Per-user directories and encrypted profile/account files
- SHA-256 password hashing for all logins
- Employee access to account summaries and individual account details
- Customer access to deposit, withdraw, transfer, and balance check
- JSON-based persistent storage in structured vault directories
- Secure and modular architecture using inheritance and encapsulation

## Structure

### Classes

- `Account`: Handles basic account operations like deposit, withdrawal, and balance tracking
- `User`: Abstract base class for shared user attributes like username and password; includes SHA-256 hashing for credentials
- `Customer`: Inherits from `User`, linked to a specific bank account
- `Employee`: Inherits from `User`, can view all accounts
- `Bank`: Manages all customer and employee data, account management, and file I/O

## File Organization

- `main.cpp`: All class definitions and the main logic
- `vaults/customers/[username]/`: Each customer's encrypted profile and account data
- `vaults/employees/[username]/`: Each employee's encrypted profile

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
   - Log in or register as a customer and manage your account
   - Register or log in as an employee to view account summaries and balances

## Future Enhancements

- Admin role with extended permissions
- Transaction history logs
- UI improvements (e.g. color formatting)
- Automatic employee-customer account linking and audit trails

## Author

Cody Cherrington