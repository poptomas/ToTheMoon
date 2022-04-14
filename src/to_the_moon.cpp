
// NOTE: Uncomment only for a further contribution
// #define DEBUG

// NOTE: Uncomment only for a further contribution
//#define GOLD_DATA

#include "../include/to_the_moon.h"

#ifndef ENTRYPOINT_FUNCTIONS

void run_loop(
	Processor& in_processor, GenericConn& conn, const std::vector<std::string>& input
) {
	// delay needs to be set, otherwise the program's request spam
	// would result in a quick suspension by the API service provider
	auto delay = seconds(10);
	auto start = std::chrono::system_clock::now();
	bool add_to_dataset = false;
	bool is_initial_run = true;

	conn.prepare_datasets(input);
	in_processor.print_help_initial();
	auto controller = std::make_shared<ThreadController>();
	
	while (true) {
		std::atomic<bool> run(true);
		auto&& cin_func = std::bind(&Processor::read_cin, in_processor, std::ref(run), controller);
		std::thread cin_thread(cin_func);
		while (run.load()) {
			auto&& worker_func = std::bind(&GenericConn::receive_current_data, conn, add_to_dataset);
#ifdef DEBUG
			// to check whether 3rd party library
			// cpprest provides reasonably fast requests
			auto time = measure_time(worker_func);
			print_time_elapsed(time, delay);
			std::this_thread::sleep_for(delay); // do not use outside of debugging purposes - slow thread join
#else
			if (is_initial_run || controller->wait_for(delay)) {
				std::thread worker(worker_func);
				worker.join();
			}
#endif // !DEBUG
			add_to_dataset = false;
			auto current = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed = current - start;
			if (elapsed >= std::chrono::minutes(1)) {
				// in order to stay consistent with API provided
				// we shall update only once upon a time (not every call)
				add_to_dataset = true;
				start = current;
			}
			is_initial_run = false;
		}
		cin_thread.join();
		break;
	}
}

int main(int argc, char** argv) {
	std::shared_ptr<BinanceApiConn> binance = create_shared<BinanceApiConn>();
	//std::shared_ptr<CoinbaseApiConn> coinbase = create_shared<CoinbaseApiConn>();
	GenericConn conn(binance);
	Processor in_processor(conn);
	std::vector<std::string> input = in_processor.receive_user_input(argc, argv);
	// an initial api call is required in advance
	// in order to receive available cryptocurrency pairs of the provider given
	conn.receive_current_data(false);
	input = conn.filter_set_preferences(input);
	// it is expected to receive e.g. BTCUSDT ETHUSDT SOLUSDT ADAUSDT
	if (input.empty()) {
		print_empty_watchlist_warning();
	}

	// - an alternative option - to grab initial values
	//via python script provided in the data directory
#ifdef GOLD_DATA
	DataHandler d_handler;
	d_handler.download_initial_values(input);
#endif // !GOLD_DATA
	run_loop(in_processor, conn, input);
	print_end();
	return 0;
}

#endif // !ENTRYPOINT_FUNCTIONS
