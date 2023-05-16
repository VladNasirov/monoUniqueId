#pragma once
#include "AppModel.h"

AppModel::AppModel(const char* ip_address, int port_number, UniqueIdGenerator& idgen): id_gen(idgen) {
    start_time_ = std::chrono::system_clock::now();
    SERVER_IP = ip_address;
    PORT = port_number;
    counter_ = 0;
    state_ = ServerState::kIdle;

    WSAStartup(MAKEWORD(2, 2), &wsaData);


    sendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sendSocket == INVALID_SOCKET) {
        std::cerr << "Error creating send socket: " << WSAGetLastError() << std::endl;
        return;
    }
    recvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (recvSocket == INVALID_SOCKET) {
        std::cerr << "Error creating recv socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(recvSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding recv socket: " << WSAGetLastError() << std::endl;
        return;
    }

    if (listen(recvSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening recv socket: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "AppModel initialized on " << PORT << std::endl;
}
void AppModel::incCounter()
{

        std::lock_guard<std::mutex> lock(counterMutex_);
        counter_++;
}
void AppModel::decCounter()
{
        std::lock_guard<std::mutex> lock(counterMutex_);
        counter_--;
}
AppModel::AppModel(const AppModel& other) {
    SERVER_IP = other.SERVER_IP;
    PORT = other.PORT;
    sendSocket = other.sendSocket;
    recvSocket = other.recvSocket;
    wsaData = other.wsaData;
    counter_ = other.counter_;
    start_time_ = other.start_time_;
    state_ = other.state_;
}

AppModel::~AppModel() {
    close();
    std::cout << "AppModel destroyed" << std::endl;
}
void AppModel::close()
{
    closesocket(sendSocket);
    closesocket(recvSocket);
    WSACleanup();
}
void AppModel::setState(ServerState state) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = state;
}

ServerState AppModel::getState() {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}
bool AppModel::isAvailable()
{
    if (getState() == ServerState::kIdle)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool AppModel::sendDataToRemoteAppModel(const char* remoteIp, int remotePort) {
    state_ = ServerState::kSending;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sendSocket == INVALID_SOCKET) {
        std::cerr << "Error creating send socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(remoteIp);
    addr.sin_port = htons(remotePort);

    if (connect(sendSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to remote host: " << PORT << std::endl;
        WSAGetLastError();
        closesocket(sendSocket);
        return false;
    }
    
    std::string request;
    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client_addr.sin_port = htons(PORT);
    incCounter();
    Message message(counter_, client_addr, "0");
    std::cout << "Send Counter = " << counter_ << std::endl;
    std::ostringstream oss(request);
    oss << message;
    request = oss.str();
    std::cout << "App with port " << PORT << " send address with port " << client_addr.sin_port << std::endl;
    int bytesSent = send(sendSocket, request.c_str(), request.length(), 0);

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
        counter_--;
        closesocket(sendSocket);
        WSACleanup();
        return false;
    }

    closesocket(sendSocket);

    WSACleanup();
    //state_ = ServerState::kIdle;
    return true;
}

int AppModel::getCounter()
{
    return counter_;
}

std::chrono::nanoseconds AppModel::uptime() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
}

bool AppModel::receiveDataFromRemoteAppModel(int localPort) {
    setState(ServerState::kReceiving);
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET recvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (recvSocket == INVALID_SOCKET) {
        std::cerr << "Error creating recv socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(localPort);

    if (bind(recvSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding recv socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (listen(recvSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening recv socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    SOCKET clientSocket = accept(recvSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error accepting client: " << WSAGetLastError() << std::endl;
        return false;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived;
    {
        std::lock_guard<std::mutex> lock(recvMutex_);

    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return false;
    }
    }
    std::string message_str(buffer, bytesReceived);
    std::istringstream iss(message_str);
    Message message;
    iss >> message;
    std::cout << "App with port " << PORT << " recieved address with port " << message.client_address.sin_port << std::endl;
    std::cout << "recieved Counter = " << message.counter << " and client_address " << message.GetSourceAddressAsString() << std::endl;
    insertMessageToDBbyId(message);
    insertMessageToDBbyUniqueId(message);
    closesocket(clientSocket);
    closesocket(recvSocket);
    WSACleanup();
    //state_ = ServerState::kIdle;
    return true;
}

void AppModel::insertMessageToDBbyId(Message& msg) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    try {
        // MySQL
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("127.0.0.1:3306", "root", "2j4h5k");
        /* Connect to the MySQL test database */
        con->setSchema("requestdb");

        /* Insert data into the table */
        stmt = con->createStatement();
        std::string query = "INSERT INTO requestslog (counter, client_address) VALUES (" + std::to_string(msg.counter) + ", '" + msg.GetSourceAddressAsString() + "')";
        stmt->execute(query);
        delete stmt;
        delete con;

    }
    catch (sql::SQLException& e) {
        std::cout << "SQL error: " << e.what() << std::endl;
    }
}

void AppModel::insertMessageToDBbyUniqueId(Message& msg) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    try {
        msg.changeRequestId(id_gen.generateUniqueId(PORT, uptime()));
        // MySQL
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("127.0.0.1:3306", "root", "2j4h5k");
        /* Connect to the MySQL test database */
        con->setSchema("requestdb");

        /* Insert data into the table */
        stmt = con->createStatement();
        std::string query = "INSERT INTO requestsloguniqueid (counter, client_address, UniqueId) VALUES (" + std::to_string(msg.counter) + ", '" + msg.GetSourceAddressAsString() + "', '" + msg.request_id + "')";
        stmt->execute(query);
        delete stmt;
        delete con;

    }
    catch (sql::SQLException& e) {
        std::cout << "SQL error: " << e.what() << std::endl;
    }
}