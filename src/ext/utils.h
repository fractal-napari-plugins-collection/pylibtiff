#ifndef __UTILS_H__
#define __UTILS_H__

#include <math.h>
#include <fstream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;


#ifdef DEBUG
#define DEBUG_PRINTF(fmt, args...) printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args);
#else
#define DEBUG_PRINTF(fmt, args...)
#endif


/**
 * Pack a py::array using the C order.
 * @note This function guaranties a dense array using the C (row-major) ordering.
 * @tparam T Data type of an array component.
 * @param py_array Numpy array.
 * @return Packed Numpy array
 */
template<typename T>
py::array_t<T, py::array::c_style | py::array::forcecast> make_c_style(py::array_t<T> py_array) {
    return py_array;
}

/**
 * Computes the minimum of two integer values.
 * @param a First integer value.
 * @param b Second integer value.
 * @return The minimum value
 */
const int& min(const int& a, const int& b);
/**
 * Computes the maximum of two integer values.
 * @param a First integer value.
 * @param b Second integer value.
 * @return The maximum value
 */
const int& max(const int& a, const int& b);

/**
 * Computes a sequence of integer values.
 * @param start Start value (incl).
 * @param stop Stop value (excl).
 * @param step Step size.
 * @return The integer sequence
 */
std::vector<int> range(int start, int stop, int step=1);

/**
 * Checks wherever a file exists on the file system.
 * @param file_path Path to the file.
 * @return True, if the file exists. Otherwise, false.
 */
bool file_exists(const std::string& file_path);

#endif /* __UTILS_H__ */ 