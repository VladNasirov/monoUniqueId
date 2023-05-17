#pragma once
#include <queue>
#include <algorithm>
#include "message.h"

class MessageQueue {
public:
    void Push(const Message& message) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(message);
        lock.unlock();
        sortQueue();
    }

    Message Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        Message message = queue_.front();
        queue_.pop();
        return message;
    }

    bool IsEmpty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    std::vector<Message> GetMessagesByPort(int PORT) {
        std::vector<Message> messages;
        std::unique_lock<std::mutex> lock(mutex_);

        while (!queue_.empty()) {
            const Message& message = queue_.front();
            if (message.recv_PORT == PORT) {
                messages.push_back(message);
            }
            queue_.pop();
        }

        lock.unlock();

        return messages;
    }
    Message PopByPort(int PORT) {
        std::unique_lock<std::mutex> lock(mutex_);
        std::queue<Message> newQueue;
        Message foundMessage;
        while (!queue_.empty()) {
            Message message = queue_.front();
            queue_.pop();
            if (message.recv_PORT == PORT) {
                foundMessage = message;
            }
            else {
                newQueue.push(message);
            }
        }
        queue_ = newQueue;
        lock.unlock();
        return foundMessage;
    }
private:
    std::queue<Message> queue_;
    std::mutex mutex_;
    struct SortByMessageClock {//дописать
        bool operator()(const Message& message1, const Message& message2) const {
            return message1.MessageClock <= message2.MessageClock;
        }
    };

    void sortQueue() {
        std::priority_queue<Message, std::vector<Message>, SortByMessageClock> sortedQueue;
        while (!queue_.empty()) {
            sortedQueue.push(queue_.front());
            queue_.pop();
        }
        while (!sortedQueue.empty()) {
            queue_.push(sortedQueue.top());
            sortedQueue.pop();
        }
    }
};
