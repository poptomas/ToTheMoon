

#pragma once

#include <filesystem>
#include <chrono>
#include <cctype>
#include <functional>

#include "mapping.h"
#include "dataset.h"
#include "utilities.h"
#include "connection.h"
#include "thread_controller.h"

using seconds = std::chrono::seconds;

/**
 * @brief Processor class which takes care of user input processing,
 * commands handling - validation and further transfer of the command
 */
class Processor {
public:
	Processor(const GenericConn& input_conn)
		: conn(std::make_shared<GenericConn>(input_conn)),
		slash('/'), delimiter(' ') {
		set_supported_commands();
	}

	/**
	 * @brief Gets user input either from std::cin or from the commandline
	 */
	std::vector<std::string> receive_user_input(int argc, char** argv);

	/**
	 * @brief Reads standard input (non-blocking) in a separate thread
	 * and transfers the input appropriately
	 * to appropriate instance methods for a further processing
	 * - runs until the user withdraws or ends the program by force
	 * @param run - atomic boolean value which determines
	 * whether the reading of the input should keep going
	 */
	void read_cin(std::atomic<bool>& run, std::shared_ptr<ThreadController>& c);

	/**
	 * @brief A guide for the first launch
	 * - since it is not a triggered command
	 * it has its separarators added manually
	 */
	void print_help_initial() const;

private: // methods
	// input parsers
	std::vector<std::string> parse_args(int argc, char** argv);
	inline std::vector<std::string> parse_args_cin();
	
	/**
	 * @brief sets supported commands of the library
	 * - string constants kept on a single place for further extensions
	 */
	void set_supported_commands();

	/**
	 * @brief Processes commands via parameterless void functor.
	 * @param user_input
	 */
	void process_simple_command(const std::string& user_input) const;

	/**
	 * @brief Processes commands via void functor with one argument.
	 * @param input_tokens - tokenized user input
	 */
	void process_param_command(std::vector<std::string>& input_tokens);

	/**
	 * @brief Asks to add a cryptrocurrency to a user's watchlist
	 * via transferring the query to a concerned class
	 * - upon invalid values prints error message
	 * @param user_v entered value from the user
	 */
	void try_add_cryptocurrency(const std::string& user_v);

	/**
	 * @brief Asks to remove a cryptrocurrency from a user's watchlist
	 * via transferring the query to a concerned class
	 * - upon invalid values prints error message
	 * @param user_v entered value from the user
	 */
	void try_remove_cryptocurrency(const std::string& user_v);

	/**
	 * 
	 * @brief Adds cash to the account
	 * - upon invalid values prints error message
	 * @param user_v entered value from the user
	 */
	void try_deposit(const std::string& user_v);

	// simple commands
	void call_history() const;
	void call_current() const;
	void call_market() const;
	void call_withdraw() const;
	void print_help() const;

	void print_commands_common(bool found, const std::string& user_input) const;
	void get_indicators() const;

private: // fields
	/**
	 * @brief Currently supported options for an user.
	 */
	enum class Options {
		WithdrawCash, GetCurrent,
		GetMarket, GetHistory,
		GetHelp, GetIndicators,
		Add, Remove, DepositCash
	};

	/**
	 * @brief Options mapping to the defined string constants.
	 */
	std::map<Options, std::string> enum_mapper;

	/**
	 * @brief Mapping from the user input to function callbacks.
	 */
	std::map<std::string, std::function<void()>> simple_func_mapper;
	std::map<std::string, std::function<void(const std::string&)>> param_func_mapper;

	/**
	 * @brief A hook to the API connection.
	 */
	std::shared_ptr<GenericConn> conn;

	// input parsing constants
	char slash;
	char delimiter;
};

#ifndef PRINT_FUNCTIONS

inline static void print_time_elapsed(long long time, const seconds& delay) {
	print("Getting data took: ", time, " ms (consider delay afterwards: ", delay.count(), " s)\n");
}

inline static void print_empty_watchlist_warning() {
	print("[WARNING] Make sure to use add [symbol] command, otherwise your watchlist is empty\n");
}

inline static void print_end() {
	print("Program ended successfully\n");
}

inline static void print_unknown_action(const std::string& user_input) {
	print("Unknown action: \"" + user_input + "\"\n");
}

inline static void print_header() {
	print("ToTheMoon (Cryptocurrency Trading Bot)\n");
	print("For cryptocurrency symbols see https://coinmarketcap.com/exchanges/binance \n");
	// slashes e.g. BTC/USDT can be included - when parsing the input it is removed anyway
	print("- example: BTCUSDT ETHUSDT SOLUSDT ADAUSDT \n"); 
	print("Enter symbols (case insensitive): "); 
	// final decision - no input limit set, nevertheless, it is recommended to keep it between 1 to 5
}

inline static void print_invalid_amount() {
	print("Invalid amount\n");
}

inline static void print_invalid_operation() {
	print("Invalid operation\n");
}

inline static void print_added(const std::string& symbol) {
	print(symbol, " added successfully\n");
}

inline static void print_removed(const std::string& symbol) {
	print(symbol, " removed successfully\n");
}

inline static void print_deposit(double value) {
	print(value, " USD added\n");
}

inline static void print_separator() {
	print("-------------------------------------\n");
}

void Processor::print_help_initial() const {
	print_separator();
	print_help();
	print_separator();
}

void Processor::print_help() const {
	print("Supported commands (case insensitive): \n");
	for (auto&& [option, strval] : enum_mapper) {
		print(strval, "\n");
	}
}

void Processor::print_commands_common(
	bool found, const std::string& user_input
) const {
	if (!found) {
		print_unknown_action(user_input);
		print_help();
	}
}

#endif // !PRINT_FUNCTIONS

#ifndef PARSING

std::vector<std::string> Processor::receive_user_input(int argc, char** argv) {
	if (argc > 1) {
		// support enabled (nevertheless, without header
		// which may help the user to get oriented)
		return parse_args(argc, argv);
	}
	else {
		print_header();
		return parse_args_cin();
	}
}

inline std::vector<std::string> Processor::parse_args(int argc, char** argv) {
	std::vector<std::string> vect (argv + 1, argv + argc);
	for (auto&& input : vect) {
		erase_from(input, slash);
		to_uppercase(input);
	}
	return vect;
}

std::vector<std::string> Processor::parse_args_cin() {
	std::string input;
	std::getline(std::cin, input);
	// Uniformity lacks in the field of
	// cryptocurrencies in terms of pairs definition
	// -> BTC/USDT vs BTCUSDT
	erase_from(input, slash);
	to_uppercase(input);
	return tokenize(input, delimiter);
}
#endif // !PARSING

#ifndef COMMANDS

void Processor::set_supported_commands() {
	// string constants are kept on a single place
	map_init(enum_mapper)
		(Options::GetHelp, "help")
		(Options::DepositCash, "deposit [value]")(Options::WithdrawCash, "withdraw")
		(Options::GetCurrent, "current")(Options::GetHistory, "history")
		(Options::GetMarket, "market")(Options::GetIndicators, "indicators")
		(Options::Add, "add [symbol]")(Options::Remove, "remove [symbol]");
	// func_mapper added for the straightforward parameterless void commands
	map_init(simple_func_mapper)
		("history", std::bind(&Processor::call_history, this))
		("current", std::bind(&Processor::call_current, this))
		("market", std::bind(&Processor::call_market, this))
		("withdraw", std::bind(&Processor::call_withdraw, this))
		("indicators", std::bind(&Processor::get_indicators, this))
		("help", std::bind(&Processor::print_help, this));
	map_init(param_func_mapper)
		("deposit", std::bind(
			&Processor::try_deposit, this, std::placeholders::_1
		))
		("add", std::bind(
			&Processor::try_add_cryptocurrency, this, std::placeholders::_1
		))
		("remove", std::bind(
			&Processor::try_remove_cryptocurrency, this, std::placeholders::_1
		));
}

void Processor::try_add_cryptocurrency(const std::string& symbol) {
	conn->try_add_cryptocurrency(symbol) ?
		print_added(symbol) : print_invalid_operation();
}

void Processor::try_remove_cryptocurrency(const std::string& symbol) {
	conn->try_remove_cryptocurrency(symbol) ?
		print_removed(symbol) : print_invalid_operation();

}

void Processor::process_simple_command(const std::string& user_input) const {
	bool found = false;
	for (auto&& [key, func_call] : simple_func_mapper) {
		if (key == user_input) {
			func_call();
			found = true;
		}
	}
	print_commands_common(found, user_input);
}

void Processor::process_param_command(std::vector<std::string>& input_tokens) {
	bool found = false;
	for (auto&& [key, func_call] : param_func_mapper) {
		if (key == input_tokens.at(0)) {
			to_uppercase(input_tokens.at(1));
			func_call(input_tokens.at(1));
			found = true;
		}
	}
	print_commands_common(found, stringify(input_tokens));
}

void Processor::try_deposit(const std::string& user_input) {
	try {
		double amount = convert_string_to<double>(user_input);
		if (amount > 0) {
			conn->deposit(amount);
			print_deposit(amount);
		}
		else {
			print_invalid_amount();
		}
	}
	catch (std::invalid_argument&) {
		print_invalid_amount();
	}
}

void Processor::get_indicators() const {
	conn->show_indicators();
}

void Processor::call_current() const {
	conn->show_current_state();
}

void Processor::call_history() const {
	conn->show_transactions();
}

void Processor::call_market() const {
	conn->show_current_values();
}

void Processor::call_withdraw() const {
	conn->show_result();
}

#endif // !COMMANDS

#ifndef INPUT_READER

void Processor::read_cin(std::atomic<bool>& run, std::shared_ptr<ThreadController>& c) {
	std::string user_input;
	while (run.load()) {
		getline(std::cin, user_input);
		// prevent unknown action spamming with empty cin
		if (user_input.empty()) {
			continue;
		}
		to_lowercase(user_input);
		trim(user_input);
		print_separator();
		std::vector<std::string> tokens = tokenize(user_input, ' ');
		// special case
		if (tokens.size() == 1 
			&& tokens.at(0) == enum_mapper.at(Options::WithdrawCash)) {
			run.store(false);
			c->kill();
			process_simple_command(user_input);
		}
		else if (tokens.size() == 1) {
			process_simple_command(user_input);
		}
		else if (tokens.size() == 2) {
			process_param_command(tokens);
		}
		else {
			print_unknown_action(user_input);
			print_help();
		}
		print_separator();
	}
}

#endif // !INPUT_READER
