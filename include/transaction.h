#pragma once

/**
 * Transaction class
 * @brief Used as a data class to hold data about accomplished
 * transcations
 * -- contains parametric constructor and variety of getters
 */
class Transaction {
public:
	~Transaction() { }

	double get_amount() const { return amount; }
	double get_xrate() const { return exchange_rate; }
	const std::string& get_name() const { return cryptopair; }
	const std::string& get_action() const { return action; }
	const std::string& get_datetime() const { return date_time; }

private:
	template<typename T, typename ...Args>
	friend std::shared_ptr<T> create_shared(Args&& ...args);

	Transaction(
		double in_amount,
		double in_xrate,
		const std::string& in_action,
		const std::string& in_cryptopair
	) : amount(in_amount), exchange_rate(in_xrate),
		action(in_action), cryptopair(in_cryptopair),
		date_time(get_current_datetime()) {}

	double amount;
	double exchange_rate;
	std::string action;
	std::string cryptopair;
	std::string date_time;
};