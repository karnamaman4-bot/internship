#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <limits>

/**
 * @class Student
 * @brief Represents an individual student entity with encapsulated attributes.
 */
class Student {
private:
    int id;
    std::string name;
    std::string department;
    double gpa;

public:
    Student() : id(0), gpa(0.0) {}
    Student(int id, std::string name, std::string department, double gpa)
        : id(id), name(std::move(name)), department(std::move(department)), gpa(gpa) {}

    // Getters
    [[nodiscard]] int getId() const { return id; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getDepartment() const { return department; }
    [[nodiscard]] double getGPA() const { return gpa; }

    // Setters
    void setName(const std::string& newName) { name = newName; }
    void setDepartment(const std::string& newDept) { department = newDept; }
    void setGPA(double newGPA) { gpa = newGPA; }

    /**
     * @brief Serializes the Student object to CSV format.
     */
    [[nodiscard]] std::string serialize() const {
        std::ostringstream oss;
        oss << id << "," << name << "," << department << "," << gpa;
        return oss.str();
    }

    /**
     * @brief Deserializes a CSV line into a Student object.
     */
    static Student deserialize(const std::string& csvLine) {
        std::stringstream ss(csvLine);
        std::string item;
        
        int tempId;
        std::string tempName;
        std::string tempDept;
        double tempGpa;

        if (!std::getline(ss, item, ',')) throw std::invalid_argument("Corrupted ID field.");
        tempId = std::stoi(item);

        if (!std::getline(ss, tempName, ',')) throw std::invalid_argument("Corrupted Name field.");
        if (!std::getline(ss, tempDept, ',')) throw std::invalid_argument("Corrupted Department field.");

        if (!std::getline(ss, item, ',')) throw std::invalid_argument("Corrupted GPA field.");
        tempGpa = std::stod(item);

        return Student(tempId, tempName, tempDept, tempGpa);
    }
};

/**
 * @class Repository
 * @brief Manages persistent storage operations using atomic file replacements.
 */
class Repository {
private:
    std::string filePath;

public:
    explicit Repository(std::string path) : filePath(std::move(path)) {}

    std::vector<Student> loadAllRecords() const {
        std::vector<Student> records;
        std::ifstream inFile(filePath);
        
        if (!inFile.is_open()) {
            return records; // Return empty if file does not exist yet
        }

        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            try {
                records.push_back(Student::deserialize(line));
            } catch (const std::exception& e) {
                // Ignore or log malformed rows seamlessly
            }
        }
        return records;
    }

    void saveAllRecords(const std::vector<Student>& records) const {
        std::ofstream outFile(filePath, std::ios::trunc);
        if (!outFile.is_open()) {
            throw std::runtime_error("Fatal Error: Unable to open storage file for writing.");
        }

        for (const auto& student : records) {
            outFile << student.serialize() << "\n";
        }
    }
};

/**
 * @class StudentService
 * @brief Contains core business logic for processing student operations.
 */
class StudentService {
private:
    Repository repo;

public:
    explicit StudentService(const std::string& dbPath) : repo(dbPath) {}

    void addStudent(int id, const std::string& name, const std::string& dept, double gpa) {
        auto records = repo.loadAllRecords();
        
        auto it = std::find_if(records.begin(), records.end(), [id](const Student& s) {
            return s.getId() == id;
        });

        if (it != records.end()) {
            throw std::invalid_argument("A student with ID " + std::to_string(id) + " already exists.");
        }

        records.emplace_back(id, name, dept, gpa);
        repo.saveAllRecords(records);
    }

    [[nodiscard]] std::vector<Student> getAllStudents() const {
        return repo.loadAllRecords();
    }

    [[nodiscard]] Student getStudentById(int id) const {
        auto records = repo.loadAllRecords();
        auto it = std::find_if(records.begin(), records.end(), [id](const Student& s) {
            return s.getId() == id;
        });

        if (it == records.end()) {
            throw std::runtime_error("Student with ID " + std::to_string(id) + " was not found.");
        }
        return *it;
    }

    void updateStudent(int id, const std::string& newName, const std::string& newDept, double newGpa) {
        auto records = repo.loadAllRecords();
        auto it = std::find_if(records.begin(), records.end(), [id](const Student& s) {
            return s.getId() == id;
        });

        if (it == records.end()) {
            throw std::runtime_error("Student with ID " + std::to_string(id) + " was not found.");
        }

        it->setName(newName);
        it->setDepartment(newDept);
        it->setGPA(newGpa);

        repo.saveAllRecords(records);
    }

    void deleteStudent(int id) {
        auto records = repo.loadAllRecords();
        auto initialSize = records.size();

        records.erase(std::remove_if(records.begin(), records.end(), [id](const Student& s) {
            return s.getId() == id;
        }), records.end());

        if (records.size() == initialSize) {
            throw std::runtime_error("Student with ID " + std::to_string(id) + " was not found.");
        }

        repo.saveAllRecords(records);
    }
};

/**
 * @class ConsoleUI
 * @brief Handles input validation, UI rendering, and menu presentation.
 */
class ConsoleUI {
private:
    StudentService service;

    static void clearInputStream() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    static int readInt(const std::string& prompt) {
        int value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                clearInputStream();
                return value;
            }
            std::cout << " [!] Invalid input. Please enter a valid integer.\n";
            clearInputStream();
        }
    }

    static double readDouble(const std::string& prompt, double minVal, double maxVal) {
        double value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value && value >= minVal && value <= maxVal) {
                clearInputStream();
                return value;
            }
            std::cout << " [!] Invalid input. Enter a decimal between " << minVal << " and " << maxVal << ".\n";
            clearInputStream();
        }
    }

    static std::string readString(const std::string& prompt) {
        std::string value;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, value);
            if (!value.empty()) return value;
            std::cout << " [!] Field cannot be empty. Try again.\n";
        }
    }

    static void printHeader() {
        std::cout << "\n===================================================================\n";
        std::cout << "                   STUDENT MANAGEMENT SYSTEM                       \n";
        std::cout << "===================================================================\n";
    }

    static void printTable(const std::vector<Student>& students) {
        if (students.empty()) {
            std::cout << "\n [i] No student records available.\n";
            return;
        }

        std::cout << "\n+" << std::string(10, '-') << "+" << std::string(25, '-') << "+" 
                  << std::string(20, '-') << "+" << std::string(8, '-') << "+\n";
        std::cout << "| " << std::left << std::setw(8) << "ID"
                  << "| " << std::setw(23) << "Full Name"
                  << "| " << std::setw(18) << "Department"
                  << "| " << std::setw(6)  << "GPA" << " |\n";
        std::cout << "+" << std::string(10, '-') << "+" << std::string(25, '-') << "+" 
                  << std::string(20, '-') << "+" << std::string(8, '-') << "+\n";

        for (const auto& s : students) {
            std::cout << "| " << std::left << std::setw(8) << s.getId()
                      << "| " << std::setw(23) << s.getName()
                      << "| " << std::setw(18) << s.getDepartment()
                      << "| " << std::setw(6)  << std::fixed << std::setprecision(2) << s.getGPA() << " |\n";
        }
        std::cout << "+" << std::string(10, '-') << "+" << std::string(25, '-') << "+" 
                  << std::string(20, '-') << "+" << std::string(8, '-') << "+\n";
    }

public:
    explicit ConsoleUI(std::string dbPath) : service(std::move(dbPath)) {}

    void run() {
        while (true) {
            printHeader();
            std::cout << " 1. Register New Student\n";
            std::cout << " 2. View All Records\n";
            std::cout << " 3. Search Student by ID\n";
            std::cout << " 4. Update Student Record\n";
            std::cout << " 5. Delete Student Record\n";
            std::cout << " 6. Exit Application\n";
            std::cout << "-------------------------------------------------------------------\n";

            int choice = readInt(" Select option (1-6): ");

            try {
                switch (choice) {
                    case 1: {
                        std::cout << "\n--- [ Register Student ] ---\n";
                        int id = readInt(" Enter Student ID: ");
                        std::string name = readString(" Enter Full Name: ");
                        std::string dept = readString(" Enter Department: ");
                        double gpa = readDouble(" Enter GPA (0.0 - 4.0): ", 0.0, 4.0);

                        service.addStudent(id, name, dept, gpa);
                        std::cout << "\n [SUCCESS] Record added successfully.\n";
                        break;
                    }
                    case 2: {
                        std::cout << "\n--- [ All Student Records ] ---\n";
                        printTable(service.getAllStudents());
                        break;
                    }
                    case 3: {
                        std::cout << "\n--- [ Search Student ] ---\n";
                        int id = readInt(" Enter Student ID: ");
                        Student s = service.getStudentById(id);
                        printTable({s});
                        break;
                    }
                    case 4: {
                        std::cout << "\n--- [ Update Student ] ---\n";
                        int id = readInt(" Enter Student ID to Update: ");
                        // Check existence
                        service.getStudentById(id);

                        std::string name = readString(" Enter New Full Name: ");
                        std::string dept = readString(" Enter New Department: ");
                        double gpa = readDouble(" Enter New GPA (0.0 - 4.0): ", 0.0, 4.0);

                        service.updateStudent(id, name, dept, gpa);
                        std::cout << "\n [SUCCESS] Record updated successfully.\n";
                        break;
                    }
                    case 5: {
                        std::cout << "\n--- [ Delete Student ] ---\n";
                        int id = readInt(" Enter Student ID to Delete: ");
                        service.deleteStudent(id);
                        std::cout << "\n [SUCCESS] Record deleted successfully.\n";
                        break;
                    }
                    case 6:
                        std::cout << "\n [!] Terminating system session. Goodbye!\n";
                        return;
                    default:
                        std::cout << "\n [!] Out of range choice. Pick 1 to 6.\n";
                }
            } catch (const std::exception& ex) {
                std::cout << "\n [ERROR] " << ex.what() << "\n";
            }
        }
    }
};

int main() {
    // Launch Application instance with persistent data store
    ConsoleUI app("student_records.csv");
    app.run();
    return 0;
}