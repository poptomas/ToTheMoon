#pragma once
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <memory>
#include <random>
#include <utility>

// hopefully not for long
// https://en.cppreference.com/w/cpp/compiler_support
// https://en.cppreference.com/w/cpp/feature_test
#ifdef __cpp_lib_format 
    #include <format>
#endif

/**
 * Utilities which were collected during the exploration 
 * of the C++ language and were suited to be kept
 * for a further use in a separate file
 */

#ifndef PRINT_UTILITIES

/**
 * @brief Thread safe printing
 * @note A standard option could have been taken into consideration:
 * https://en.cppreference.com/w/cpp/io/basic_osyncstream
 */
class ThreadSafeCout : public std::ostringstream {

public:
    ThreadSafeCout() {}
    ThreadSafeCout(const ThreadSafeCout&) = delete;
    ThreadSafeCout(ThreadSafeCout&&) = delete;
    ThreadSafeCout& operator=(const ThreadSafeCout&) = delete;
    ~ThreadSafeCout() {
        std::mutex mutex;
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << this->str();
    }
};

template <typename T>
inline void single_print(const T& argument) {
    ThreadSafeCout{} << argument;
}

void print() { }

template <typename T, typename...Ts>
inline void print(T&& first, Ts&&... rest) {
    single_print(first);
    print(rest...);
}

template <typename T>
inline void print_vector(const std::vector<T>& vect, char delimiter) {
    for (auto&& elem : vect)
        print(elem, delimiter);
}
#endif // !PRINT_UTILITIES

#ifndef CHAR_UTILITIES

inline bool not_digit(char ch) {
    return ('0' <= ch && ch <= '9') || ch == '.';
}

#endif // !CHAR_UTILITIES

#ifndef STRING_UTILITIES

inline static bool is_prefix(const std::string& prefix, const std::string& input) {
    return input.rfind(prefix, 0) == 0;
}

inline static void to_uppercase(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline static void to_lowercase(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

inline static void erase_from(std::string& str, char character) {
    str.erase(std::remove(str.begin(), str.end(), character), str.end());
}

inline static void front_trim(std::string& input_str) {
    input_str.erase(
        input_str.begin(), std::find_if(
            input_str.begin(), input_str.end(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            }
        )
   );
}

inline static void back_trim(std::string& input_str) {
    input_str.erase(
        std::find_if(input_str.rbegin(), input_str.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            })
       .base(), input_str.end()
    );
}

static inline void trim(std::string& input_str) {
    front_trim(input_str);
    back_trim(input_str);
}

template<typename T>
inline T convert_string_to(const std::string& str) {
    T converted {};
    std::istringstream s_stream(str);
    if (!(s_stream >> converted))
        throw std::invalid_argument("Can't be converted.");
    return converted;
}

template<typename T>
inline std::string convert_to_string(const T& arg) {
    std::ostringstream o_stream;
    if (!(o_stream << arg))
        throw std::invalid_argument("Can't be converted.");
    return o_stream.str();
}

inline std::string remove_non_digits(const std::string& str) {
    std::string builder;
    for (auto&& chr : str) {
        if (isalpha(chr)) {
            builder += chr;
        }
    }
    return builder;
}

inline std::vector<std::string> tokenize(const std::string& str, char delimiter) {
    std::vector<std::string>tokens;
    std::stringstream ss(str);
    std::string part;
    while (std::getline(ss, part, delimiter)) {
        if (!part.empty()) {
            tokens.push_back(part);
        }
    }
    return tokens;
}

inline bool is_contained_once(const std::string& substr, const std::string& mainstr) {
    size_t occurrences = 0;
    size_t pos = 0;
    while ((pos = mainstr.find(substr, pos)) != std::string::npos) {
        ++occurrences;
        pos += substr.length();
    }
    return occurrences == 1;
}

#endif // !STRING_UTILITIES

#ifndef VECTOR_UTILITIES

inline std::string stringify(const std::vector<std::string>& vect) {
    std::stringstream builder;
    for (auto it = vect.begin(); it != vect.end(); ++it) {
        builder << *it;
        if (it != vect.end() - 1) {
            builder << " ";
        }
    }
    return builder.str();
}

template<typename T> 
inline std::vector<T> linearize(const std::vector<std::vector<T>>& matrix) {
    std::vector<T> vec;
    for (auto&& vect : matrix) {
        for (auto&& value : vect) {
            vec.push_back(value);
        }
    }
    return vec;
}
#endif // !VECTOR_UTILITIES

#ifndef RANDOM_UTILITIES

unsigned int get_random() {
    static unsigned int seed = 0;
    typedef std::mt19937 rng_type;
    rng_type rng;
    std::uniform_int_distribution<rng_type::result_type> udist(0, 100);
    rng_type::result_type seedval = seed;
    rng.seed(seedval);
    rng_type::result_type random_number = udist(rng);
    ++seed;
    return random_number;
}

#endif // !RANDOM_UTILITIES

#ifndef TIME_UTILITIES

using ms = std::chrono::milliseconds;
using high_clock = std::chrono::high_resolution_clock;
using sys_clock = std::chrono::system_clock;
using time_var = high_clock::time_point;

/**
 * @brief A helper wrapper function to measure elapsed time of an input function
 * @param func - Input function to be measured
 * @param args - Variable number of input function arguments
 * @returns Elapsed time in milliseconds
 */
template<typename F, typename... Args>
long long measure_time(F func, Args&&... args) {
    time_var current_time = high_clock::now();
    func(std::forward<Args>(args)...);
    return std::chrono::duration_cast<ms>(high_clock::now() - current_time).count();
}

/**
 * @brief Get current datetime in dd-mm-yyyy hh:MM:ss format.
 * - if std::format is supported - time format is set via this header
 * - otherwise a string buffer is created and std::localtime is utilized
 * @see https://stackoverflow.com/a/52884698/14262598 - non std::format version
 */
inline std::string get_current_datetime() {
#ifdef __cpp_lib_format
    std::ostringstream os;
    //auto&& now = std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now()}; 
    // - cleaner zone time solution, nevertheless, it is not yet supported by g++-11
    auto&& now = std::chrono::system_clock::now() + std::chrono::hours(1);
    os << std::format("{:%d-%m-%Y %H:%M:%OS}", now) << '\n';
    return os.str();
#else
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string str_buffer;
    str_buffer.resize(30);
    std::strftime(&str_buffer.front(), str_buffer.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return str_buffer;
#endif
}

#endif // !TIME_UTILITIES

#ifndef MISC_UTILITIES

template <typename T, typename U>
inline const std::pair<T, U>& get_structured_bindings(const std::pair<T, U>& pair) {
    return pair;
}

/**
 * @brief An alternative how to cope with disabling of direct instance creation
 * - it is supposed to be befriended
 * @see: https://stackoverflow.com/a/17135547/14262598 - template version
 */
template <typename T, typename ...Args>
inline std::shared_ptr<T> create_shared(Args&& ...args) {
    return std::shared_ptr<T>(
        new T(std::forward<Args>(args)...)
    );
}

#endif // !MISC_UTILITIES
