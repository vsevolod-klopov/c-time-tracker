#include "apiClient.h"
#include <iostream>
#include "eventQueue.h"
#include "eventSource.h"
#include <chrono>
#include "json.hpp"
#include <thread>
#include <cstdlib>
#include <windows.h>
#include <curl/curl.h>
#include <fstream>

ApiClient::ApiClient(const std::string& url) : url_(url), last_send_(std::chrono::steady_clock::now()) {}

void ApiClient::processQueue(EventQueue& Queue) {
	std::string agent_name;
	if (!events_buffer.contains("payload") || !events_buffer["payload"].is_array()) {
		events_buffer["payload"] = nlohmann::json::array();
	}
	Event ev("", "", false, "");
	while (!Queue.empty()) {
		ev = Queue.get();
		Queue.pop();
		if (agent_name.empty()) {
			agent_name = ev.agent_name;
		}
		events_buffer["payload"].push_back(ev.toJson());
	}

	if (!agent_name.empty()) {
		events_buffer["agent_id"] = agent_name;
	}
	else {
		events_buffer["agent_id"] = "unknown";
	}

	auto nowSys = std::chrono::system_clock::now();
	events_buffer["timestamp"] = (std::int64_t)std::chrono::system_clock::to_time_t(nowSys);

	size_t payload_size = 0;
	if (events_buffer.contains("payload") && events_buffer["payload"].is_array()) {
		payload_size = events_buffer["payload"].size();
	}

	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_send_).count();

	// Если накопилось достаточно событий или прошло достаточно времени - отправляем
	if (payload_size >= send_threshold || elapsed >= send_interval_seconds) {
		bool ok = sendNow();
		if (ok) last_send_ = std::chrono::steady_clock::now();
	}
}

void ApiClient::backupToFile(const std::string& path) {
	try {
		std::ofstream ofs(path, std::ios::out | std::ios::trunc);
		if (!ofs) {
			std::cout << "Failed to open backup file: " << path << std::endl;
			return;
		}
		// Ensure payload exists
		if (!events_buffer.contains("payload") || !events_buffer["payload"].is_array()) {
			events_buffer["payload"] = nlohmann::json::array();
		}
		ofs << events_buffer.dump(2);
		ofs.close();
		std::cout << "Buffer backed up to " << path << std::endl;
	} catch (const std::exception& ex) {
		std::cout << "Exception while backing up buffer: " << ex.what() << std::endl;
	}

}

bool ApiClient::sendNow() {
	if (!events_buffer.contains("payload") || !events_buffer["payload"].is_array() || events_buffer["payload"].empty()) {
		// ничего отправлять
		return false;
	}

	std::string postData = events_buffer.dump();
	bool success = false;

	std::cout << "Preparing to send payload, events=" << events_buffer["payload"].size() << std::endl << std::flush;

	CURL* curl = curl_easy_init();
	if (curl == nullptr) {
		std::cout << "curl_easy_init failed." << std::endl;
		return false;
	}

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "x-api-key: secret");
	curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)postData.size());
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		std::cout << "curl perform failed: " << curl_easy_strerror(res) << std::endl;
	}
	long response_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (res == CURLE_OK && response_code >= 200 && response_code < 300) {
		success = true;
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (success) {
		events_buffer["payload"] = nlohmann::json::array();
		std::cout << "Events sent successfully." << std::endl;
	} else {
		std::cout << "Failed to send events." << std::endl;
		size_t sz = 0;
		if (events_buffer.contains("payload") && events_buffer["payload"].is_array()) sz = events_buffer["payload"].size();
		if (sz > 100) {
			nlohmann::json newarr = nlohmann::json::array();
			for (size_t i = sz > 100 ? sz - 100 : 0; i < sz; ++i) {
				newarr.push_back(events_buffer["payload"][i]);
			}
			events_buffer["payload"] = newarr;
		}
	}

	return success;
}
