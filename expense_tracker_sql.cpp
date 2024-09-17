#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>

extern "C" {
#include "sqlite3.h"
}

// Global database pointer
sqlite3* db;
char* errMessage = 0;

// Callback function to display query results
static int callback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    std::cout << std::endl;
    return 0;
}

// Function to execute SQL commands
void executeSQL(const std::string& sql) {
    int rc = sqlite3_exec(db, sql.c_str(), callback, 0, &errMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    }
}

// Function to initialize the database
void initializeDatabase() {
    int rc = sqlite3_open("expense_tracker.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    } else {
        std::cout << "Opened database successfully." << std::endl;
    }

    // Create the table with a Date column if it does not exist
    std::string createTableSQL = "CREATE TABLE IF NOT EXISTS Expenses ("
                                 "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                                 "Category TEXT NOT NULL, "
                                 "Amount REAL NOT NULL, "
                                 "Description TEXT, "
                                 "Date TEXT NOT NULL);";
    executeSQL(createTableSQL);

    // Alter the table to add the Date column if it is missing
    std::string addDateColumnSQL = "ALTER TABLE Expenses ADD COLUMN Date TEXT;";
    executeSQL(addDateColumnSQL);  // This will run only if the Date column is missing
}

// Function to get the current date as a string
std::string getCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::ostringstream oss;
    oss << (now->tm_year + 1900) << '-'
        << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '-'
        << std::setw(2) << std::setfill('0') << now->tm_mday;
    return oss.str();
}

// Function to add an expense to the database
void addExpenseToDatabase(const std::string& category, double amount, const std::string& description) {
    std::string date = getCurrentDate();
    std::string insertSQL = "INSERT INTO Expenses (Category, Amount, Description, Date) VALUES ('" +
                            category + "', " + std::to_string(amount) + ", '" + description + "', '" + date + "');";
    executeSQL(insertSQL);
}

// Function to display expenses from the database
void displayExpensesFromDatabase() {
    std::string selectSQL = "SELECT * FROM Expenses;";
    executeSQL(selectSQL);
}

// Function to delete an expense from the database
void deleteExpenseFromDatabase(int id) {
    std::string deleteSQL = "DELETE FROM Expenses WHERE ID = " + std::to_string(id) + ";";
    executeSQL(deleteSQL);
}

// Function to update an expense in the database
void updateExpenseInDatabase(int id, const std::string& category, double amount, const std::string& description) {
    std::string updateSQL = "UPDATE Expenses SET Category = '" + category + "', Amount = " +
                            std::to_string(amount) + ", Description = '" + description +
                            "' WHERE ID = " + std::to_string(id) + ";";
    executeSQL(updateSQL);
}

// Function to search expenses by description
void searchExpenseByDescription(const std::string& description) {
    std::string searchSQL = "SELECT * FROM Expenses WHERE Description LIKE '%" + description + "%';";
    executeSQL(searchSQL);
}

// Function to filter expenses by date range
void viewExpensesByDateRange(const std::string& startDate, const std::string& endDate) {
    std::string filterSQL = "SELECT * FROM Expenses WHERE Date BETWEEN '" + startDate + "' AND '" + endDate + "';";
    executeSQL(filterSQL);
}


// Function to reset the ID counter of the Expenses table
void resetIDCounter() {
    std::string deleteAllSQL = "DELETE FROM Expenses;"; // Clear all rows
    executeSQL(deleteAllSQL);

    std::string resetCounterSQL = "DELETE FROM sqlite_sequence WHERE name='Expenses';"; // Reset the AUTOINCREMENT counter
    executeSQL(resetCounterSQL);

    std::string vacuumSQL = "VACUUM;"; // Reorganize the database to reset counters
    executeSQL(vacuumSQL);

    std::cout << "ID counter reset successfully.\n";
}

int main() {
    initializeDatabase();

    int choice;
    std::string category, description, startDate, endDate;
    double amount;
    int id;

    do {
        std::cout << "\nExpense Tracker Menu:\n1. Add Expense\n2. Delete Expense\n3. Display Expenses\n4. Update Expense\n5. Search by Description\n6. View Expenses by Date Range\n7. Reset ID Counter\n8. Exit\nEnter choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                std::cout << "Enter category: ";
                std::getline(std::cin, category);
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cin.ignore();
                std::cout << "Enter description: ";
                std::getline(std::cin, description);
                addExpenseToDatabase(category, amount, description);
                std::cout << "Expense added successfully!\n";
                break;
            case 2:
                std::cout << "Enter the ID of the expense to delete: ";
                std::cin >> id;
                deleteExpenseFromDatabase(id);
                std::cout << "Expense deleted successfully!\n";
                break;
            case 3:
                std::cout << "\nCurrent Expenses:\n";
                displayExpensesFromDatabase();
                break;
            case 4:
    std::cout << "Enter the ID of the expense to update: ";
    std::cin >> id;
    std::cin.ignore(); // Clears the newline character left by std::cin
    std::cout << "Enter new category: ";
    std::getline(std::cin, category); // Reads the new category as a string
    std::cout << "Enter new amount: ";
    std::cin >> amount;
    std::cin.ignore(); // Clears the newline character left by std::cin
    std::cout << "Enter new description: ";
    std::getline(std::cin, description); // Reads the new description
    updateExpenseInDatabase(id, category, amount, description);
    std::cout << "Expense updated successfully!\n";
    break;

            case 5:
                std::cout << "Enter description to search: ";
                std::getline(std::cin, description);
                searchExpenseByDescription(description);
                break;
            case 6:
                std::cout << "Enter start date (YYYY-MM-DD): ";
                std::getline(std::cin, startDate);
                std::cout << "Enter end date (YYYY-MM-DD): ";
                std::getline(std::cin, endDate);
                viewExpensesByDateRange(startDate, endDate);
                break;
            case 7:
                resetIDCounter();
                break;
            case 8:
                std::cout << "Exiting the Expense Tracker.\n";
                break;
            default:
                std::cout << "Invalid choice, please try again.\n";
        }
    } while (choice != 8);

    sqlite3_close(db);
    return 0;
}



// gcc -c sqlite3.c -o sqlite3.o

// g++ -g expense_tracker_sql.cpp sqlite3.o -o expense_tracker -fpermissive -I.