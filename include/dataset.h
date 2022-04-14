
#pragma once

#include <filesystem>

#include "utilities.h"
#include "connection.h"

/**
 * @brief (optional) used as a bypass to a Python script
 * to access gold data creation
 */
class DataHandler {
public:
	DataHandler() : extension(".csv") {}
	DataHandler(const DataHandler&) = delete;
	DataHandler(DataHandler&&) = delete;
	DataHandler& operator=(const DataHandler&) = delete;
	void download_initial_values(const std::vector<std::string>&);
private:
	void download_initial_values_py(const std::vector<std::string>&);
	std::string extension;
};

void DataHandler::download_initial_values_py(const std::vector<std::string>& crypto_pairs) {
	std::string command = "python ../../../data/data_download.py --pairs ";
	for (auto&& cr_pair : crypto_pairs) {
		command += cr_pair;
		if (cr_pair != crypto_pairs.back()) {
			command += ' ';
		}
	}
	int result = system(command.c_str()); // Suboptimal solution
		// Nevertheless, correct initial values are 
		// crucial for the rest of the application
		// since it relies on the correct beginning 
		// - upcoming time series values rely on the previous ones
		// UPDATED: works as gold data
	if(result == -1) {
		print("An error during system call occured\n");
	}
}

void DataHandler::download_initial_values(const std::vector<std::string>& user_input) {
	std::vector<std::string> relevant_pairs;
	for (auto&& crypto_pair : user_input) {
		if (cryptocurrency_pairs.find(crypto_pair) != cryptocurrency_pairs.end()) {
			relevant_pairs.push_back(crypto_pair);
		}
	}
	download_initial_values_py(relevant_pairs);
}

