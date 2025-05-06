#include<iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <fstream>
#include "task.h"


std::vector<Event> readEvents(const std::string& filename, int& tables, Time& openTime, Time& closeTime, int& hourCost) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file");
    }
    std::vector<Event> events;
    static std::string line;
    int lineNumber = 0;
    if (!std::getline(file, line)) {
        throw std::runtime_error("Missing number of tables");
    }
    lineNumber++;
    try {
        tables = std::stoi(line);
        if (tables <= 0) {
            throw std::invalid_argument("Number of tables must be positive");
        }
    }
    catch (...) {
        throw std::runtime_error("Line #" + std::to_string(lineNumber) + ' ' + line + ": Invalid number of tables");
    }
    if (!std::getline(file, line)) {
        throw std::runtime_error("Missing opening and closing time");
    }
    lineNumber++;

    try {
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
            throw std::invalid_argument("Invalid time format");
        }
        std::string openStr = line.substr(0, spacePos);
        std::string closeStr = line.substr(spacePos + 1);
        openTime = Time(openStr);
        closeTime = Time(closeStr);
        if (openTime >= closeTime) {
            throw std::invalid_argument("Opening time must be before closing time");
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Line #" + std::to_string(lineNumber) + ' ' + line + ": " + e.what());
    }
    if (!std::getline(file, line)) {
        throw std::runtime_error("Missing hour cost");
    }
    lineNumber++;
    try {
        hourCost = std::stoi(line);
        if (hourCost <= 0) {
            throw std::invalid_argument("Hour cost must be positive");
        }
    }
    catch (...) {
        throw std::runtime_error("Line #" + std::to_string(lineNumber) + ' ' + line + ": Invalid hour cost");
    }
    while (std::getline(file, line)) {
        lineNumber++;
        if (line.empty()) { 
            continue;
        }
        try {
            events.push_back(Event::parse(line));
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Line #" + std::to_string(lineNumber) + ' ' + line + ": " + e.what());
        }
    }
    return events;
}

int main(int arg_count, char* arg_values[]) {
    if (arg_count != 2) {
        std::cerr << "Usage: " << arg_values[0] << " <test_file>" << std::endl;
        return 1;
    }
    try {
        int tables;
        Time openTime, closeTime;
        int hourCost;
        auto events = readEvents(arg_values[1], tables, openTime, closeTime, hourCost);
        ComputerClub club(tables, openTime, closeTime, hourCost);
        for (const auto& event : events) {
            club.processEvent(event);
        }
        club.endOfDay();
        club.printResults();
    }
    catch (const std::exception& e) {
        std::cerr<< e.what() << std::endl;
        return 1;
    }
    return 0;
}