#ifndef COMMON_HH
#define COMMON_HH

#include <vector>
#include <string>

bool IsNumeric(const std::string& str);
bool IsDigitOrDot(int c);

struct MCoordinates {
    std::vector<double> longitude;
    std::vector<double> latitude;
};

const std::string NAME_STR = ">name";
const std::string OUTER_BOUNDARY_STR = ">>outer";
const std::string INNER_BOUNDARY_STR = ">>inner";
const int DEFAULT_SVG_WIDTH = 800;
const int SHORTNAME_LENGTH = 5;
const double LONGITUDE_MAX = 180.0;
const double LATITUDE_MAX = 90.0;

struct MCapital {
    std::string name;
    double lon;
    double lat;
};

//Expected input type
enum InputType {
    NAME,
    OUTER_BOUNDARY,
    INNER_BOUNDARY,
    COORDINATES, 
    NONE
};

InputType GetInputType(std::string& val1, std::string& val2);

#endif
