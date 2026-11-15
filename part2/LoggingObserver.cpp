#include "LoggingObserver.h"

#include <algorithm>
#include <fstream>

using namespace std;

// ILoggable

// Default constructor: the interface has no data
ILoggable::ILoggable() {
}

// Copy constructor: nothing to copy
ILoggable::ILoggable(const ILoggable& other) {
}

// Virtual destructor so deleting through an ILoggable* runs the subclass destructor
ILoggable::~ILoggable() {
}

// Assignment operator: nothing to copy
ILoggable& ILoggable::operator=(const ILoggable& other) {
    return *this;
}

// Prints a short description of the interface
ostream& operator<<(ostream& out, const ILoggable& loggable) {
    out << "ILoggable object";
    return out;
}

// Observer

// Default constructor: the interface has no data
Observer::Observer() {
}

// Copy constructor: nothing to copy
Observer::Observer(const Observer& other) {
}

// Virtual destructor so deleting through an Observer* runs the subclass destructor
Observer::~Observer() {
}

// Assignment operator: nothing to copy
Observer& Observer::operator=(const Observer& other) {
    return *this;
}

// Prints a short description of the interface
ostream& operator<<(ostream& out, const Observer& observer) {
    out << "Observer";
    return out;
}

// Subject

// Default constructor: no observers yet
Subject::Subject() {
    observers = new vector<Observer*>();
}

// Copy constructor: the copy is watched by the same observers. Only the
// pointers are copied, because the subject does not own its observers.
Subject::Subject(const Subject& other) {
    observers = new vector<Observer*>(*other.observers);
}

// Destructor: deletes the list only, the observers belong to someone else
Subject::~Subject() {
    delete observers;
    observers = nullptr;
}

// Assignment operator: copies the other subject's observer pointers
Subject& Subject::operator=(const Subject& other) {
    if (this != &other) {
        *observers = *other.observers;
    }
    return *this;
}

// Prints how many observers are attached
ostream& operator<<(ostream& out, const Subject& subject) {
    out << "Subject with " << subject.observerCount() << " observer(s)";
    return out;
}

// Adds an observer, unless it is null or already attached
void Subject::attach(Observer* observer) {
    if (observer == nullptr) {
        return;
    }
    if (find(observers->begin(), observers->end(), observer) == observers->end()) {
        observers->push_back(observer);
    }
}

// Removes an observer if it is attached
void Subject::detach(Observer* observer) {
    observers->erase(remove(observers->begin(), observers->end(), observer), observers->end());
}

// Tells every attached observer that the given object changed
void Subject::notify(ILoggable* loggable) {
    for (Observer* observer : *observers) {
        observer->update(loggable);
    }
}

// Attaches all of this subject's observers to another subject
void Subject::attachObserversTo(Subject* other) const {
    if (other == nullptr) {
        return;
    }
    for (Observer* observer : *observers) {
        other->attach(observer);
    }
}

// Number of attached observers
int Subject::observerCount() const {
    return static_cast<int>(observers->size());
}

// LogObserver

// Default constructor: writes to gamelog.txt
LogObserver::LogObserver() {
    fileName = new string("gamelog.txt");
}

// Constructor with the name of the log file
LogObserver::LogObserver(string name) {
    fileName = new string(name);
}

// Copy constructor: the copy writes to the same file
LogObserver::LogObserver(const LogObserver& other) : Observer(other) {
    fileName = new string(*other.fileName);
}

// Destructor: frees the file name
LogObserver::~LogObserver() {
    delete fileName;
    fileName = nullptr;
}

// Assignment operator: copies the file name
LogObserver& LogObserver::operator=(const LogObserver& other) {
    if (this != &other) {
        Observer::operator=(other);
        *fileName = *other.fileName;
    }
    return *this;
}

// Prints the name of the log file
ostream& operator<<(ostream& out, const LogObserver& observer) {
    out << "LogObserver writing to " << *observer.fileName;
    return out;
}

// Appends the object's stringToLog() as a new line of the log file
void LogObserver::update(ILoggable* loggable) {
    if (loggable == nullptr) {
        return;
    }
    ofstream file(*fileName, ios::app);
    if (!file.is_open()) {
        cout << "LogObserver: could not open " << *fileName << endl;
        return;
    }
    file << loggable->stringToLog() << endl;
}

// Empties the log file
void LogObserver::clearLog() {
    ofstream file(*fileName, ios::trunc);
}

// Getter of the log file name
string LogObserver::getFileName() const {
    return *fileName;
}
