#pragma once

/** Cpprest cross-platform pitfall */
#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/json.h>
#include <cpprest/http_client.h>

#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "utilities.h"
#include "crypto_token.h"
#include "transaction.h"
#include "analysis.h"
#include "mapping.h"

using JSON_value = web::json::value;
using JSON_value_t = web::json::value::value_type;
using namespace web::http;
using namespace web::http::client;

/**
 * Connection header
 * @brief contains various classes concerning API connection
 * - utilizes cpprest library for http requests and further json parsing
 * @see https://github.com/microsoft/cpprestsdk
 * - currently supports connection to Binance API
 * @see https://binance-docs.github.io/apidocs
 */

// Forward declarations are required here
class ApiConn;
class GenericConn;
class BinanceApiConn;
// and other

// these variables are left in the global scope
// since they are accessed from multiple endpoints
// - to keep one common variable across the *Conn classes

std::unordered_map<std::string, std::shared_ptr<CryptoToken>> crypto_actions;
std::unordered_map<std::string, double> cryptocurrency_pairs;
std::shared_ptr<Analyzer> analyzer;

/**
 * @brief Parent class of all API connectors 
 * -- connector to the cryptocurrencies analyzer
 */
class ApiConn {
public:
    ApiConn() {
        analyzer = create_shared<Analyzer>();
    }
    virtual ~ApiConn() { }

    /////////////////////////////////////////
    // Functions required for the initial run
    virtual void receive_current_data(bool) = 0;
    virtual void prepare_datasets(const std::vector<std::string>&) = 0;

    /**
     * @brief Checks user's entered input whether the symbol exists in the API
     * - if it does - new cryptocurrency token is created (pointer to it)
     * - otherwise an error message is shown in the console,
     * nevertheless, the program continues
     * @returns Filtered (valid) input records
     */
    std::vector<std::string> filter_set_preferences(const std::vector<std::string>&);

    //////////////////////////////////////////
    // Commmands handler
    inline void show_result() const;
    inline void show_current_state() const;
    inline void show_current_values() const;
    inline void show_transactions() const;
    inline void show_indicators() const;
    /**
     * @brief the analyzer to increase amount of money
     * if the value is valid, otherwise an error message is 
     * shown in the console
     */
    inline void deposit(double);

    //////////////////////////////////////////
    // debugging purposes
    inline void print_concrete(const std::string&) const;
    inline void print_all() const;

    //TODO: doc
    void add_new_crypto_token(const std::string&);
    inline bool is_valid_input(const std::string&) const;
};

/**
 * @brief A general purpose API handler
 * -- an entrypoint from the outer code
 */
class GenericConn final : public ApiConn {
public:
    ~GenericConn() { }
    GenericConn(const std::shared_ptr<BinanceApiConn>& binance) : mem_binance(binance) { }

    /**
     * @brief Transfers the responsibility to the concerned connector
     */
    virtual void prepare_datasets(const std::vector<std::string>& fnames) override;

    /**
     * @brief Transfers the responsibility to the concerned connector
     * @param shall_add - Whether the received data should be included in the dataset
     * - only once upon a time it happens
     */
    virtual void receive_current_data(bool shall_add) override;

    /**
     * @brief Checks whether the symbol is correct according to 
     * specified conditions
     * - if yes - it transfers a request for a symbol to be added to the watchlist 
     * @param symbol - cryptocurrency
     * @returns boolean value whether the action was accomplished
     */
    bool try_add_cryptocurrency(const std::string& symbol);

    /**
     * @brief Looks in the user's watchlist and if it exists,
     * it transfers a request for a symbol to be removed from the watchlist
     * @returns boolean value whether the action was accomplished
     */
    bool try_remove_cryptocurrency(const std::string&);
private:
    GenericConn() { }
    std::shared_ptr<BinanceApiConn> mem_binance;
};

/**
 * @brief Handler of the connection to Binance API
 */
class BinanceApiConn final : public ApiConn {
public:
    ~BinanceApiConn() { }

    /**
     * @brief 
     * - cpprest makes an http request to the Binance API to receive
     * json data (an array containing key value pairs of price and a symbol)
     * Example: https://api.binance.com/api/v3/ticker/price
     * - json is processed and saved to local memory as a map
     */
    virtual void receive_current_data(bool) override;

    /**
     * @brief Makes an http request to Binance API via cpprest,
     * processes received json data from the API
     * and makes a request to save the dataset
     */
    virtual void prepare_datasets(const std::vector<std::string>&) override;

    /**
     * @brief Initial dataset preparation is figured out via
     * previously called Python script - only calls the analyzer
     * to build the dataset by itself
     */
    void prepare_datasets_gold_data(const std::vector<std::string>&);
   
private: // methods
    BinanceApiConn() : url("https://api.binance.com") {}

    template<typename T, typename ...Args>
    friend std::shared_ptr<T> create_shared(Args&& ...args);

    /**
     * @brief Obscure cpprest based JSON values to a map
     * consisting of keys as symbol, values as prices
     */
    void save_json_data(const JSON_value&);

    /**
     * @brief Stores the data received from the API to a map
     * which is further transfered to the analyzer
     * @see https://binance-docs.github.io/apidocs/spot/en/#kline-candlestick-data
     */
    void save_dataset(const JSON_value&, const std::string&);
private: // fields
    std::string url;
};

#ifndef PRINT_FUNCTIONS

inline static void print_unavailable(const std::string& symbol) {
    print("\"", symbol, "\" unavailable\n");
}

inline static void print_total(double final_balance) {
    print("You ended up with ", final_balance, " USD\n");
}

inline void ApiConn::show_transactions() const {
    analyzer->print_transactions();
}

inline void ApiConn::show_indicators() const {
    analyzer->print_indicators();
}

inline void ApiConn::show_current_state() const {
    analyzer->print_current(crypto_actions);
}

inline void ApiConn::show_result() const {
    double final_balance = analyzer->withdraw(crypto_actions);
    print_total(final_balance);
}

inline void ApiConn::show_current_values() const {
    for (auto&& [key, val] : crypto_actions) {
        double current_value = cryptocurrency_pairs[key];
        print("[", key, ": ", current_value, " USD]\n");
    }
}

inline void ApiConn::print_all() const {
    for (auto&& [key, val] : cryptocurrency_pairs) {
        print(key, " : ", val, "\n");
    }
}

inline void ApiConn::print_concrete(const std::string& symbol) const {
    auto it = cryptocurrency_pairs.find(symbol);
    if (it != cryptocurrency_pairs.end())
        print(it->first, " : ", it->second, "\n");
}
#endif // !PRINT_FUNCTIONS

#ifndef GENERICCONN_DEFINITIONS
inline void GenericConn::receive_current_data(bool add_to_ds) {
    mem_binance->receive_current_data(add_to_ds);
}

bool GenericConn::try_remove_cryptocurrency(const std::string& symbol) {
    bool is_valid_op = crypto_actions.find(symbol) != crypto_actions.end();
    if (is_valid_op) {
        crypto_actions.erase(symbol);
        analyzer->remove(symbol);
    }
    return is_valid_op;
}

bool GenericConn::try_add_cryptocurrency(const std::string& symbol) {
    bool is_valid_op = is_valid_input(symbol)
        && crypto_actions.find(symbol) == crypto_actions.end(); // not yet included in a watchlist
    if (is_valid_op) {
        add_new_crypto_token(symbol);
        mem_binance->prepare_datasets({ symbol });
    }
    return is_valid_op;
}

void GenericConn::prepare_datasets(const std::vector<std::string>& fnames) {
    // NOTE: for other shared pointers => other services than Binance
    // - at least prepare_datasets needs to be implemented
    // - gold data are in this library as an utility to check whether underlying mathematics
    // of the technical indicators is correct
    // - it does not have to be implemented but for the further development of
    // other API endpoints it is highly recommended  since it is rather error prone
#ifdef GOLD_DATA
    mem_binance->prepare_datasets_gold_data(fnames);
#else
    mem_binance->prepare_datasets(fnames);
#endif
}

#endif // !GENERICCONN_DEFINITIONS

#ifndef APICONN_DEFINITIONS
inline bool ApiConn::is_valid_input(const std::string& crypto_pair) const {
    return cryptocurrency_pairs.find(crypto_pair) != cryptocurrency_pairs.end()
        && is_contained_once("USD", crypto_pair);
    // we want to support only cryptocurrency<->us dollar direction to easily determine
    // buy-sell relationship
    // NOTE: a classical str.contains(other) (or the prolonged variant with find)
    // may have sufficed, nevertheless, it is better not to allow obscurities
    // such as USDT to USDC which is a completely valid conversion too
}

void ApiConn::add_new_crypto_token(const std::string& cryptocurrency) {
    auto&& ptr = create_shared<CryptoToken>();
    Action default_action = Action::DEFAULT;
    ptr->set_state(default_action);
    ptr->set_value(cryptocurrency_pairs[cryptocurrency]);
    crypto_actions[cryptocurrency] = std::move(ptr);
    //show_current_values();
}

std::vector<std::string> ApiConn::filter_set_preferences(const std::vector<std::string>& input) {
    std::vector<std::string> values;
    for (auto&& cryptocurrency : input) {
        if (is_valid_input(cryptocurrency)) {
            add_new_crypto_token(cryptocurrency);
            values.push_back(cryptocurrency);
        }
        else {
            print_unavailable(cryptocurrency);
        }
    }
    return values;
}

inline void ApiConn::deposit(double value) {
    analyzer->deposit(value);
}

#endif // !APICONN_DEFINITIONS

#ifndef BINANCE_DEFINITIONS

#ifndef BINANCE_API_SPECIFIC_FUNCTIONS
inline void quotes_trim(std::string& str) {
    str = str.substr(1, str.size() - 2);
}

/**
 * @brief Cleans up received data (price and symbol)
 * cpprest specific format serialization to std::string
 * @param object - a record from the json array 
 * @param key - symbol or price desired from the object above
 * @returns cleaned up string of demanded data
 */
std::string api_specific_object_conversion(
    const web::json::object& object, const std::string& key
) {
    auto&& util_key = utility::conversions::to_string_t(key);
    auto&& serialized = object.at(util_key).serialize();
    std::string str = utility::conversions::to_utf8string(serialized);
    quotes_trim(str);
    return str;
}

/**
 * @brief Gets price from an obscure cpprest-specific format and converts
 * it a standard c++ double value
 * @param arr - an array in a specified format on the link
 * @see https://binance-docs.github.io/apidocs/spot/en/#kline-candlestick-data
 * @param index - demanded index from the array above
 */
double api_specific_array_conversion(const web::json::array& arr, size_t index) {
    auto&& util_price = arr.at(index).as_string();
    auto str_price = utility::conversions::to_utf8string(util_price);
    double value = convert_string_to<double>(str_price);
    return value;
}
#endif // !BINANCE_API_SPECIFIC_FUNCTIONS

void BinanceApiConn::prepare_datasets_gold_data(const std::vector<std::string>& fnames) {
    analyzer->prepare_values_from_file(fnames);
}

void BinanceApiConn::prepare_datasets(const std::vector<std::string>& fnames) {
    auto util_url = utility::conversions::to_string_t(url);
    http_client client(util_url);
    for (auto&& name : fnames) {
        std::string address = ("/api/v3/klines?symbol=" + name + "&interval=1m");
        client.request(methods::GET, utility::conversions::to_string_t(address))
            .then([](const http_response& response) {
                if (response.status_code() == status_codes::OK) {
                    return response.extract_json();
                }
                else {
                    print("Can't connect right now: ",
                        convert_to_string(response.status_code()), "\n"
                    );
                    return pplx::task_from_result(JSON_value());
                }
            })
            .then([this, name](const JSON_value& json) {
                save_dataset(json, name);
                return json;
            })
            .wait();
    }
}

void BinanceApiConn::save_dataset(const JSON_value& data, const std::string& symbol) {
    std::unordered_map<std::string, std::deque<double>> values;
    auto&& json_arr = data.as_array();
    // According to https://binance-docs.github.io/apidocs/spot/en/#kline-candlestick-data
    size_t close_index = 4;
    for (auto it = json_arr.begin(); it != json_arr.end(); ++it) {
        auto&& array_v = it->as_array();
        double price = api_specific_array_conversion(array_v, close_index);
        values[symbol].push_back(price);
    }
    analyzer->prepare(values);
}

void BinanceApiConn::receive_current_data(bool add_to_ds) {
    auto util_url = utility::conversions::to_string_t(url);
    http_client client(util_url);
    std::string address = "/api/v3/ticker/price";
    client.request(methods::GET, utility::conversions::to_string_t(address))
        .then([](const http_response& response) {
            if (response.status_code() == status_codes::OK) {
                return response.extract_json();
            }
            else {
                print("Can't connect right now: ", std::to_string(response.status_code()), "\n");
                return pplx::task_from_result(JSON_value());
            }
        })
        .then([this](const JSON_value& json) {
            save_json_data(json);
            return json;
        })
        .wait();        
        analyzer->get_analysis(crypto_actions, add_to_ds);
}

void BinanceApiConn::save_json_data(const JSON_value& data) {
    auto&& json_arr = data.as_array();
    for (auto it = json_arr.begin(); it != json_arr.end(); ++it) {
        auto&& object = it->as_object();
        const std::string& symbol = api_specific_object_conversion(object, "symbol");
        const std::string& price = api_specific_object_conversion(object, "price");
        double converted_price = convert_string_to<double>(price);
        cryptocurrency_pairs[symbol] = converted_price;
        if (crypto_actions.find(symbol) != crypto_actions.end()) {
            crypto_actions[symbol]->set_value(converted_price);
        }
    }
}
#endif // !BINANCE_DEFINITIONS
