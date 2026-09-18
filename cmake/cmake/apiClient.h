#pragma once

#include "json.hpp"
#include <string>
#include "event.h"
#include "eventQueue.h"
#include <atomic>

class ApiClient
{
public:
    ApiClient(const std::string& url);
    // Process queue: accumulate events and send when threshold/time reached
    void processQueue(EventQueue& Queue);

    // For tests or forced flush
    bool sendNow();
    // Save current buffer to file (used on shutdown instead of sending)
    void backupToFile(const std::string& path);

private:
    std::string url_;
	nlohmann::json events_buffer;
    std::chrono::steady_clock::time_point last_send_;
    const int send_interval_seconds = 30;
    const size_t send_threshold = 10;
};