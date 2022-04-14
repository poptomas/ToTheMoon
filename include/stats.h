#pragma once

/**
 * @brief A class which encapsulates calculations
 * of basic formulae concerning statistics
 */
class StatsCalc {
public:
	// inline formulae
	inline double get_moving_average(const std::vector<double>&) const;
	inline double get_exp_moving_average(double, double, size_t) const;
	inline double get_rel_strength_index(double, double) const;

	/**
	 * @brief Calculates moving average on absolute values of input values.
	 */
	double get_moving_average_abs(const std::deque<double>&, bool) const;

	/**
	 * @brief Calculates standard deviation
	 */
	double get_standard_deviation(const std::vector<double>&, double) const;
private:
	StatsCalc() { }
};

inline double StatsCalc::get_rel_strength_index(
	double perc, double rel_strength
) const {
	return perc - (perc / (1 + rel_strength));
}

inline double StatsCalc::get_moving_average(
	const std::vector<double>& values
) const {
	if (values.empty()) {
		return 0;
	}
	return std::accumulate(values.begin(), values.end(), 0.0) / (double)values.size();
}

double StatsCalc::get_moving_average_abs(
	const std::deque<double>& values, bool is_positive
) const {
	std::vector<double> new_values;
	for (auto&& val : values) {
		if ((is_positive && val < 0) || (!is_positive && val > 0)) {
			new_values.push_back(0);
		}
		else if (val < 0) {
			new_values.push_back(-1.0 * val);
		}
		else {
			new_values.push_back(val);
		}
	}
	return get_moving_average(new_values);
}

double StatsCalc::get_standard_deviation(
	const std::vector<double>& values, double mean
) const {
	if (values.empty()) {
		return 0;
	}
	double variance = 0;
	for (auto&& v : values) {
		variance = variance + (v - mean) * (v - mean);
	}
	variance = sqrt(variance / (double)values.size());
	return variance;
}

inline double StatsCalc::get_exp_moving_average(
	double last_close, double last_EMA, size_t period
) const {

	double multiplier = 2.0 / (double)(period + 1);
	double result = last_close * multiplier + last_EMA * (1 - multiplier);
	return result;
}