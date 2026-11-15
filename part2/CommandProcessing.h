// Assignment 2, Part 1: command processor and file command processor adapter.
// Part 5: Command and CommandProcessor are subjects of the game log observer.

#ifndef COMMANDPROCESSING_H
#define COMMANDPROCESSING_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "GameEngine.h"
#include "LoggingObserver.h"

// A command typed by the user (for example "addplayer Alice") and its effect.
// Saving the effect notifies the observers so the effect is logged.
class Command : public Subject, public ILoggable {
public:
    Command(std::string text);
    Command(const Command& other);
    ~Command();
    Command& operator=(const Command& other);
    friend std::ostream& operator<<(std::ostream& out, const Command& command);

    void saveEffect(std::string newEffect);
    std::string getCommand() const;
    std::string getEffect() const;

    // Log line: the command and its effect
    std::string stringToLog();

private:
    std::string* command;
    std::string* effect;
};

// Reads commands from the console and saves them in a list of commands.
// The command processor owns its commands and deletes them.
// Saving a command notifies the observers so the command is logged, and every
// new command is given the processor's observers so its effect is logged too.
class CommandProcessor : public Subject, public ILoggable {
public:
    CommandProcessor();
    CommandProcessor(const CommandProcessor& other);
    virtual ~CommandProcessor();
    CommandProcessor& operator=(const CommandProcessor& other);
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor);

    Command* getCommand();
    bool validate(Command* command, GameEngine* engine);
    GameEngine::State nextState(Command* command);

    // Log line: the last command that was saved
    std::string stringToLog();

protected:
    virtual std::string readCommand();
    void saveCommand(std::string text);

private:
    std::vector<Command*>* commands;
};

// Reads a text file line by line (the adaptee of the Adapter pattern).
class FileLineReader {
public:
    FileLineReader(std::string fileName);
    FileLineReader(const FileLineReader& other);
    ~FileLineReader();
    FileLineReader& operator=(const FileLineReader& other);
    friend std::ostream& operator<<(std::ostream& out, const FileLineReader& reader);

    bool readLineFromFile(std::string& line);
    bool isOpen() const;

private:
    std::string* fileName;
    std::ifstream* file;
};

// Adapter: a command processor that reads its commands from a file instead of
// the console, by using a FileLineReader.
class FileCommandProcessorAdapter : public CommandProcessor {
public:
    FileCommandProcessorAdapter(std::string fileName);
    FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other);
    ~FileCommandProcessorAdapter();
    FileCommandProcessorAdapter& operator=(const FileCommandProcessorAdapter& other);
    friend std::ostream& operator<<(std::ostream& out, const FileCommandProcessorAdapter& adapter);

    bool isFileOpen() const;

protected:
    std::string readCommand();

private:
    FileLineReader* flr;
};

#endif