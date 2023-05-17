#include "MessageQueue.h"
std::mutex MessageQuemutex_;

void MessageQueue::Push(const Message& message) {
    std::lock_guard<std::mutex> lock1(MessageQuemutex_);
    queue_.push(message);
}

Message MessageQueue::Pop() {
    std::lock_guard<std::mutex> lock1(MessageQuemutex_);
    Message message = queue_.front();
    queue_.pop();
    return message;
}

bool MessageQueue::IsEmpty() {
    std::unique_lock<std::mutex> lock2(MessageQuemutex_);
    return queue_.empty();
}
std::vector<Message> MessageQueue::GetMessagesByPort(int PORT) {
    std::lock_guard<std::mutex> lock3(MessageQuemutex_);
    std::vector<Message> messages;

    while (!queue_.empty()) {
        const Message& message = queue_.front();
        if (message.recv_PORT == PORT) {
            messages.push_back(message);
        }
        queue_.pop();
    }


    return messages;
}
Message MessageQueue::PopByPort(int PORT) {
    std::lock_guard<std::mutex> lock4(MessageQuemutex_);
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
    return foundMessage;
}

size_t MessageQueue::GetCount() {
    std::unique_lock<std::mutex> lock5(MessageQuemutex_);
    return queue_.size();
}