#pragma once
#include <queue>
#include <iostream>
#include <mutex>
enum RequestState { kIdle, kProcessing, kSuccess, kFailed, kRecieved };

struct Request {
    Request(int send, int recv, RequestState state)
    {
        senderIndex = send;
        receiverIndex = recv;
        this->state=state;
    }
    int senderIndex;
    int receiverIndex;
    RequestState state = kIdle;
};

class RequestQueue {
public:
    void push(Request request)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        requests.push(request);
    }
    Request front()  {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requests.empty()) {
            throw std::logic_error("RequestQueue is empty");
        }
        return requests.front();
    }

    Request pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requests.empty()) {
            throw std::logic_error("RequestQueue is empty");
        }
        Request request = requests.front();
        requests.pop();
        return request;
    }
    bool empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requests.empty())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    int size() 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        int size= requests.size();
        return size;

    }
    void remove(int index) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp;
        while (!requests.empty()) {
            Request request = requests.front();
            requests.pop();
            if (request.senderIndex != index && request.receiverIndex != index) {
                temp.push(request);
            }
        }
        requests = std::move(temp);
    }

    Request at(int index)  {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp = requests;
        while (!temp.empty()) {
            Request request = temp.front();
            temp.pop();
            if (request.senderIndex == index || request.receiverIndex == index) {
                return request;
            }
        }
        throw std::out_of_range("Invalid index");
    }
    Request findRequestBySenderIndex(int index)  {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp = requests;
        while (!temp.empty()) {
            Request request = temp.front();
            temp.pop();
            if (request.senderIndex == index&& request.state!=kSuccess) {
                return request;
            }
        }
        throw std::logic_error("Request not found");
    }
    Request findRequestByRecvIndex(int index) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp = requests;
        while (!temp.empty()) {
            Request request = temp.front();
            temp.pop();
            if (request.receiverIndex == index && request.state != kSuccess) {
                return request;
            }
        }
        throw std::logic_error("Request not found");
    }
    void changeRequestState(int receiverIndex, RequestState newState) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp;
        while (!requests.empty()) {
            Request request = requests.front();
            requests.pop();
            if (request.receiverIndex == receiverIndex) {
                request.state = newState;
            }
            temp.push(request);
        }
        requests = std::move(temp);
    }

    void removeSuccessRequests() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Request> temp;
        while (!requests.empty()) {
            Request request = requests.front();
            requests.pop();
            if (request.state != RequestState::kSuccess) {
                temp.push(request);
            }
        }
        requests = std::move(temp);
    }
private:
    std::mutex mutex_;
    std::queue<Request> requests;
};
