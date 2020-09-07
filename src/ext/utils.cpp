#include "utils.h"


const int& min(const int& a, const int& b) {
    return (b < a) ? b : a;
}
const int& max(const int& a, const int& b) {
    return (a < b) ? b : a;
}

std::vector<int> range(int start, int stop, int step) {
    int s = int((stop - start) / step);
    std::vector<int> v(s);
    std::generate(v.begin(), v.end(), [n = start-step, step] () mutable { n += step; return n; });
    return v;
};

bool file_exists(const std::string& file_path) {
    if (FILE *file = fopen(file_path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}
