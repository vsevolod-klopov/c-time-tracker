#pragma once
#include <chrono>
#include <iostream>
#include "event.h"
#include <queue>
#include <mutex>

class EventQueue {
    std::queue<Event> events;
    mutable std::mutex mutex_;

public:
    void push(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        events.push(event);
    }

    Event get() {
        std::lock_guard<std::mutex> lock(mutex_);
        Event event = events.front();
        return event;
    }
    void pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        events.pop();
    }
	bool empty() {
		std::lock_guard<std::mutex> lock(mutex_);
		return events.empty();
	}

	EventQueue() = default;
};