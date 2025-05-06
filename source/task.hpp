#pragma once
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <algorithm>

Time::Time(int hours, int minutes) : hours(hours), minutes(minutes) {
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        throw std::invalid_argument("Invalid time");
    }
}

Time::Time(const std::string& timeStr) {
    if (timeStr.size() != 5 || timeStr[2] != ':') {
        throw std::invalid_argument("Invalid time format");
    }

    try {
        hours = std::stoi(timeStr.substr(0, 2));
        minutes = std::stoi(timeStr.substr(3, 2));
    }
    catch (...) {
        throw std::invalid_argument("Invalid time format");
    }

    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        throw std::invalid_argument("Invalid time");
    }
}

bool Time::isValid() const {
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59;
}

std::string Time::toString() const {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes;
    return oss.str();
}

int Time::toMinutes() const {
    return hours * 60 + minutes;
}

bool Time::operator<(const Time& other) const {
    return toMinutes() < other.toMinutes();
}

bool Time::operator<=(const Time& other) const {
    return toMinutes() <= other.toMinutes();
}

bool Time::operator>(const Time& other) const {
    return toMinutes() > other.toMinutes();
}

bool Time::operator>=(const Time& other) const {
    return toMinutes() >= other.toMinutes();
}

bool Time::operator==(const Time& other) const {
    return toMinutes() == other.toMinutes();
}

Time Time::operator+(const Time& other) const {
    int totalMinutes = toMinutes() + other.toMinutes();
    return fromMinutes(totalMinutes);
}

Time Time::operator-(const Time& other) const {
    int totalMinutes = toMinutes() - other.toMinutes();
    return fromMinutes(totalMinutes);
}

Time Time::fromMinutes(int minutes) {
    if (minutes < 0) minutes += 24 * 60;
    return Time((minutes / 60) % 24, minutes % 60);
}

//--------------------------------------------------------------------------------------------------------


Event::Event(Time time, int id, std::string body)
    : time(time), id(id), body(std::move(body)) {
}

Time Event::getTime() const {
    return time;
}

int Event::getId() const {
    return id;
}

std::string Event::getBody() const {
    return body;
}

std::string Event::toString() const {
    return time.toString() + " " + std::to_string(id) + " " + body;
}

Event Event::parse(const std::string& line) {
    std::istringstream iss(line);
    std::string timeStr, idStr;

    if (!(iss >> timeStr >> idStr)) {
        throw std::invalid_argument("Invalid event format");
    }

    Time time(timeStr);
    int id;

    try {
        id = std::stoi(idStr);
    }
    catch (...) {
        throw std::invalid_argument("Invalid event ID");
    }

    if (id < 1 || id > 13) {
        throw std::invalid_argument("Invalid event ID");
    }

    std::string body;
    std::getline(iss, body);
    if (!body.empty() && body[0] == ' ') {
        body = body.substr(1);
    }

    return Event(time, id, body);
}



//--------------------------------------------------------------------------------------------------------


ComputerClub::ComputerClub(int tables, Time openTime, Time closeTime, int hourCost) : tablesCount(tables), openTime(openTime), closeTime(closeTime), hourCost(hourCost) {
    m_tables.resize(tables + 1); 
}

void ComputerClub::processEvent(const Event& event) {
    m_outputEvents.push_back(event);

    switch (event.getId()) {
    case 1: {
        std::istringstream iss(event.getBody());
        std::string clientName;
        iss >> clientName;
        clientArrives(event.getTime(), clientName);
        break;
    }
    case 2: {
        std::istringstream iss(event.getBody());
        std::string clientName;
        int tableNumber;
        iss >> clientName >> tableNumber;
        if (tableNumber < 1 || tableNumber > tablesCount) {
            addError(event.getTime(), "Invalid table number");
            return;
        }
        clientSits(event.getTime(), clientName, tableNumber);
        break;
    }
    case 3: {
        std::istringstream iss(event.getBody());
        std::string clientName;
        iss >> clientName;
        clientWaits(event.getTime(), clientName);
        break;
    }
    case 4: {
        std::istringstream iss(event.getBody());
        std::string clientName;
        iss >> clientName;
        clientLeaves(event.getTime(), clientName);
        break;
    }
    default:
        addError(event.getTime(), "Unknown event type");
    }
}

void ComputerClub::endOfDay() {
    std::vector<std::string> remainingClients;
    for (const auto& client : m_clients) {
        remainingClients.push_back(client.first);
    }
    std::sort(remainingClients.begin(), remainingClients.end());
    for (const auto& client : remainingClients) {
        addOutgoingEvent(Event(closeTime, 11, client));
        auto it = m_clients.find(client);
        if (it != m_clients.end() && it->second != 0) {
            freeTable(it->second, closeTime);
        }
    }
    m_clients.clear();
    m_waitingQueue = std::queue<std::string>();
}

void ComputerClub::printResults() const {
    std::cout << openTime.toString() << std::endl;
    for (const auto& event : m_outputEvents) {
        std::cout << event.toString() << std::endl;
    }
    std::cout << closeTime.toString() << std::endl;
    for (int i = 1; i <= tablesCount; ++i) {
        const auto& table = m_tables[i];
        std::cout << i << " " << table.revenue << " " << table.totalTime.toString() << std::endl;
    }
}

void ComputerClub::clientArrives(const Time& time, const std::string& clientName) {
    if (time < openTime || time >= closeTime) {
        addError(time, "NotOpenYet");
        return;
    }
    if (isClientInClub(clientName)) {
        addError(time, "YouShallNotPass");
        return;
    }
    m_clients[clientName] = 0; 
}

void ComputerClub::clientSits(const Time& time, const std::string& clientName, int tableNumber) {
    if (!isClientInClub(clientName)) {
        addError(time, "ClientUnknown");
        return;
    }
    if (m_tables[tableNumber].isOccupied) {
        addError(time, "PlaceIsBusy");
        return;
    }
    int oldTable = m_clients[clientName];
    if (oldTable != 0) {
        freeTable(oldTable, time);
    }
    m_tables[tableNumber].isOccupied = true;
    m_tables[tableNumber].currentClient = clientName;
    m_tables[tableNumber].startTime = time;
    m_clients[clientName] = tableNumber;
}

void ComputerClub::clientWaits(const Time& time, const std::string& clientName) {
    if (!isClientInClub(clientName)) {
        addError(time, "ClientUnknown");
        return;
    }
    if (m_clients[clientName] != 0) {
        addError(time, "Client is already sitting");
        return;
    }
    if (findFreeTable() != -1) {
        addError(time, "ICanWaitNoLonger!");
        return;
    }
    if (m_waitingQueue.size() >= tablesCount) {
        addOutgoingEvent(Event(time, 11, clientName));
        m_clients.erase(clientName);
        return;
    }
    m_waitingQueue.push(clientName);
}

void ComputerClub::clientLeaves(const Time& time, const std::string& clientName) {
    if (!isClientInClub(clientName)) {
        addError(time, "ClientUnknown");
        return;
    }
    int tableNumber = m_clients[clientName];
    if (tableNumber != 0) {
        freeTable(tableNumber, time);
    }
    m_clients.erase(clientName);
    if (!m_waitingQueue.empty()) {
        int freeTable = findFreeTable();
        if (freeTable != -1) {
            std::string nextClient = m_waitingQueue.front();
            m_waitingQueue.pop();
            clientSits(time, nextClient, freeTable);
            addOutgoingEvent(Event(time, 12, nextClient + " " + std::to_string(freeTable)));
        }
    }
}

void ComputerClub::addError(const Time& time, const std::string& error) {
    m_outputEvents.push_back(Event(time, 13, error));
}

void ComputerClub::addOutgoingEvent(const Event& event) {
    m_outputEvents.push_back(event);
}

int ComputerClub::findFreeTable() const {
    for (int i = 1; i <= tablesCount; ++i) {
        if (!m_tables[i].isOccupied) {
            return i;
        }
    }
    return -1;
}

bool ComputerClub::isClientInClub(const std::string& clientName) const {
    return m_clients.find(clientName) != m_clients.end();
}

void ComputerClub::freeTable(int tableNumber, const Time& currentTime) {
    Table& table = m_tables[tableNumber];
    if (!table.isOccupied) return;
    Time timeSpent = currentTime - table.startTime;
    int hours = timeSpent.toMinutes() / 60;
    if (timeSpent.toMinutes() % 60 != 0) hours++;
    table.totalTime = table.totalTime + timeSpent;
    table.revenue += hours * hourCost;
    table.isOccupied = false;
    table.currentClient.clear();
}