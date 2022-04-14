#pragma once

enum class Action { DEFAULT, BUY, SELL, HOLD };

/**
 * @brief used as a data class to hold information
 * about a particular cryptocurrency
 */
class CryptoToken {
public:
	void set_state(const Action& action);
	void set_value(double v);
	Action get_state() const;
	double get_value() const;
private:
	template<typename T, typename ...Args>
	friend std::shared_ptr<T> create_shared(Args&& ...args);

	CryptoToken()
		: state(Action::DEFAULT), value(0) {}
	Action state;
	double value;
};

double CryptoToken::get_value() const {
	return value;
}

Action CryptoToken::get_state() const {
	return state;
}

void CryptoToken::set_state(const Action& act) {
	state = act;
}

void CryptoToken::set_value(double val) {
	value = val;
}