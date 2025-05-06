#pragma once
#include<iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>

class Time {
public:
    Time() = default;
    Time(int hours, int minutes);
    explicit Time(const std::string& timeStr);
    bool isValid() const;
    std::string toString() const;
    int toMinutes() const;
    bool operator<(const Time& other) const;
    bool operator<=(const Time& other) const;
    bool operator>(const Time& other) const;
    bool operator>=(const Time& other) const;
    bool operator==(const Time& other) const;
    Time operator+(const Time& other) const;
    Time operator-(const Time& other) const;
    static Time fromMinutes(int minutes);
private:
    int hours = 0;
    int minutes = 0;
};

class Event {
public:
    Event(Time time, int id, std::string body);
    static Event parse(const std::string& line);
    Time getTime() const;
    int getId() const;
    std::string getBody() const;
    std::string toString() const;
private:
    Time time;
    int id;
    std::string body;
};




class ComputerClub {
public:
    ComputerClub(int tables, Time openTime, Time closeTime, int hourCost);
    void processEvent(const Event& event);
    void endOfDay();
    void printResults() const;
private:
    struct Table {
        bool isOccupied = false;
        std::string currentClient;
        Time startTime;
        Time totalTime;
        int revenue = 0;
    };
    int tablesCount;
    Time openTime;
    Time closeTime;
    int hourCost;
    std::vector<Table> m_tables;
    std::map<std::string, int> m_clients;
    std::queue<std::string> m_waitingQueue;
    std::vector<Event> m_outputEvents;
    void clientArrives(const Time& time, const std::string& clientName);
    void clientSits(const Time& time, const std::string& clientName, int tableNumber);
    void clientWaits(const Time& time, const std::string& clientName);
    void clientLeaves(const Time& time, const std::string& clientName);
    void addError(const Time& time, const std::string& error);
    void addOutgoingEvent(const Event& event);
    int findFreeTable() const;
    bool isClientInClub(const std::string& clientName) const;
    void freeTable(int tableNumber, const Time& currentTime);
};


#include "task.hpp"