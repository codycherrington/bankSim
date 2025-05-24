/* g++ main.cpp -std=c++17 -o bankSim
./bankSim*/

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using namespace std;

namespace fs = std::filesystem;

// The Account class represents a bank account with basic operations like deposit, withdraw, and transfer.
class Account {
private:
    string ownerName;
    int accountNumber;
    double balance;

public:
    // Constructs an Account with the owner's name, account number, and initial deposit.
    Account(string name, int accNumber, double initialDeposit) {
        ownerName = name;
        accountNumber = accNumber;
        balance = initialDeposit;
    }

    // Deposits a positive amount into the account.
    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            cout << "Deposited $" << amount << " to account " << accountNumber << endl;
        }
        else {
            cout << "Invalid deposit amount." << endl;
        }
    }

    // Withdraws a positive amount from the account, if sufficient funds exist.
    void withdraw(double amount) {
        if (amount <= 0) {
            cout << "Invalid withdrawal amount." << endl;
        }
        else if (amount > balance) {
            cout << "Insufficient funds in account " << accountNumber << "." << endl;
        }
        else {
            balance -= amount;
            cout << "Withdrew $" << amount << " from account " << accountNumber << endl;
        }
    }

    // Transfers a positive amount to another account, if sufficient funds exist.
    void transferTo(Account& other, double amount) {
        if (amount <= 0) {
            cout << "Invalid transfer amount." << endl;
        }
        else if (amount > balance) {
            cout << "Insufficient funds in account " << accountNumber << " for transfer." << endl;
        }
        else {
            balance -= amount;
            other.balance += amount;
            cout << "Transferred $" << amount << " from account " << accountNumber
                << " to account " << other.accountNumber << "." << endl;
        }
    }

    // Returns the account number.
    int getAccountNumber() const {
        return accountNumber;
    }

    // Returns the owner's name.
    string getOwnerName() const {
        return ownerName;
    }

    // Returns the current balance.
    double getBalance() const {
        return balance;
    }

    // Displays account details.
    void display() const {
        cout << "Account #" << accountNumber << " | Owner: " << ownerName << " | Balance: $" << balance << endl;
    }
};

// The User class is a base class for customers and employees, storing username and password.
class User {
private:
    string username;
    string password;

public:
    // Constructs a User with a username and password.
    User(string uname, string pword) : username(uname), password(pword) {}

    // Returns the username.
    string getUsername() const { 
        return username; 
    }

    // Checks if the given password matches the user's password.
    bool checkPassword(const string& input) const { 
        return input == password; 
    }

    // Returns the password.
    string getPassword() const { 
        return password; 
    }

    // Virtual destructor for base class.
    virtual ~User() {}
};

// The Customer class represents a bank customer and associates them with an account.
class Customer : public User {
private:
    int accountNumber;

public:
    // Constructs a Customer with username, password, and account number.
    Customer(string uname, string pword, int accNum)
        : User(uname, pword), accountNumber(accNum) {}

    // Returns the associated account number.
    int getAccountNumber() const { 
        return accountNumber; 
    }
};

// The Employee class represents a bank employee with access to view all accounts.
class Employee : public User {
public:
    // Constructs an Employee with username and password.
    Employee(string uname, string pword)
        : User(uname, pword) {}

    // Allows employee to view all accounts in the bank.
    void viewAllAccounts(class Bank& bank) const;
};

// The Bank class manages accounts, customers, and employees, and handles persistence.
class Bank {
private:
    vector<Account> account;
    int nextAccountNumber = 1000;
    vector<Customer> customers;
    vector<Employee> employees;

public:
    // Adds a customer to the bank.
    void addCustomer(const Customer& customer) {
        customers.push_back(customer);
    }

    // Adds an employee to the bank.
    void addEmployee(const Employee& employee) {
        employees.push_back(employee);
    }

    // Returns a const reference to the list of customers.
    const vector<Customer>& getCustomers() const {
        return customers;
    }

    // Returns a const reference to the list of employees.
    const vector<Employee>& getEmployees() const {
        return employees;
    }

    // Adds a new account with the given name and initial deposit, returns new account number.
    int addAccount(const string& name, double initialDeposit) {
        int newAccountNumber = nextAccountNumber++;
        Account newAccount(name, newAccountNumber, initialDeposit);
        account.push_back(newAccount);
        return newAccountNumber;
    }

    // Finds and returns a pointer to an account by account number, or nullptr if not found.
    Account* findAccount(int accountNum) {
        for (int i = 0; i < account.size(); i++) {
            if (account[i].getAccountNumber() == accountNum) {
                return &account[i];
            }
        }
        cout << "NO ACCOUNT FOUND" << endl;
        return nullptr;
    }

    // Displays all accounts in the bank.
    void showAllAccounts() const {
        for (int i = 0; i < account.size(); i++) {
            account[i].display();
        }
    }

    // Saves account data to a file in JSON format.
    void saveToFile(const string& filename) {
        fs::create_directories("vaults");
        nlohmann::json j;
        for (const auto& acc : account) {
            j.push_back({
                {"name", acc.getOwnerName()},
                {"number", acc.getAccountNumber()},
                {"balance", acc.getBalance()}
            });
        }
        ofstream file(filename);
        file << j.dump(4);
        file.close();
    }

    // Loads account data from a file in JSON format.
    void loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) return;
        nlohmann::json j;
        file >> j;
        file.close();
        for (const auto& item : j) {
            string name = item.at("name");
            int number = item.at("number");
            double balance = item.at("balance");
            Account acc(name, number, balance);
            account.push_back(acc);
            if (number >= nextAccountNumber) {
                nextAccountNumber = number + 1;
            }
        }
    }

    // Loads customers and employees from their respective files.
    void loadUsersFromFile(const string& customerFile, const string& employeeFile) {
        // Load customers
        ifstream cFile(customerFile);
        if (cFile.is_open()) {
            nlohmann::json cj;
            cFile >> cj;
            cFile.close();
            for (const auto& item : cj) {
                string username = item.at("username");
                string password = item.at("password");
                int accNum = item.at("accountNumber");
                customers.emplace_back(username, password, accNum);
            }
        }

        // Load employees
        ifstream eFile(employeeFile);
        if (eFile.is_open()) {
            nlohmann::json ej;
            eFile >> ej;
            eFile.close();
            for (const auto& item : ej) {
                string username = item.at("username");
                string password = item.at("password");
                employees.emplace_back(username, password);
            }
        }
    }

    // Saves customers and employees to their respective files.
    void saveUsersToFile(const string& customerFile, const string& employeeFile) {
        nlohmann::json cj;
        for (const auto& c : customers) {
            cj.push_back({
                {"username", c.getUsername()},
                {"password", c.getPassword()},
                {"accountNumber", c.getAccountNumber()}
            });
        }
        ofstream cOut(customerFile);
        cOut << cj.dump(4);
        cOut.close();

        nlohmann::json ej;
        for (const auto& e : employees) {
            ej.push_back({
                {"username", e.getUsername()},
                {"password", e.getPassword()}
            });
        }
        ofstream eOut(employeeFile);
        eOut << ej.dump(4);
        eOut.close();
    }
};

// Allows an employee to view all accounts in the bank.
inline void Employee::viewAllAccounts(Bank& bank) const {
    bank.showAllAccounts();
}

int main() {
    Bank account;
    account.loadFromFile("vaults/bank.json");
    account.loadUsersFromFile("vaults/customers.json", "vaults/employees.json");

    while (true) {
        // Main menu: Prompt for employee or customer or exit
        char employeeOrCustomerChoice = ' ';
        cout << "Welcome to Cherrington Bank!" << endl;
        cout << "Are you an employee or a customer?:" << endl;
        cout << "(e) Employee" << endl;
        cout << "(c) Customer" << endl;
        cout << "(x) to exit" << endl;
        cout << "---> ";
        cin >> employeeOrCustomerChoice;
        cout << endl;

        // Handle invalid main menu choice
        if (employeeOrCustomerChoice != 'e' && employeeOrCustomerChoice != 'E'
            && employeeOrCustomerChoice != 'c' && employeeOrCustomerChoice != 'C'
            && employeeOrCustomerChoice != 'x' && employeeOrCustomerChoice != 'X') {
            cout << "Invalid Choice." << endl;
        }
        // Exit program
        else if (employeeOrCustomerChoice == 'x' || employeeOrCustomerChoice == 'X') {
            cout << "Thank you for choosing Cherrington Bank!" << endl;
            break;
        }
        // Customer menu branch
        else if (employeeOrCustomerChoice == 'c' || employeeOrCustomerChoice == 'C') {
            // Prompt for new or returning customer
            char newOrReturningChoice = ' ';
            cout << "Are you a returning customer or a new customer?" << endl;
            cout << "(n) New" << endl;
            cout << "(r) Returning" << endl;
            cout << "--->";
            cin >> newOrReturningChoice;

            // Handle invalid customer menu choice
            if (newOrReturningChoice != 'n' && newOrReturningChoice != 'N' &&
                newOrReturningChoice != 'r' && newOrReturningChoice != 'R') {
                cout << "Invalid Choice." << endl;
            }
            // New customer registration
            else if (newOrReturningChoice == 'n' || newOrReturningChoice == 'N') {
                string newCustomerName = " ";
                cout << endl << "What is your full name? : ";
                cin.ignore();
                getline(cin, newCustomerName);

                string newUsername, newPassword;
                cout << "Create a username: ";
                cin >> newUsername;
                cout << "Create a password: ";
                cin >> newPassword;

                double newCustomerDeposit = 0;
                cout << "What is your initial deposit?: ";
                cin >> newCustomerDeposit;

                cout << endl << "New customer account created successfully!" << endl << endl;

                int accountNum = account.addAccount(newCustomerName, newCustomerDeposit);
                // Add the new customer to the customers vector
                Customer newCustomer(newUsername, newPassword, accountNum);
                account.addCustomer(newCustomer);
                cout << endl << "Account number: " << accountNum << endl;
                cout << "Account Name: " << newCustomerName << endl;
                cout << "Current Balance: " << newCustomerDeposit << endl << endl;
                cout << "Retuning to main menu...." << endl << endl;
                account.saveToFile("vaults/bank.json");
                account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");
            }
            // Returning customer login and actions
            else if (newOrReturningChoice == 'r' || newOrReturningChoice == 'R') {
                string username, password;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;

                int returningAccountNumber = -1;
                bool loginSuccess = false;
                // Authenticate customer
                for (const auto& cust : account.getCustomers()) {
                    if (cust.getUsername() == username && cust.checkPassword(password)) {
                        returningAccountNumber = cust.getAccountNumber();
                        loginSuccess = true;
                        break;
                    }
                }
                if (!loginSuccess) {
                    cout << endl << "INVALID LOGIN" << endl << endl;
                    continue;
                }
                cout << endl;

                Account* found = account.findAccount(returningAccountNumber);

                if (found != nullptr) {
                    cout << "Welcome " << found->getOwnerName() << endl;
                    // Customer action menu
                    while (true) {
                        char customerMenuChoice = ' ';
                        cout << endl << "Customer Menu: " << endl;
                        cout << "(c) Check Balance" << endl;
                        cout << "(d) Deposit" << endl;
                        cout << "(w) Withdrawal" << endl;
                        cout << "(t) Tansfer" << endl;
                        cout << "(r) Return to main menu" << endl;
                        cout << "--->";
                        cin >> customerMenuChoice;
                        cout << endl;

                        // Handle invalid customer action menu choice
                        if (customerMenuChoice != 'c' && customerMenuChoice != 'C'
                            && customerMenuChoice != 'd' && customerMenuChoice != 'D'
                            && customerMenuChoice != 'w' && customerMenuChoice != 'W'
                            && customerMenuChoice != 't' && customerMenuChoice != 'T'
                            && customerMenuChoice != 'r' && customerMenuChoice != 'R') {
                            cout << "Invalid Choice." << endl;
                        }
                        // Check balance
                        else if (customerMenuChoice == 'c' || customerMenuChoice == 'C') {
                            found->display();
                            continue;
                        }
                        // Deposit funds
                        else if (customerMenuChoice == 'd' || customerMenuChoice == 'D') {
                            int dep = 0;
                            cout << "How much would you like to deposit?: ";
                            cin >> dep;
                            cout << endl;
                            found->deposit(dep);
                            found->display();
                            continue;
                        }
                        // Withdraw funds
                        else if (customerMenuChoice == 'w' || customerMenuChoice == 'W') {
                            int wd = 0;
                            cout << "How much would you like to withdraw?: ";
                            cin >> wd;
                            cout << endl;
                            found->withdraw(wd);
                            cout << endl;
                            found->display();
                            continue;
                        }
                        // Transfer funds to another account
                        else if (customerMenuChoice == 't' || customerMenuChoice == 'T') {
                            int recipientNumber = 0;
                            double transferAmount = 0;

                            cout << "How much would you like to transfer? :$";
                            cin >> transferAmount;

                            cout << "To which account number? :";
                            cin >> recipientNumber;

                            Account* recipientAccount = account.findAccount(recipientNumber);

                            if (recipientAccount != nullptr) {
                                found->transferTo(*recipientAccount, transferAmount);
                                found->display();
                                recipientAccount->display();
                            }
                            else {
                                cout << "Transfer failed: recipient account not found." << endl;
                            }
                        }
                        // Return to main menu
                        else if (customerMenuChoice == 'r' || customerMenuChoice == 'R') {
                            cout << "Returning to main menu...." << endl << endl;
                            break;
                        }
                    }
                }
            }
        }
        // Employee menu branch
        else if (employeeOrCustomerChoice == 'e' || employeeOrCustomerChoice == 'E') {
            // Prompt for new or returning employee
            char newOrExisting = ' ';
            cout << "Are you a new or existing employee?" << endl;
            cout << "(n) New" << endl;
            cout << "(r) Returning" << endl;
            cout << "---> ";
            cin >> newOrExisting;

            // New employee registration
            if (newOrExisting == 'n' || newOrExisting == 'N') {
                string newUsername, newPassword;
                cout << "Create a username: ";
                cin >> newUsername;
                cout << "Create a password: ";
                cin >> newPassword;

                Employee newEmployee(newUsername, newPassword);
                account.addEmployee(newEmployee);
                account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");

                cout << endl << "New employee account created successfully!" << endl << endl;
                continue;
            }

            // Returning employee login and actions
            string username, password;
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;
            cout << endl;

            bool loginSuccess = false;
            // Authenticate employee
            for (const auto& emp : account.getEmployees()) {
                if (emp.getUsername() == username && emp.checkPassword(password)) {
                    loginSuccess = true;
                    break;
                }
            }

            if (!loginSuccess) {
                cout << endl << "INVALID LOGIN" << endl << endl;
                continue;
            }

            // Employee action menu
            while (true) {
                char employeeChoice = ' ';
                cout << "Employee Menu: " << endl;
                cout << "(s) Show all accounts" << endl;
                cout << "(r) Return to main menu" << endl;
                cout << "--->";
                cin >> employeeChoice;
                cout << endl;

                // Handle invalid employee action menu choice
                if (employeeChoice != 's' && employeeChoice != 'S' &&
                    employeeChoice != 'r' && employeeChoice != 'R') {
                    cout << endl << "INVALID CHOICE" << endl;
                    continue;
                }
                // Show all accounts
                if (employeeChoice == 's' || employeeChoice == 'S') {
                    account.showAllAccounts();
                    cout << endl;
                    continue;
                }
                // Return to main menu
                if (employeeChoice == 'r' || employeeChoice == 'R') {
                    cout << endl << "Returning to main menu...." << endl;
                    break;
                }
            }
        }
    }
    // Save all data before exiting
    account.saveToFile("vaults/bank.json");
    account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");
    return 0;
}