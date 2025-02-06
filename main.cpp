#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <libpq-fe.h>
#include <algorithm>

// Function to check PostgreSQL connection
void checkConnection(PGconn* conn) {
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        exit(1);
    }
}

// Function to read an SQL file into a string
std::string readSQLFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open SQL file: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();  // Read the entire file into a string
    return buffer.str();
}

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& s) {
    std::string result = s;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), result.end());
    return result;
}

// Function to execute multiple SQL statements from a file.
// Splits the file contents on ';' and executes each non-empty statement.
void executeSQLFile(PGconn* conn, const std::string& filename) {
    std::string sqlScript = readSQLFile(filename);
    if (sqlScript.empty()) {
        std::cerr << "No SQL script loaded from " << filename << std::endl;
        return;
    }

    std::stringstream ss(sqlScript);
    std::string statement;
    int statementCount = 0;

    while (std::getline(ss, statement, ';')) {
        statement = trim(statement);
        if (statement.empty()) {
            continue;
        }
        // Append the semicolon back (if needed by PostgreSQL)
        statement += ";";
        PGresult* res = PQexec(conn, statement.c_str());
        ExecStatusType status = PQresultStatus(res);
        if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
            std::cerr << "SQL execution failed for statement:\n" << statement
                << "\nError: " << PQerrorMessage(conn) << std::endl;
        }
        PQclear(res);
        statementCount++;
    }
    std::cout << "Executed " << statementCount << " statements from " << filename << std::endl;
}

// Function to read an SQL file and execute it as before (for non-metadata commands)
// This function is the same as your original executeSQL.
void executeSQL(PGconn* conn, const std::string& sql, const std::string& sqlFilename) {
    if (sql.empty()) {
        std::cerr << "Error: SQL script is empty or file could not be read.\n";
        return;
    }
    PGresult* res = PQexec(conn, sql.c_str());
    ExecStatusType status = PQresultStatus(res);
    if (status == PGRES_TUPLES_OK) {  // Handle SELECT queries
        // For this example, we save SELECT query results to CSV (same as before)
        size_t dotPos = sqlFilename.find_last_of(".");
        std::string csvFilename = (dotPos == std::string::npos) ? sqlFilename + "_result.csv"
            : sqlFilename.substr(0, dotPos) + "_result.csv";
        std::ofstream csvFile(csvFilename);
        if (!csvFile) {
            std::cerr << "Error: Could not create CSV file: " << csvFilename << std::endl;
        }
        else {
            int rows = PQntuples(res);
            int cols = PQnfields(res);
            // Write header row
            for (int i = 0; i < cols; ++i) {
                csvFile << PQfname(res, i);
                if (i < cols - 1)
                    csvFile << ",";
            }
            csvFile << "\n";
            // Write data rows
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    csvFile << PQgetvalue(res, i, j);
                    if (j < cols - 1)
                        csvFile << ",";
                }
                csvFile << "\n";
            }
            csvFile.close();
            std::cout << "Query results saved to: " << csvFilename << std::endl;
        }
    }
    else if (status == PGRES_COMMAND_OK) {  // Handle non-SELECT queries
        std::cout << "SQL script executed successfully!\n";
    }
    else {
        std::cerr << "SQL execution failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

int main() {
    // Connect to PostgreSQL
    PGconn* conn = PQconnectdb("host=localhost dbname=dota2 user=postgres password=postgres");
    checkConnection(conn);

    while (true) {
        std::string input;
        std::cout << "Enter the SQL file name (or type 'exit' to quit, 'load' to load metadata, 'join' to execute join.sql): ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }
        else if (input == "load") {
            // Instead of embedded SQL, load metadata using external SQL files.
            executeSQLFile(conn, "table_metadata.sql");
            executeSQLFile(conn, "relational_metadata.sql");
        }
        else if (input == "join") {
            // Execute the join query stored in join.sql.
            std::string joinSQL = readSQLFile("join.sql");
            executeSQL(conn, joinSQL, "join.sql");
        }
        else {
            // Otherwise, treat the input as an SQL file name.
            std::string sqlScript = readSQLFile(input);
            executeSQL(conn, sqlScript, input);
        }
    }

    PQfinish(conn);
    std::cout << "Disconnected from PostgreSQL.\n";
    return 0;
}
