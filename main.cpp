
/**************************************************************
 *                       bankSim.cpp
g++ main.cpp -std=c++17 -o bankSim
./bankSim
 **************************************************************/

// ==========================
//        Includes
// ==========================
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "nlohmann/json.hpp"

using namespace std;
namespace fs = std::filesystem;


// ==========================
//   Lightweight SHA-256 Implementation (no external dependencies)
// ==========================
typedef unsigned char BYTE;   // 8-bit byte
typedef unsigned int  WORD;   // 32-bit word

const unsigned int SHA256_BLOCK_SIZE = 32; // SHA256 outputs a 32 byte digest

typedef struct {
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[8];
} SHA256_CTX;

#define ROTLEFT(a,b)  ((a << b) | (a >> (32-b)))
#define ROTRIGHT(a,b) ((a >> b) | (a << (32-b)))

#define CH(x,y,z)     ((x & y) ^ (~x & z))
#define MAJ(x,y,z)    ((x & y) ^ (x & z) ^ (y & z))
#define EP0(x)        (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)        (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)       (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ (x >> 3))
#define SIG1(x)       (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ (x >> 10))

static const WORD k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

// SHA-256 core transformation
void sha256_transform(SHA256_CTX *ctx, const BYTE data[]) {
    WORD a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];

    for (i=0,j=0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

// Initialize SHA-256 context
void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

// Update SHA-256 context with input data
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

// Finalize SHA-256 context and output hash
void sha256_final(SHA256_CTX *ctx, BYTE hash[]) {
    int i = ctx->datalen;

    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

// Utility function to hash a string using SHA-256
string sha256(const string& input) {
    BYTE hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)input.c_str(), input.length());
    sha256_final(&ctx, hash);

    stringstream ss;
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// ==========================
//        Class Definitions
// ==========================

// --------------------------
// Account Class
// --------------------------
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
        } else {
            cout << "Invalid deposit amount." << endl;
        }
    }

    // Withdraws a positive amount from the account, if sufficient funds exist.
    void withdraw(double amount) {
        if (amount <= 0) {
            cout << "Invalid withdrawal amount." << endl;
        } else if (amount > balance) {
            cout << "Insufficient funds in account " << accountNumber << "." << endl;
        } else {
            balance -= amount;
            cout << "Withdrew $" << amount << " from account " << accountNumber << endl;
        }
    }

    // Transfers a positive amount to another account, if sufficient funds exist.
    void transferTo(Account& other, double amount) {
        if (amount <= 0) {
            cout << "Invalid transfer amount." << endl;
        } else if (amount > balance) {
            cout << "Insufficient funds in account " << accountNumber << " for transfer." << endl;
        } else {
            balance -= amount;
            other.balance += amount;
            cout << "Transferred $" << amount << " from account " << accountNumber
                 << " to account " << other.accountNumber << "." << endl;
        }
    }

    // Returns the account number.
    int getAccountNumber() const { return accountNumber; }
    // Returns the owner's name.
    string getOwnerName() const { return ownerName; }
    // Returns the current balance.
    double getBalance() const { return balance; }
    // Displays account details.
    void display() const {
        cout << "Account #" << accountNumber << " | Owner: " << ownerName << " | Balance: $" << balance << endl;
    }
};

// --------------------------
// User Class (base)
// --------------------------
// The User class is a base class for customers and employees, storing username and password.
class User {
private:
    string username;
    string password;
public:
    // Constructs a User with a username and password (hashes the password).
    User(string uname, string pword)
        : username(uname), password(sha256(pword)) {}
    // Constructs a User with a username and a password, with option to specify if password is already hashed.
    User(string uname, string pword, bool isHashed)
        : username(uname) {
        password = isHashed ? pword : sha256(pword);
    }
    // Returns the username.
    string getUsername() const { return username; }
    // Checks if the given password matches the user's password (hashes input before comparison).
    bool checkPassword(const string& input) const {
        return sha256(input) == password;
    }
    // Returns the hashed password.
    string getPassword() const { return password; }
    // Virtual destructor for base class.
    virtual ~User() {}
};

// --------------------------
// Customer Class
// --------------------------
// The Customer class represents a bank customer and associates them with an account.
class Customer : public User {
private:
    string accountHash;
public:
    // Constructs a Customer with username, password, and account hash.
    Customer(string uname, string pword, string accHash, bool isHashed = false)
        : User(uname, pword, isHashed), accountHash(accHash) {}
    // Returns the associated account hash.
    string getAccountHash() const { return accountHash; }
};

// --------------------------
// Employee Class
// --------------------------
// The Employee class represents a bank employee with access to view all accounts.
class Employee : public User {
public:
    // Constructs an Employee with username and password.
    Employee(string uname, string pword, bool isHashed = false)
        : User(uname, pword, isHashed) {}
    // Allows employee to view all accounts in the bank.
    void viewAllAccounts(class Bank& bank) const;
};

// --------------------------
// Bank Class
// --------------------------
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
    // Returns a const reference to all accounts.
    const vector<Account>& getAllAccounts() const {
        return account;
    }
    // Saves account data to a file in JSON format (no encryption).
    void saveToFile(const string& filename) {
        // Removed: no longer saving to bank.json
    }
    // Loads account data from a file in JSON format (no encryption).
    void loadFromFile(const string& filename) {
        // Removed: no longer loading from bank.json
    }
    // Loads customers and employees from their respective directories (no encryption).
    void loadUsersFromFile(const string& customerFile, const string& employeeFile) {
        // Load customers from vaults/customers/[username]/profile.json and account.json
        customers.clear();
        account.clear();
        nextAccountNumber = 1000;
        string customersDir = "vaults/customers";
        if (fs::exists(customersDir) && fs::is_directory(customersDir)) {
            for (const auto& entry : fs::directory_iterator(customersDir)) {
                if (fs::is_directory(entry)) {
                    string profilePath = entry.path().string() + "/profile.json";
                    if (fs::exists(profilePath)) {
                        ifstream pf(profilePath);
                        if (pf.is_open()) {
                            nlohmann::json cj;
                            pf >> cj;
                            pf.close();
                            string username = cj.value("username", "");
                            string password = cj.value("password", "");
                            string accNumHash = cj.value("accountNumber", "");
                            customers.emplace_back(username, password, accNumHash, true);
                            // Load account.json for this customer
                            string accountPath = entry.path().string() + "/account.json";
                            if (fs::exists(accountPath)) {
                                ifstream af(accountPath);
                                if (af.is_open()) {
                                    nlohmann::json aj;
                                    af >> aj;
                                    af.close();
                                    string name = aj.value("name", "");
                                    int number = aj.value("number", 0);
                                    double balance = aj.value("balance", 0.0);
                                    Account acc(name, number, balance);
                                    account.push_back(acc);
                                    if (number >= nextAccountNumber) {
                                        nextAccountNumber = number + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // Load employees from vaults/employees/[username]/profile.json and account.json (if needed)
        employees.clear();
        string employeesDir = "vaults/employees";
        if (fs::exists(employeesDir) && fs::is_directory(employeesDir)) {
            for (const auto& entry : fs::directory_iterator(employeesDir)) {
                if (fs::is_directory(entry)) {
                    string profilePath = entry.path().string() + "/profile.json";
                    if (fs::exists(profilePath)) {
                        ifstream pf(profilePath);
                        if (pf.is_open()) {
                            nlohmann::json ej;
                            pf >> ej;
                            pf.close();
                            string username = ej.value("username", "");
                            string password = ej.value("password", "");
                            employees.emplace_back(username, password, true);
                            // Optionally, load account.json for employees if you want to store their accounts too
                            string accountPath = entry.path().string() + "/account.json";
                            if (fs::exists(accountPath)) {
                                ifstream af(accountPath);
                                if (af.is_open()) {
                                    nlohmann::json aj;
                                    af >> aj;
                                    af.close();
                                    string name = aj.value("name", "");
                                    int number = aj.value("number", 0);
                                    double balance = aj.value("balance", 0.0);
                                    Account acc(name, number, balance);
                                    account.push_back(acc);
                                    if (number >= nextAccountNumber) {
                                        nextAccountNumber = number + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Saves customers and employees to their respective directories (no encryption).
    void saveUsersToFile(const string& customerFile, const string& employeeFile) {
        // Save customers and their account.json
        for (const auto& c : customers) {
            string custDir = "vaults/customers/" + c.getUsername();
            fs::create_directories(custDir);
            nlohmann::json cj = {
                {"username", c.getUsername()},
                {"password", c.getPassword()},
                {"accountNumber", c.getAccountHash()}
            };
            ofstream pf(custDir + "/profile.json");
            pf << cj.dump(4);
            pf.close();
            // Save account.json for this customer
            // Find their account by matching the hash
            for (const auto& acc : account) {
                if (sha256(to_string(acc.getAccountNumber())) == c.getAccountHash()) {
                    nlohmann::json aj = {
                        {"name", acc.getOwnerName()},
                        {"number", acc.getAccountNumber()},
                        {"balance", acc.getBalance()}
                    };
                    ofstream af(custDir + "/account.json");
                    af << aj.dump(4);
                    af.close();
                    break;
                }
            }
        }
        // Save employees and their account.json (if any)
        for (const auto& e : employees) {
            string empDir = "vaults/employees/" + e.getUsername();
            fs::create_directories(empDir);
            nlohmann::json ej = {
                {"username", e.getUsername()},
                {"password", e.getPassword()}
            };
            ofstream pf(empDir + "/profile.json");
            pf << ej.dump(4);
            pf.close();
            // Save account.json if this employee has an account (optional)
            // This block is only meaningful if you want to store employee-owned accounts
            for (const auto& acc : account) {
                if (acc.getOwnerName() == e.getUsername()) {
                    nlohmann::json aj = {
                        {"name", acc.getOwnerName()},
                        {"number", acc.getAccountNumber()},
                        {"balance", acc.getBalance()}
                    };
                    ofstream af(empDir + "/account.json");
                    af << aj.dump(4);
                    af.close();
                    break;
                }
            }
        }
    }
};

// Allows an employee to view all accounts in the bank.
inline void Employee::viewAllAccounts(Bank& bank) const {
    bank.showAllAccounts();
}

// ==========================================================
//                        MAIN FUNCTION
// ==========================================================
// Entry point for Cherrington Bank simulation.
int main() {
    Bank account;
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
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl <<"Invalid input. Please try again." << endl;
            cout << endl;
            continue;
        }

        // Handle invalid main menu choice
        if (employeeOrCustomerChoice != 'e' && employeeOrCustomerChoice != 'E'
            && employeeOrCustomerChoice != 'c' && employeeOrCustomerChoice != 'C'
            && employeeOrCustomerChoice != 'x' && employeeOrCustomerChoice != 'X') {
            cout << endl;
            cout << "Invalid Choice." << endl;
            cout << endl;
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
            cout << endl << "Are you a returning customer or a new customer?" << endl;
            cout << "(n) New" << endl;
            cout << "(r) Returning" << endl;
            cout << "--->";
            cin >> newOrReturningChoice;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << endl;
                cout << "Invalid input. Please try again." << endl;
                cout << endl;
                continue;
            }

            // Handle invalid customer menu choice
            if (newOrReturningChoice != 'n' && newOrReturningChoice != 'N' &&
                newOrReturningChoice != 'r' && newOrReturningChoice != 'R') {
                cout << endl;
                cout << "Invalid Choice." << endl << endl;
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
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << endl;
                    cout << "Invalid input. Please try again." << endl;
                    cout << endl;
                    continue;
                }

                cout << endl << "New customer account created successfully!" << endl << endl;

                int accountNum = account.addAccount(newCustomerName, newCustomerDeposit);
                // Hash the account number and add the new customer
                string accountHash = sha256(to_string(accountNum));
                Customer newCustomer(newUsername, newPassword, accountHash);
                account.addCustomer(newCustomer);
                // Create directory and profile.json for new customer
                string custDir = "vaults/customers/" + newUsername;
                fs::create_directories(custDir);
                nlohmann::json cj = {
                    {"username", newUsername},
                    {"password", sha256(newPassword)},
                    {"accountNumber", accountHash}
                };
                ofstream pf(custDir + "/profile.json");
                pf << cj.dump(4);
                pf.close();
                cout << endl << "Account number: " << accountNum << endl;
                cout << "Account Name: " << newCustomerName << endl;
                cout << "Current Balance: " << newCustomerDeposit << endl << endl;
                cout << "Retuning to main menu...." << endl << endl;
                // Removed: account.saveToFile("vaults/bank.json");
                account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");
            }
            // Returning customer login and actions
            else if (newOrReturningChoice == 'r' || newOrReturningChoice == 'R') {
                string username, password;
                cout << endl << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;

                int returningAccountNumber = -1;
                bool loginSuccess = false;
                // Authenticate customer
                for (const auto& cust : account.getCustomers()) {
                    if (cust.getUsername() == username && cust.checkPassword(password)) {
                        // Find the account number by matching the hash
                        for (const auto& acc : account.getAllAccounts()) {
                            if (sha256(to_string(acc.getAccountNumber())) == cust.getAccountHash()) {
                                returningAccountNumber = acc.getAccountNumber();
                                break;
                            }
                        }
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
                        if (cin.fail()) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << endl;
                            cout << "Invalid input. Please try again." << endl;
                            cout << endl;
                            continue;
                        }
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
                            if (cin.fail()) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << endl;
                                cout << "Invalid input. Please try again." << endl;
                                cout << endl;
                                continue;
                            }
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
                            if (cin.fail()) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << endl;
                                cout << "Invalid input. Please try again." << endl;
                                cout << endl;
                                continue;
                            }
                            cout << endl;
                            found->withdraw(wd);
                            found->display();
                            continue;
                        }
                        // Transfer funds to another account
                        else if (customerMenuChoice == 't' || customerMenuChoice == 'T') {
                            int recipientNumber = 0;
                            double transferAmount = 0;

                            cout << "How much would you like to transfer? :$";
                            cin >> transferAmount;
                            if (cin.fail()) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << endl;
                                cout << "Invalid input. Please try again." << endl;
                                cout << endl;
                                continue;
                            }

                            cout << "To which account number? :";
                            cin >> recipientNumber;
                            if (cin.fail()) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << "Invalid input. Please try again." << endl;
                                continue;
                            }

                            Account* recipientAccount = account.findAccount(recipientNumber);

                            if (recipientAccount != nullptr) {
                                cout << endl;
                                found->transferTo(*recipientAccount, transferAmount);
                                found->display();
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
            cout << endl << "Are you a new or existing employee?" << endl;
            cout << "(n) New" << endl;
            cout << "(r) Returning" << endl;
            cout << "---> ";
            cin >> newOrExisting;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << endl;
                cout << "Invalid input. Please try again." << endl;
                cout << endl;
                continue;
            }

            // New employee registration
            if (newOrExisting == 'n' || newOrExisting == 'N') {
                string newUsername, newPassword;
                cout << "Create a username: ";
                cin >> newUsername;
                cout << "Create a password: ";
                cin >> newPassword;

                Employee newEmployee(newUsername, newPassword);
                account.addEmployee(newEmployee);
                // Create directory and profile.json for new employee
                string empDir = "vaults/employees/" + newUsername;
                fs::create_directories(empDir);
                nlohmann::json ej = {
                    {"username", newUsername},
                    {"password", sha256(newPassword)}
                };
                ofstream pf(empDir + "/profile.json");
                pf << ej.dump(4);
                pf.close();
                account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");

                cout << endl << "New employee account created successfully!" << endl << endl;
                continue;
            }

            // Returning employee login and actions
            string username, password;
            cout << endl << "Enter username: ";
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
                cout << endl;
                cout << "INVALID LOGIN" << endl;
                cout << endl;
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
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid input. Please try again." << endl;
                    continue;
                }
                cout << endl;

                // Handle invalid employee action menu choice
                if (employeeChoice != 's' && employeeChoice != 'S' &&
                    employeeChoice != 'r' && employeeChoice != 'R') {
                    cout << endl;
                    cout << "INVALID CHOICE" << endl;
                    cout << endl;
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
                    cout << "Returning to main menu...." << endl << endl;
                    break;
                }
            }
        }
    }
    // Save all data before exiting
    // Removed: account.saveToFile("vaults/bank.json");
    account.saveUsersToFile("vaults/customers.json", "vaults/employees.json");
    return 0;
}