#pragma once
//#ifndef EVENT_H
//#define EVENT_H

#include <chrono>
#include <string>
#include "json.hpp"

struct Event{
    std::string agent_name;
    std::string window_title;
    std::string process_name;
    bool active;
    std::chrono::system_clock::time_point timestamp;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["agent_name"] = agent_name;
		j["window_title"] = window_title;
		j["process_name"] = process_name;
		j["active"] = active;
		j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
		return j;
	}

    Event(const std::string& window_title, const std::string& process_name, bool active, const std::string& agent_name)
        : agent_name(agent_name), window_title(window_title), process_name(process_name), active(active), timestamp(std::chrono::system_clock::now()){
    }
};