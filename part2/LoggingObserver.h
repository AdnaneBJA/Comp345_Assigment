// Assignment 2, Part 5: game log observer (Observer design pattern).
//
// Command, CommandProcessor, Order, OrdersList and GameEngine inherit from both
// Subject and ILoggable. When something worth logging happens they call
// notify(this), and every attached Observer receives the object. The concrete
// LogObserver asks the object for its stringToLog() and writes it to gamelog.txt.

#ifndef LOGGINGOBSERVER_H
#define LOGGINGOBSERVER_H

#include <iostream>
#include <string>
#include <vector>

// Interface of every object that can be written to the game log.
class ILoggable {
public:
    ILoggable();
    ILoggable(const ILoggable& other);
    virtual ~ILoggable();
    ILoggable& operator=(const ILoggable& other);
    friend std::ostream& operator<<(std::ostream& out, const ILoggable& loggable);

    // Returns the line that the log observer writes to the log file
    virtual std::string stringToLog() = 0;
};

// Interface of an observer. It is told about every change of the subjects it
// is attached to.
class Observer {
public:
    Observer();
    Observer(const Observer& other);
    virtual ~Observer();
    Observer& operator=(const Observer& other);
    friend std::ostream& operator<<(std::ostream& out, const Observer& observer);

    // Called by a subject when it notifies its observers
    virtual void update(ILoggable* loggable) = 0;
};

// A subject keeps a list of observers and notifies all of them when it changes.
// The observers are not owned by the subject, so they are never deleted here.
class Subject {
public:
    Subject();
    Subject(const Subject& other);
    virtual ~Subject();
    Subject& operator=(const Subject& other);
    friend std::ostream& operator<<(std::ostream& out, const Subject& subject);

    void attach(Observer* observer);
    void detach(Observer* observer);
    void notify(ILoggable* loggable);

    // Gives this subject's observers to another subject (for example, a
    // command processor passes its observers on to every command it creates)
    void attachObserversTo(Subject* other) const;
    int observerCount() const;

private:
    std::vector<Observer*>* observers;  // not owned
};

// Concrete observer: writes the string of every notified object to a log file
// (gamelog.txt by default). The file is opened in append mode for every entry,
// so many objects can share it and the log keeps everything written so far.
class LogObserver : public Observer {
public:
    LogObserver();
    LogObserver(std::string fileName);
    LogObserver(const LogObserver& other);
    ~LogObserver();
    LogObserver& operator=(const LogObserver& other);
    friend std::ostream& operator<<(std::ostream& out, const LogObserver& observer);

    void update(ILoggable* loggable);

    // Empties the log file, used at the start of a new demonstration
    void clearLog();
    std::string getFileName() const;

private:
    std::string* fileName;
};

#endif
