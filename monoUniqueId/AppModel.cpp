#pragma once
#include "AppModel.h"
std::mutex uniquedb_mtx;
std::mutex db_mtx;
std::mutex sendMutex_;
std::mutex recvMutex_;
AppModel::AppModel(const char* ip_address, int port_number, std::shared_ptr<MessageQueue> Que, std::shared_ptr<UniqueIdGenerator> idgen):id_gen(idgen), MesQue(Que) {
    start_time_ = std::chrono::system_clock::now();
    SERVER_IP = ip_address;
    PORT = port_number;
    counter_ = 0;
    is_recv = false;
    {

        std::cout << "AppModel initialized on " << PORT << std::endl;
    }
}
void AppModel::start_recv() {
    is_recv = true;
    std::thread receivingThread(&AppModel::receiveDataFromRemoteAppModel, this);
    receivingThread.detach();
}

void AppModel::stop_recv()
{
    is_recv = false;
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

bool AppModel::sendDataToRemoteAppModel(const char* remoteIp, int remotePort) {
    
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
    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client_addr.sin_port = htons(PORT);

    if (connect(sendSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to remote host: " << PORT << std::endl;
        WSAGetLastError();
        closesocket(sendSocket);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(sendMutex_);
        std::string request;
        vc.increment(PORT);
        incCounter();
        Message message(counter_, client_addr, remotePort, "0", vc);
       /*std::cout << "Send Counter = " << counter_ << std::endl;*/
        std::ostringstream oss(request);
        oss << message;
        request = oss.str();
        int bytesSent = send(sendSocket, request.c_str(), request.length(), 0);

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
            decCounter();
            closesocket(sendSocket);
            WSACleanup();
            return false;
        }
    }
    closesocket(sendSocket);

    WSACleanup();
    //state_ = ServerState::kIdle;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true;
}


void AppModel::setVC(VectorClock vec)
{
    vc = vec;
    vc.setAppId(PORT);
}

int AppModel::getCounter()
{
    return counter_;
}

std::chrono::nanoseconds AppModel::uptime() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
}

bool AppModel::receiveDataFromRemoteAppModel() {
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
    addr.sin_port = htons(PORT);

    if (bind(recvSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding recv socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (listen(recvSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening recv socket: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Start receiving!!" << std::endl;
    while (is_recv) {
        SOCKET clientSocket = accept(recvSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting client: " << WSAGetLastError() << std::endl;
            continue;
        }
        else {

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            int bytesReceived;
                
                bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesReceived == SOCKET_ERROR) {
                    std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    continue;
                }
                else
            {

                //std::lock_guard<std::mutex> lock(recvAPPMutex_);
                std::string message_str(buffer, bytesReceived);
                std::istringstream iss(message_str);
                Message message;
                iss >> message;
                insertMessageToDBbyId(message);
                vc.update(message.MessageClock);
                insertMessageToDBbyUniqueId(message);
                MesQue->Push(message);
                
            }
        }
        closesocket(clientSocket);
    }

    closesocket(recvSocket);
    WSACleanup();
    return true;
}

const char* AppModel::getAddr()
{
    return SERVER_IP;
}
int AppModel::getPort()
{
    return PORT;
}
void AppModel::insertMessageToDBbyId(Message& msg) {
    std::lock_guard<std::mutex> lock(db_mtx);
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
    std::lock_guard<std::mutex> lock(db_mtx);
    try {
        msg.changeRequestId(id_gen->generateUniqueId(msg.MessageClock.getValueByPort(ntohs(msg.client_address.sin_port)),PORT, uptime()));
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
//void AppModel::processMessageQueue() {
//    while (is_recv||!(MesQue->IsEmpty())) {
//        // Получаем сообщения с нужным портом из очереди
//        std::vector<Message> messages = MesQue->GetMessagesByPort(PORT);
//
//        // Обрабатываем каждое сообщение
//        for (const auto& message : messages) {
//            // Генерируем уникальный идентификатор
//            std::string uniqueId = id_gen->generateUniqueId(PORT, uptime());
//
//            // Создаем копию сообщения и изменяем идентификатор
//            Message modifiedMessage = message;
//            modifiedMessage.changeRequestId(uniqueId);
//
//            // Вставляем сообщение в базу данных
//            insertMessageToDBbyUniqueId(modifiedMessage);
//        }
//    }
//}