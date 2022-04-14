#pragma once

/**
 * @brief Map initialization supporting class
 * to easen its use
 */
template<typename T>
class MapInitHelper {
public:
    T& mem_data;
    MapInitHelper(T& data) : mem_data(data) { }
    MapInitHelper(T&&) = delete;
    MapInitHelper(const T&) = delete;
    MapInitHelper& operator=(const MapInitHelper&) = delete;
    MapInitHelper& operator() (
        const typename T::key_type& key,
        const typename T::mapped_type&  value
    ) {
        mem_data[key] = value;
        return *this;
    }
};

template<typename T>
inline MapInitHelper<T> map_init(T& item) {
    return MapInitHelper<T>(item);
}
