#pragma once
#include <queue>
#include <algorithm>
#include "message.h"
class MessageQueue {
public:
    void Push(const Message& message);
    MessageQueue() = default;

    Message Pop();
    size_t GetCount();
    bool IsEmpty();
    std::vector<Message> GetMessagesByPort(int PORT);
    Message PopByPort(int PORT);
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
private:
    std::queue<Message> queue_;
    struct SortByMessageClock {
        bool operator()(const Message& message1, const Message& message2) const {
            return message1.MessageClock <= message2.MessageClock;
        }
    };

    
};
