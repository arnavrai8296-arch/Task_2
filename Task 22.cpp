#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <exception>
#include <iomanip>
using namespace std;

// ---------- Custom exception for invalid marks ----------
class InvalidMarksException : public exception {
    string message;
public:
    InvalidMarksException(const string &subject, int value)
        : message("Invalid marks " + to_string(value) + " in " + subject +
                  " (must be between 0 and 100)") {}
    const char *what() const noexcept override { return message.c_str(); }
};

// ---------- Base class ----------
class Student {
protected:
    string name;
    int rollNo;
public:
    Student(const string &n, int r) : name(n), rollNo(r) {}
    virtual ~Student() = default;                 // virtual destructor

    // Virtual functions (overridden in Result)
    virtual void displayDetails() const {
        cout << "Roll No : " << rollNo << endl;
        cout << "Name    : " << name << endl;
    }
    virtual void displayMarks() const {
        cout << "Marks   : not available" << endl;
    }
    virtual int getTotal() const { return 0; }
    virtual string toRecord() const {
        return to_string(rollNo) + "," + name;
    }

    int getRollNo() const { return rollNo; }
    const string &getName() const { return name; }
};

// ---------- Derived class ----------
class Result : public Student {
public:
    static constexpr int SUBJECTS = 3;
    static constexpr int MAX_MARKS = 100;
private:
    static const char *subjectName(int i) {
        static const char *names[SUBJECTS] = {"Maths", "Electronics", "C++"};
        return names[i];
    }
    int marks[SUBJECTS];
    int total;

public:
    // Validates marks; throws InvalidMarksException if out of range
    Result(const string &n, int r, const int m[SUBJECTS]) : Student(n, r), total(0) {
        for (int i = 0; i < SUBJECTS; i++) {
            if (m[i] < 0 || m[i] > MAX_MARKS)
                throw InvalidMarksException(subjectName(i), m[i]);
            marks[i] = m[i];
            total += m[i];
        }
    }

    int getTotal() const override { return total; }

    double getPercentage() const {
        return total * 100.0 / (SUBJECTS * MAX_MARKS);
    }

    bool isPass() const {                          // pass: >= 40 in every subject
        for (int i = 0; i < SUBJECTS; i++)
            if (marks[i] < 40) return false;
        return true;
    }

    char getGrade() const {
        double p = getPercentage();
        if (!isPass()) return 'F';
        if (p >= 90) return 'A';
        if (p >= 75) return 'B';
        if (p >= 60) return 'C';
        return 'D';
    }

    void displayDetails() const override {
        Student::displayDetails();                 // reuse base version
    }

    void displayMarks() const override {
        cout << "Marks   : ";
        for (int i = 0; i < SUBJECTS; i++)
            cout << subjectName(i) << "=" << marks[i] << "  ";
        cout << endl;
        cout << "Total   : " << total << " / " << SUBJECTS * MAX_MARKS
             << "   Percentage: " << fixed << setprecision(2) << getPercentage() << "%"
             << "   Grade: " << getGrade()
             << "   Result: " << (isPass() ? "PASS" : "FAIL") << endl;
    }

    string toRecord() const override {
        string s = to_string(rollNo) + "," + name;
        for (int i = 0; i < SUBJECTS; i++) s += "," + to_string(marks[i]);
        return s;
    }
};

typedef vector<unique_ptr<Student>> StudentList;

// ---------- Helper functions ----------
void showRecord(const Student &s) {               // polymorphic call via base reference
    s.displayDetails();
    s.displayMarks();
    cout << "---------------------------------------------------------------" << endl;
}

void showAll(const StudentList &list) {
    for (const auto &s : list) showRecord(*s);
}

void addStudent(StudentList &list, const string &name, int roll, int m1, int m2, int m3) {
    int m[Result::SUBJECTS] = {m1, m2, m3};
    try {
        list.push_back(make_unique<Result>(name, roll, m));
        cout << "Added   : " << name << " (Roll " << roll << ")" << endl;
    } catch (const InvalidMarksException &e) {
        cout << "Rejected: " << name << " (Roll " << roll << ") -> " << e.what() << endl;
    }
}

void saveToFile(const StudentList &list, const string &filename) {
    ofstream out(filename);
    if (!out) throw runtime_error("Cannot open file '" + filename + "' for writing");
    for (const auto &s : list) out << s->toRecord() << "\n";
}

StudentList loadFromFile(const string &filename) {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open file '" + filename + "' for reading");
    StudentList list;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string roll, name, f[Result::SUBJECTS];
        getline(ss, roll, ',');
        getline(ss, name, ',');
        for (int i = 0; i < Result::SUBJECTS; i++) getline(ss, f[i], ',');
        try {
            int m[Result::SUBJECTS];
            for (int i = 0; i < Result::SUBJECTS; i++) m[i] = stoi(f[i]);
            list.push_back(make_unique<Result>(name, stoi(roll), m));
        } catch (const exception &e) {         // bad number or invalid marks
            cout << "Skipped corrupt record: \"" << line << "\" (" << e.what() << ")" << endl;
        }
    }
    return list;
}

int main(int argc, char *argv[]) {
    int threshold = (argc > 1) ? atoi(argv[1]) : 240;   // specified mark (total out of 300)
    const string file = "students.txt";
    StudentList students;

    cout << "=== 1. Entering student records (with mark validation) ===" << endl;
    addStudent(students, "Arjun Rao",   101, 85, 78, 92);
    addStudent(students, "Meera Nair",  102, 95, 91, 97);
    addStudent(students, "Rahul Shetty", 103, 55, 38, 62);
    addStudent(students, "Sneha Kumar", 104, 72, 68, 75);
    addStudent(students, "Kiran Das",   105, 105, 80, 70);   // invalid
    addStudent(students, "Divya Iyer",  106, 88, -5, 90);    // invalid
    addStudent(students, "Varun Patel", 107, 80, 84, 79);
    cout << "Valid records stored in vector: " << students.size() << endl;

    cout << endl << "=== 2. Student details, total marks and results ===" << endl;
    showAll(students);

    cout << endl << "=== 3. File handling: save and retrieve ===" << endl;
    saveToFile(students, file);
    cout << "Saved " << students.size() << " records to " << file << endl;
    students.clear();
    cout << "Vector cleared. Size = " << students.size() << endl;
    students = loadFromFile(file);
    cout << "Loaded " << students.size() << " records from " << file << endl << endl;
    showAll(students);
    try {
        loadFromFile("missing_file.txt");
    } catch (const runtime_error &e) {
        cout << "Exception caught: " << e.what() << endl;
    }

    cout << endl << "=== 4. Students scoring above " << threshold << " (lambda + STL) ===" << endl;
    // unique_ptr cannot be copied, so collect raw pointers and filter them with a lambda
    vector<const Student *> selected;
    for (const auto &s : students) selected.push_back(s.get());
    selected.erase(remove_if(selected.begin(), selected.end(),
                   [threshold](const Student *s) { return s->getTotal() <= threshold; }),
                   selected.end());
    if (selected.empty()) cout << "No student scored above " << threshold << endl;
    for_each(selected.begin(), selected.end(), [](const Student *s) {
        cout << setw(4) << s->getRollNo() << "  " << left << setw(14) << s->getName()
             << right << " Total = " << s->getTotal() << endl;
    });

    cout << endl << "=== 5. Summary (STL algorithms) ===" << endl;
    auto topper = max_element(students.begin(), students.end(),
        [](const unique_ptr<Student> &a, const unique_ptr<Student> &b) {
            return a->getTotal() < b->getTotal();
        });
    int passed = count_if(students.begin(), students.end(), [](const unique_ptr<Student> &s) {
        const Result *r = dynamic_cast<const Result *>(s.get());
        return r && r->isPass();
    });
    cout << "Total students : " << students.size() << endl;
    cout << "Passed         : " << passed << endl;
    cout << "Failed         : " << students.size() - passed << endl;
    if (topper != students.end())
        cout << "Topper         : " << (*topper)->getName() << " (" << (*topper)->getTotal() << ")" << endl;

    return 0;
}
