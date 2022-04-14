#pragma once
#include <unordered_map>
#include <map>
#include <deque>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <numeric>

#include "stats.h"
#include "utilities.h"

namespace fs = std::filesystem;
using matrix = std::deque<std::deque<double>>;
using data_map = std::unordered_map<std::string, matrix>;
using crypto_map = std::unordered_map<std::string, std::shared_ptr<CryptoToken>>;
using action_map = std::unordered_map<Action, std::string>;

/**
 * @brief Backbone of the algorithmic trading (AT) bot
 * Analyzer class
 * - prepares a "dataset" with whom the AT bot operates
 * - utilizes statistical indicators for signal (buy/hold/sell) detection
 * - takes care of money management strategy
 */
class Analyzer {
public:
	/**
	 * 
	 * @brief A set of initialization methods
	 */
	void init();

	/**
	 * @brief API function used to process various crypto tokens (symbols)
	 * @param data - input data - keys as symbols,
	 * values as cryptocurrency tokens with info needed for the analysis
	 * @param shall_add - whether the findings should be added to the dataset or not
	 * -- adding happens only once upon a time
	 */
	void get_analysis(crypto_map& data, bool shall_add);

	/**
	 * @brief Prepares dataset from previously created csv file
	 * @param symbols - crypto tokens (i.e. BTCUSDT, ETHUSDT) whose dataset is supposed to be created
	 * - with the help of the dataset file which consists of initial values
	 */
	void prepare_values_from_file(const std::vector<std::string>& symbols);

	/**
	 * @brief Prepares dataset from the polished output of rest api call
	 * @param dictionary - a dictionary where keys are symbols (i. e. BTCUSDT), values are column records
	 */
	void prepare(const std::unordered_map<std::string, std::deque<double>>& dictionary);

	/**
	 * @brief Removes a cryptocurrency from the watchlist
	 * if the user possesses a cryptocurrency of this kind it is
	 * at the current exchange rate
	 * @param symbol - cryptocurrency
	 */
	void remove(const std::string& symbol);

	/**
	 * @brief Adds USD to user's account 
	 * @param value amount of cash in USD
	 */
	void deposit(double value);

	/**
	 * @brief Converts all currently possessed cryptocurrencies to USD
	 * @param map - keys: cryptocurrency symbols, values: pointers to cryptocurrencies
	 * (with their value and amount embedded)
	 * @returns USD which the user obtains (if it were real) 
	 */
	double withdraw(const crypto_map& map);

	/**
	 * @returns an user's current USD asset.
	 */
	double get_balance() const;

	/**
	 * @brief Print current market exchange rate
	 * of user's watchlist
	 */
	void print_current(const crypto_map&) const;

	/**
	 * @brief Prints last couple of accomplished transactions
	 * -- full history is stored in a separate csv file
	 */
	void print_transactions() const;

	/**
	 * @brief Shows indicators of Relative Strength Index (RSI), Bollinger Bands (BB)
	 * for a user's current watchlist
	 * @see https://www.investopedia.com/terms/r/rsi.asp
	 * @see https://www.investopedia.com/terms/b/bollingerbands.asp.
	 */
	void print_indicators() const;
	
private: // methods
	template<typename T, typename ...Args>
	friend std::shared_ptr<T> create_shared(Args&& ...args);

	Analyzer() : dataset(), assets(), transactions(),
		out_dir("transactions"), out_fname("results"),
		extension(".csv"), us_dollar("USD") {
		init();
	}
	Analyzer(Analyzer&&) = delete;
	Analyzer(const Analyzer&) = delete;
	Analyzer& operator=(const Analyzer&) = delete;

	/**
	 * @param symbol - cryptocurrency
	 * @param action - whether to buy or sell
	 * @param price - current exchange rate of the cryptocurrency given 
	 */
	void print_signal(const std::string& symbol, const Action& action, double price) const;

	/**
	 * @brief Prints current dataset
	 * - primarily for debugging purposes.
	 */
	void print_dataset() const;

	/**
	 * @brief Prepares one row of the dataset.
	 */
	void prepare_single(const std::pair<std::string, std::deque<double>>&);

	/**
	 * @brief Sets typical actions - decisions to be
	 * done for each user desired cryptocurrency.
	 */
	void set_actions();

	/**
	 * @brief Buys an amount of cryptocurrency (according to strategy)
	 * at a given exchange rate.
	 * @param symbol - cryptocurrency which we want to buy
	 * @param price - current exchange rate of the cryptocurrency given 
	 */
	void process_buy_signal(const std::string& symbol, double price);
	
	/**
	 * @brief Takes currently possessed amount of cryptocurrency and converts it to USD
	 * according to current exchange rate - afterwards it is added to a user's account.
	 * - an alternative could be to use a similar strategy mentioned in the buy signal
	 * @param symbol - cryptocurrency which we want to sell
	 * @param price - current exchange rate of the cryptocurrency given 
	 */
	void process_sell_signal(const std::string& symbol, double price);
	
	/**
	 * @brief Adds a transaction 
	 * record to a local storage (contains last couple of records).
	 * @param symbol - cryptocurrency
	 * @param xrate - exchange rate
	 * @param amount - amount of cryptocurrency we want to process a transaction of
	 * @param action - whether to buy or sell
	 */
	void create_transaction(const std::string& symbol, double xrate, double amount, Action action);

	/**
	 * @brief 
	 * @returns A new row which may be further added to the dataset 
	 */
	std::deque<double> set_technical_indicators(const std::string&, double);

	/**
	* @brief Calculates Bollinger Bands
	* @param symbol - cryptocurrency
	* @param price - current exchange rate of the cryptocurrency
	* @param cells - row cells to be filled in order to form a dataset row
	* @param period - window which we consider when calculating bollinger bands
	* @see https://www.investopedia.com/terms/b/bollingerbands.asp
	*/
	Action set_bollinger_bands(const std::string& symbol, double price, std::deque<double>& cells, size_t period);

	/**
	* @brief Calculates Relative Strengh Index
	* @param symbol - cryptocurrency
	* @param price - current exchange rate of the cryptocurrency
	* @param cells - row cells to be filled in order to form a dataset row
	* @param period - window which we consider when calculating RSI
	* @see https://www.investopedia.com/terms/r/rsi.asp
	*/
	Action set_rsi(const std::string& symbol, double price, std::deque<double>& cells, size_t period);

	/**
	* @brief Prepares output transaction file where all transactions are accomplished
	* - it needs to clean up a potential previous run first
	*/
	void prepare_output_file();

	/**
	 * @brief Adds a row to the csv file
	 * @param transaction - pointer to transactions which holds all the needed
	 * info to be filled in the file
	 */
	void append_to_file(const std::shared_ptr<Transaction>& transaction);

	/**
	 * @brief Receive filename of the output file.
	 */
	inline std::string get_filename() const;

	/**
	 * @brief Writes the header of the output csv file
	 */
	void write_header() const;

private: // fields
	/**
	 * @brief trading fee per transaction (not deposit nor withdraw).
	 */
	const double trading_fee = 0.005;

	/**
	 * @brief modifiable pseudo strategy for the bot to split the money upon buying decision.
	 */
	const int investment_split = 10; 

	/**
	 * @brief required number of signal triggered before an action is made.
	 */
	const size_t signal_threshold = 5;

	/**
	 * @brief latest transactions "window"
	 * - whole transaction history is kept in the csv file
	 */
	const size_t max_transactions = 20;

	/**
	 * @brief last record of each (user desired)
	 * cryptocurrency symbol with indicator info
	 */
	std::map<std::string, std::deque<double>> last_records;

	/**
	 * @brief A map consiting of consecutive signals for each
	 * cryptocurrency in the user's watchlist.
	 */
	std::map<std::string, size_t> signal_counter_map;

	/**
	 * @brief A hook to statistics class to use its formulae.
	 */
	std::unique_ptr<StatsCalc> calc;

	/**
	 * @brief Dataset dictionary collection.
	 */
	data_map dataset;

	/**
	 * @brief Enum mapping to string constants.
	 */
	std::unordered_map<Action, std::string> action_mapper;

	/**
	 * @brief A map of currently possessed user assets.
	 */
	std::unordered_map<std::string, double> assets;

	/**
	 * @brief A double ended queue consiting
	 * of last couple of accomplished transactions.
	 */
	std::deque<std::shared_ptr<Transaction>> transactions;

	std::string out_dir;
	std::string out_fname;
	std::string extension;
	std::string us_dollar;
};

#ifndef PRINT_FUNCTIONS

void Analyzer::print_transactions() const {
	if (transactions.empty()) {
		print("No transactions have been accomplished yet\n");
	}
	else {
		print("Transactions (full history in ", get_filename(), ")\n");
		int row_num = 1;
		for (auto it = transactions.rbegin(); it != transactions.rend(); ++it) {
			print(row_num, ": ", (*it)->get_datetime(),
				" Name: ", (*it)->get_name(),
				" Exchange rate: ", (*it)->get_xrate(),
				" Amount: ", (*it)->get_amount(),
				" Action: ", (*it)->get_action(), '\n'
			);
			++row_num;
		}
	}
}

void Analyzer::print_dataset() const {
	for (auto&& [key, matrix] : dataset) {
		print(key, "\n");
		for (auto&& deque : matrix) {
			for (double d : deque) {
				print(d, " ");
			}
			print("\n");
		}
		print("\n");
	}
}

void Analyzer::print_current(const crypto_map& input) const {
	for (auto&& [key, value] : assets) {
		print("[", key, " : ", value, "]\n");
	}
	double withdraw_v = assets.at(us_dollar);
	for (auto&& [name, amount] : assets) {
		if (name != us_dollar) {
			double current_v = input.at(name)->get_value();
			double in_usd = current_v * amount;
			withdraw_v += in_usd;
		}
	}
	print("Estimated withdrawal: ", withdraw_v, " ", us_dollar, "\n");
}

inline static void print_indicators_header() {
	auto&& time = get_current_datetime();
	print("Indicators at ", time, "\n");
	print("RSI = Relative Strength Index \n");
	print("BB = Bollinger Bands\n\n");
}

void Analyzer::print_indicators() const {
	print_indicators_header();
	for (auto&& [symbol, value] : last_records) {
		print("[ --- ", symbol, " --- ]\n");
		print("- RSI: ", value[0], " % \n");
		print("- BB: Lowerband: ", value[1], " ", us_dollar,
			", Upperband: ", value[2], " ", us_dollar, "\n"
		);
		print("- Current value: ", value[value.size() - 1], " ", us_dollar, "\n\n");
	}
}

inline static void print_RSI_data(double rsi, double avg_up, double avg_down) {
	print("RSI: ", rsi, " % Average up: ", avg_up, " Average down: ", avg_down, "\n");
}

inline static void print_BB_data(double lower, double upper, double mean, double std_deviation) {
	print("BB - Lower band: ", lower, " Upper band: ", upper, "\n");
	print("- Mean: ", mean, " Std dev: ", std_deviation, "\n");
}

inline static void print_suggestion(const std::string& indicator, const std::string& suggestion) {
	print(indicator, " suggests: ", suggestion, "\n");
}

inline static void print_row(int iteration, const std::deque<double>& cells) {
	print("Iteration ", iteration, ": ");
	for (auto&& v : cells) {
		print(v, " ");
	}
	print("\n");
}

void Analyzer::print_signal(const std::string& symbol, const Action& action, double xrate) const {
	print("\n[", action_mapper.at(action), " SIGNAL]: ", symbol, "\n");
	print(" - at exchange rate : ", xrate, "\n\n");
}

inline static void print_insufficient_funds(const std::string& symbol, double price) {
	print("\n[BUY SIGNAL]: ", symbol, " at ", price, " USD  - insufficient funds\n\n");
}

inline static void print_cant_sell(const std::string& symbol, double price) {
	print("\n[SELL SIGNAL]: ", symbol, " at ", price, " USD  - could not sell, I dont have any\n\n");
}

inline static void print_debug_trigger_signal(const std::string& action, size_t streak) {
	print("Trigger: ", action, ": ", streak, "x\n");
}

#endif // !PRINT_FUNCTIONS

#ifndef INITIALIZATION

void Analyzer::init() {
	assets[us_dollar] = 0;
	set_actions();
	prepare_output_file();
}

void Analyzer::set_actions() {
	map_init<action_map>(action_mapper)
		(Action::DEFAULT, "Default")(Action::HOLD, "Hold")
		(Action::SELL, "Sell")(Action::BUY, "Buy");
}

#endif // !INITIALIZATION

#ifndef ANALYSIS_ENTRYPOINT

void Analyzer::get_analysis(crypto_map& data, bool shall_add) {
	for (auto&& [symbol, crypto_token] : data) {
		double cr_value = crypto_token->get_value();
		auto&& new_row = set_technical_indicators(symbol, cr_value);
		if (shall_add) {
			dataset.at(symbol).pop_front();
			dataset.at(symbol).push_back(new_row);
		}
	}
}

#endif // !ANALYSIS_ENTRYPOINT

#ifndef ASSETS_HANDLING

double Analyzer::get_balance() const {
	return assets.at(us_dollar);
}

void Analyzer::deposit(double value) {
	assets[us_dollar] += value;
}

double Analyzer::withdraw(const crypto_map& input) {
	double withdraw_v = assets[us_dollar];
	assets.erase(us_dollar);
	for (auto&& [name, amount] : assets) {
		double current_v = input.at(name)->get_value();
		double in_usd = current_v * amount;
		withdraw_v += in_usd;
	}
	return withdraw_v;
}
#endif // !ASSETS_HANDLING

#ifndef TECHNICAL_INDICATORS

Action Analyzer::set_bollinger_bands(
	const std::string& key, double value,
	std::deque<double>& cells, size_t period
) {
	matrix mat;
	std::vector<double> close_values;
	mat = dataset.at(key);
	for (auto it = mat.end() - period; it != mat.end(); ++it) {
		auto&& row = *it;
		close_values.push_back(row[row.size() - 1]);
	}
	close_values.push_back(value);
	double mean = calc->get_moving_average(close_values);
	double std_deviation = calc->get_standard_deviation(close_values, mean);

	double lowerband = mean - 2 * std_deviation;
	double upperband = mean + 2 * std_deviation;
	cells.insert(cells.end(), { lowerband, upperband });

#ifdef DEBUG
	print_BB_data(lowerband, upperband, mean, std_deviation);
#endif // !DEBUG

	if (value > upperband) {
		return Action::SELL;
	}
	else if (value < lowerband) {
		return Action::BUY;
	}
	else {
		return Action::HOLD;
	}
}

Action Analyzer::set_rsi(
	const std::string& symbol, double price,
	std::deque<double>& cells, size_t period
) {
	int perc = 100;
	int sell_signal_perc = 70;
	int buy_signal_perc = 30;

	std::deque<double> values;
	std::deque<double> differences;
	matrix mat;
	mat = dataset.at(symbol);
	for (auto it = mat.end() - period; it != mat.end(); ++it) {
		auto&& row = *it;
		values.push_back(row[row.size() - 1]);
	}
	values.push_back(price);
	double prev = 0;
	for (auto it = values.begin(); it != values.end(); ++it) {
		double current = *it;
		if (it != values.begin()) {
			differences.push_back(current - prev);
		}
		prev = current;
	}
	double avg_up = calc->get_moving_average_abs(differences, true);
	double avg_down = calc->get_moving_average_abs(differences, false);
	double rel_strength = 0;
	if (avg_down != 0) { // avoid div by zero
		rel_strength = avg_up / avg_down;
	}
	double rsi = calc->get_rel_strength_index(perc, rel_strength);
	cells.push_back(rsi);

#ifdef DEBUG
	print_RSI_data(rsi, avg_up, avg_down);
#endif // !DEBUG

	if (rsi > sell_signal_perc) {
		return Action::SELL;
	}
	else if (rsi < buy_signal_perc) {
		return Action::BUY;
	}
	else {
		return Action::HOLD;
	}
}

void Analyzer::create_transaction(
	const std::string& symbol,  double exchange_rate,
	double crypto_amount, Action signal
) {
	std::shared_ptr<Transaction> transaction = create_shared<Transaction>(
		crypto_amount, exchange_rate, action_mapper.at(signal), symbol
	);
	if (transactions.size() >= max_transactions) {
		transactions.pop_front();
	}
	transactions.push_back(transaction);
	append_to_file(transaction);
}

void Analyzer::process_sell_signal(const std::string& symbol, double price) {
	print_signal(symbol, Action::SELL, price);
	double crypto_amount = assets.at(symbol);
	double value_in_dollars = crypto_amount * price;
	double value_with_trading_fee = value_in_dollars - value_in_dollars * trading_fee;
	create_transaction(symbol, price, crypto_amount, Action::SELL);
	assets[symbol] = 0;
	assets[us_dollar] += value_with_trading_fee;
	signal_counter_map[symbol] = 0;
}

void Analyzer::process_buy_signal(const std::string& symbol, double price) {
	print_signal(symbol, Action::BUY, price);
	double invested_value = assets.at(us_dollar)  / investment_split;
	double value_with_trading_fee = invested_value - invested_value * trading_fee;
	double crypto_amount = value_with_trading_fee / price;
	assets[us_dollar] -= invested_value;
	create_transaction(symbol, price, crypto_amount, Action::BUY);
	assets[symbol] += crypto_amount;
	signal_counter_map[symbol] = 0;
}

std::deque<double> Analyzer::set_technical_indicators(
	const std::string& symbol, double price
) {
	std::deque<double> row_cells{};

	size_t rsi_period = 13;
	size_t bb_period = 20;
	Action rsi_signal = set_rsi(symbol, price, row_cells, rsi_period);
	Action bb_signal = set_bollinger_bands(symbol, price, row_cells, bb_period);

#ifdef DEBUG
	print_suggestion("RSI", action_mapper[rsi_signal]);
	print_suggestion("BB", action_mapper[bb_signal]);
#endif // !DEBUG

	// NOTE: Conditions that at least one technical indicator triggers a signal
	// may not be optimal and the decision to keep it like that is due
	// to demostration purposes (to increase the probability that the program will
	// be more responsive, although it does not guarantee anything)
	// - in a proper application it would be better to keep it that both indicators
	// need to trigger a signal in order to count it as a proper signal 
	// to do something about a particular cryptocurrency
	if (bb_signal == Action::BUY || rsi_signal == Action::BUY) {
		++signal_counter_map[symbol];
		auto&& dollars = assets[us_dollar];
		if (dollars / investment_split > 1
			&& signal_counter_map.at(symbol) >= signal_threshold) {
			process_buy_signal(symbol, price);
		}
		else if (dollars / investment_split <= 1
			&& signal_counter_map.at(symbol) >= signal_threshold) {
			print_insufficient_funds(symbol, price);
		}
		else {
#ifdef DEBUG
			// Useful to get prepared for the signal streak, 
			// nevertheless, it is not desirable in the released application
			print_debug_trigger_signal(action_mapper.at(Action::SELL), signal_counter_map.at(symbol));
#endif // !DEBUG
		}
	}
	else if (bb_signal == Action::SELL || rsi_signal == Action::SELL) {
		++signal_counter_map[symbol];
		auto&& crypto_amount = assets.at(symbol);
		if (crypto_amount > 0
			&& signal_counter_map.at(symbol) >= signal_threshold) {
			process_sell_signal(symbol, price);
		}
		else if (crypto_amount ==  0 && signal_counter_map.at(symbol) >= signal_threshold) {
			print_cant_sell(symbol, price);
		}
		else { 
#ifdef DEBUG
			print_debug_trigger_signal(action_mapper.at(Action::SELL), signal_counter_map.at(symbol));
#endif // !DEBUG
		}
	}
	else {
		signal_counter_map[symbol] = 0;
	}
	row_cells.push_back(price);
	last_records[symbol] = row_cells;
	return row_cells;
}
#endif // !TECHNICAL_INDICATORS

#ifndef OUTPUT_FILE_HANDLING

void delete_dir_content(const std::string& out_dir) {
	fs::path dir_path = out_dir;
	for (auto& path : fs::directory_iterator(dir_path)) {
		fs::remove_all(path);
	}
}

void Analyzer::prepare_output_file() {
	if (!fs::exists(out_dir)) {
		fs::create_directory(out_dir);
	}
	else {
		delete_dir_content(out_dir);
	}
	write_header();
}

void Analyzer::write_header() const {
	try {
		std::ofstream file;
		const std::string& filename = get_filename();
		file.open(filename, std::ios::app);
		file << "Time,Name,Amount,Exchange Rate\n";
		file.close();
	}
	catch (std::exception& exc) {
		print(exc.what(), "\n");
	}
}

std::string get_csv_row(const std::shared_ptr<Transaction>& value) {
#ifdef __cpp_lib_format
	return std::format(
		"{},{},{},{}\n",
		value->get_datetime(),
		value->get_name(),
		value->get_amount(),
		value->get_xrate()
	);
#else // as long as no std::format is supported
	std::ostringstream os;
	os << value->get_datetime() << ','
		<< value->get_name() << ','
		<< value->get_amount() << ','
		<< value->get_xrate() << '\n';
	return os.str();
#endif
}

inline std::string Analyzer::get_filename() const {
#ifdef __cpp_lib_format
	return std::format("{}/{}{}", out_dir, out_fname, extension);
#else // as long as no std::format is supported
	std::ostringstream os;
	os << out_dir << "/" << out_fname << extension;
	return os.str();
#endif
}

void Analyzer::append_to_file(
	const std::shared_ptr<Transaction>& transaction
) {
	try {
		std::ofstream file;
		const std::string& filename = get_filename();
		file.open(filename, std::ios::app);
		file << get_csv_row(transaction);
		file.close();
	}
	catch(std::exception& exc) {
		print(exc.what(), "\n");
	}
}


#endif // !OUTPUT_FILE_HANDLING

#ifndef DATA_HANDLING 

void Analyzer::prepare_values_from_file(const std::vector<std::string>& symbols) {
	char csv_delimiter = ',';
	for (auto&& symbol : symbols) {
		std::ifstream reader;
		try {
			reader.open(symbol + extension);
		}
		catch (std::exception& exc) {
			print(exc.what());
			continue;
		}
		std::string line;
		// skip the header
		std::getline(reader, line);
		std::deque<double> row;

		while (true) {
			std::getline(reader, line);
			if (line.empty()) {
				break;
			}
			else {
				std::string part;
				std::stringstream ss(line);
				while (ss.good()) {
					std::getline(ss, part, csv_delimiter);
					if (part.empty()) { // first few records have incomplete records
						break;
					}
					double val = convert_string_to<double>(part);
					row.push_back(val);
				}
				dataset[symbol].push_back(std::move(row));
				row = {};
			}
		}
		// initialize issued pairs
		assets[symbol] = 0;
	}
}

void Analyzer::prepare_single(const std::pair<std::string, std::deque<double>>& row) {
	auto&& [symbol, prev_close_prices] = get_structured_bindings(row);
	size_t rsi_period = 13;
	size_t bb_period = 20;
	size_t iteration = 0;

	for (auto&& price : prev_close_prices) {
		std::deque<double> cells;
		//Relative Strength Index (RSI)
		if (iteration > rsi_period) {
			set_rsi(symbol, price, cells, rsi_period);
		}
		else {
			cells.push_back(0);
		}
		//Bollinger Bands (BB)
		if (iteration > bb_period) {
			set_bollinger_bands(symbol, price, cells, bb_period);
		}
		else {
			cells.push_back(0); cells.push_back(0);
		}
		// we do not need to hold the full dataset in the memory 
		// - only a few last records are needed
		// - the reason why deque is used
		// ->we need pop front for each added record(after we fill the deque with a few records)
		if (dataset[symbol].size() > bb_period) {
			dataset[symbol].pop_front();
		}
		//add latest closing price
		cells.push_back(price);
		dataset[symbol].push_back(cells);
		++iteration;
		// create record
		assets[symbol] = 0;
		signal_counter_map[symbol] = 0;
	}
}

void Analyzer::remove(const std::string& symbol) {
	// force sell - if there is anything to sell
	if (assets.at(symbol) > 0) {
		double last_price = last_records.at(symbol).at(last_records.size() - 1);
		process_sell_signal(symbol, last_price);
	}
	dataset.erase(symbol);
	assets.erase(symbol);
	signal_counter_map.erase(symbol);
	last_records.erase(symbol);
}

void Analyzer::prepare(const std::unordered_map<std::string, std::deque<double>>& data) {
	for (auto&& [key, values] : data) {
		prepare_single({key, values});
	}
}

#endif // !DATA_HANDLING