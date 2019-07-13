#include <algorithm>
#include <vector>
#include "common.hh"


bool IsDigitOrDot(int c) {
    return (c == '.' || isdigit(c)) ? true : false;
}


bool IsNumeric(const std::string& str) {
    return std::all_of(str.begin(), str.end(), IsDigitOrDot);
}

InputType GetInputType(std::string& val1, std::string& val2) {
    if (val1.compare(NAME_STR) == 0) {
        return InputType::NAME;

    } else if (val1.compare(OUTER_BOUNDARY_STR) == 0) {
        return InputType::OUTER_BOUNDARY;

    } else if (val1.compare(INNER_BOUNDARY_STR) == 0) {
        return InputType::INNER_BOUNDARY;

    } else { // at the end we expect coords
        if (IsNumeric(val1) && IsNumeric(val2)) {
            return InputType::COORDINATES;
        }
    }   
    return InputType::NONE;
}
