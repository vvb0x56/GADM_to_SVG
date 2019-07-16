#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <locale>
#include <algorithm>
#include <cctype>
#include <cstdlib> // for strtod


#include "common.hh"

const int REGIONS_NUMBER = 14;

const MCapital regions[REGIONS_NUMBER]  = { { "Elista", 44.2701870, 46.3069990 },
                                            { "Troitskoye", 44.2607040, 46.4110680 },
                                            { "Ketchenery", 44.5283160, 47.3043710 },
                                            { "Sadovoe", 44.5046142, 47.7707657 },
                                            { "Malyye Derbety", 44.6674977, 47.9478031 },
                                            { "Priyutnoye", 43.5124320, 46.1006890 },
                                            { "Yashalta", 42.2726097, 46.3385265 },
                                            { "Gorodovikovsk", 41.9342690, 46.0828510 },
                                            { "Yashkul", 45.3434640, 46.1741680 },
                                            { "Komsomolskiy", 46.0429800, 45.3296700 },
                                            { "Lagan", 47.3612820, 45.3920590 },
                                            { "Iki-Burul", 44.6470030, 45.8229980 },
                                            { "Tsagan Aman", 46.724106, 47.565372 },
                                            { "Bolshoy Tsaryn", 45.428036, 47.850693 } };


double geo2pxf_lon(double lon, double origin_lon, double origin_lat) {
    return (lon - origin_lon) * 69.172 * 1760 * 3 * cos(origin_lat * atan(1) / 45);
}

double geo2pxf_lat(double lat, double origin_lat) {
    return (lat - origin_lat) * 69.172 * 1760 * 3;
}

struct Object {
    std::string name;
    
    MCoordinates *outerBoundary;
    std::vector<MCoordinates *> innerBoundaries;

    void Find_Max(double &lon, double &lat) {
        Find_Max_Coordinates(outerBoundary, lon, lat);

        for (int i = 0; i < innerBoundaries.size(); i++) {
            Find_Max_Coordinates(innerBoundaries[i], lon, lat);
        }       
    }

    void Find_Max_Coordinates(MCoordinates *coordinates, double &lon, double &lat) {
        for (int i = 0; i < coordinates->longitude.size(); i++) {
            if ( coordinates->longitude[i] > lon ) lon = coordinates->longitude[i];
            if ( coordinates->latitude[i]  > lat ) lat = coordinates->latitude[i];
        }
    }

    void Find_Min(double &lon, double &lat) {
        Find_Min_Coordinates(outerBoundary, lon, lat);

        for (int i = 0; i < innerBoundaries.size(); i++) {
            Find_Min_Coordinates(innerBoundaries[i], lon, lat);
        }       

    }

    void Find_Min_Coordinates(MCoordinates *coordinates, double &lon, double &lat) {
        for (int i = 0; i < coordinates->longitude.size(); i++) {
            if ( coordinates->longitude[i] < lon ) lon = coordinates->longitude[i];
            if ( coordinates->latitude[i]  < lat ) lat = coordinates->latitude[i];
        }
    }

    void TransformGeo2Pxf(double origin_lon, double origin_lat) {
        TransformGeo2Pxf_Outer(origin_lon, origin_lat);
        TransformGeo2Pxf_Inner(origin_lon, origin_lat);
    }

    void TransformGeo2Pxf_Coordinates(MCoordinates *coordinates, double origin_lon, double origin_lat) {
        for (int i = 0; i < coordinates->longitude.size(); i++) {

            double lon = coordinates->longitude[i];
            double lat = coordinates->latitude[i];

            coordinates->longitude[i] = geo2pxf_lon(lon, origin_lon, origin_lat);
           // (lon - origin_lon) * 69.172 * 1760 * 3 * cos(origin_lat * atan(1) / 45);
            coordinates->latitude[i] = geo2pxf_lat(lat, origin_lat);
            //(lat - origin_lat) * 69.172 * 1760 * 3; 
        }    

    }

    void TransformGeo2Pxf_Outer(double origin_lon, double origin_lat)  {
        TransformGeo2Pxf_Coordinates(outerBoundary, origin_lon, origin_lat);
    }

    void TransformGeo2Pxf_Inner(double origin_lon, double origin_lat) {
        for (int i = 0; i < innerBoundaries.size(); i++) {
            TransformGeo2Pxf_Coordinates(innerBoundaries[i], origin_lon, origin_lat);
        }
    }

    Object() {
        outerBoundary = new MCoordinates();
    }

    ~Object() {
        delete outerBoundary;

        for (int i = 0; i < innerBoundaries.size(); i++) {
            delete innerBoundaries[i];
        }
    }

    void AddToOU(double lon, double lat) {
        outerBoundary->longitude.push_back(lon);
        outerBoundary->latitude.push_back(lat);
    }

    void print() {
        std::cout << name << std::endl;
        printOU();
        printIN();
    }


    void printOU() {
        std::cout << "Outer:\n";
        for (int i = 0; i < outerBoundary->longitude.size(); i++) {
            std::cout << outerBoundary->longitude[i] << " ";
            std::cout << outerBoundary->latitude[i]  << "\n";
        }
    }

    void printIN() {
        for (int i = 0; i < innerBoundaries.size(); i++) {
            std::cout << "Inner:\n";
            for (int j = 0; j < innerBoundaries[i]->longitude.size(); j++) {
                std::cout << innerBoundaries[i]->longitude[j] << " ";
                std::cout << innerBoundaries[i]->latitude[j]  << "\n";
            }
        }
    }
};      


static void geo2pxf(std::vector<Object *>& objects);
static void HandleInput(std::string& val1, std::string& val2);
static void printObjects(std::vector<Object *> &objects);


static void printObject(Object* object) {
    using namespace std;

    cout << object->name << endl;
    cout << "outer:" << endl;
    for (int i = 0; i < object->outerBoundary->latitude.size(); i++) {
        cout << object->outerBoundary->longitude[i] << " ";
        cout << object->outerBoundary->latitude[i] << "\n";
    }
}

static void printObjects(std::vector<Object *> &objects) {

    for (int i = 0; i < objects.size(); i++) {
        objects[i]->print();
    }
}

static void transformGeo2PxfForObjects(std::vector<Object *> &objects, double origin_lon, double origin_lat) {
    for (int i = 0; i < objects.size(); i++) {
        objects[i]->TransformGeo2Pxf(origin_lon, origin_lat);
    }
}

static void findMaxForObjects(std::vector<Object *> &objects, double &max_lon, double &max_lat) {
    for (int i = 0; i < objects.size(); i++) {
        objects[i]->Find_Max(max_lon, max_lat);
    }
}

static void findMinForObjects(std::vector<Object *> &objects, double &min_lon, double &min_lat) {
    for (int i = 0; i < objects.size(); i++) {
        objects[i]->Find_Min(min_lon, min_lat);
    }
}

//static bool IsDigitOrDot(int c) {
//    return (c == '.' || isdigit(c)) ? true : false;
//}
//
//
//static bool IsNumeric(const std::string& str) {
//    return std::all_of(str.begin(), str.end(), IsDigitOrDot);
//}


static void HandleInput(std::string& val1, std::string& val2) {

    if (val1.compare(NAME_STR) == 0) {

    } else if (val1.compare(OUTER_BOUNDARY_STR) == 0) {

    } else if (val1.compare(INNER_BOUNDARY_STR) == 0) {

    } else { // at the end we expect coords

        
    }   
} 

// ---------------- Geo to Cartesian transformation -------------

//static const double LATITUDE_MIN = -90.0;
//static const double LATITUDE_MAX = 90.0;
//static const double LONGITUDE_MIN = -180.0;
//static const double LONGITUDE_MAX = 180.0;
//
//double lat_min = LATITUDE_MAX;
//double lon_min = LONGITUDE_MAX;
//double x_max = 0.0;
//double y_max = 0.0;


// TODO I SHOULD REWRITE THISE FUNCTIN CAUSE IT IS NOT 
// UNDERSTANDABLE WHICH CONSTANT WE ARE USING - 69.172 AND 1760

// X = (LON - LON_MIN) * 69.172 * 1760 * 3 * COS(LAT_MIN * ATAN(1) / 45); // Y = (LAT - LAT_MIN) * 69.172 * 1760 * 3;
//static void geo2pxf(std::vector<Object *>& objects) {
//    for (int i = 0; i < objects.size(); i++) {
//        for (int j = 0; j < objects[i]->latitude.size(); j++) { // LAT and LON sizes is equal
//            double lon = objects[i]->longitude[j];
//            double lat = objects[i]->latitude[j];
////            lon_min = 0.0;
////            lat_min = 0.0;
//            double x = (lon - lon_min) * 69.172 * 1760 * 3 * cos(lat_min * atan(1) / 45);
//            double y = (lat - lat_min) * 69.172 * 1760 * 3;
//
//            objects[i]->x.push_back(x);
//            objects[i]->y.push_back(y);
//        }    
//    }
//}

//////////////////      MAIN()
//
//
//

static double scaleCoordsToSvgSize(double lon, int svg_wh_size, double max_latlon_size) {
    return lon * (svg_wh_size / max_latlon_size);

}

static double scaleCoordsToSvgSizeQ(double lon, int svg_wh_size, double max_latlon_size) {
    return round(lon * (svg_wh_size / max_latlon_size));

}

static void make_svg_coords(    MCoordinates *coordinates,
                                int svg_width, 
                                int svg_height, 
                                double max_longitude,
                                double max_latitude,
                                double (*scale)(double, int, double)) { 
    
    for (int i = 0; i < coordinates->longitude.size(); i++) {
            std::cout << scale(coordinates->longitude[i], svg_width, max_longitude) << ",";
            std::cout << svg_height - scale(coordinates->latitude[i], svg_height, max_latitude);

            if (i != coordinates->longitude.size() - 1)
                std::cout << " ";
    }
} 
        
        
static void make_svg_polygons(  Object * object,
                                int svg_width, 
                                int svg_height, 
                                double max_longitude,
                                double max_latitude,
                                double (*scale)(double, int, double)) { 

    // OUTER BORDER
    std::string shortname = (object->name.length()  < SHORTNAME_LENGTH ?
                            object->name : object->name.substr(0, SHORTNAME_LENGTH));

    std::cout << "\t\t\t<polygon fill-rule=\"nonzero";
    std::cout << "\" class=\"" << shortname << "\" points=\"";

    make_svg_coords(object->outerBoundary, svg_width, svg_height, max_longitude, max_latitude, scale);

    std::cout << "\" />\n";

    // INNER BORDER
    for (int i = 0; i < object->innerBoundaries.size(); i++) {
        std::cout << "\t\t\t<polygon fill-rule=\"evenodd";
        std::cout << "\" class=\"" << shortname << "_inner\" points=\"";
        make_svg_coords(object->innerBoundaries[i], svg_width, svg_height, max_longitude, max_latitude, scale);
    }

    if (object->innerBoundaries.size() != 0)
        std::cout << "\" />\n";

}


static void print_html_header() {
    std::cout << "<!DOCTYPE html>\n";
    std::cout << "<html>\n";
    std::cout << "\t<head>\n";
    std::cout << "\t\t<style type=\"text/css\">\n";
    std::cout << "\t\t\tpolygon {\n";
    std::cout << "\t\t\t\tfill:#7d916a;\n";
    std::cout << "\t\t\t\tstroke: white;\n";
    std::cout << "\t\t\t\tstroke-width: 1;\n";
    std::cout << "\t\t\t\tstroke-dasharray: 2;\n";
    std::cout << "\t\t\t}\n";
    std::cout << "\t\t\tpolygon:hover {\n";
    std::cout << "\t\t\t\tfill:blue;\n";
    std::cout << "\t\t\t}\n";
    std::cout << "\t\t</style>\n";
    std::cout << "\t</head>\n";
    std::cout << "\t<body>\n";

}

static void print_svg_capitals( double svg_width, 
                                double svg_height,
                                double max_longitude,
                                double max_latitude,
                                double origin_lon,
                                double origin_lat) {


    for (int i = 0; i < REGIONS_NUMBER; i++) {
        double lon = geo2pxf_lon(regions[i].lon, origin_lon, origin_lat);
        double lat = geo2pxf_lat(regions[i].lat, origin_lat);
        
        std::cout << "\t\t\t<circle cx=\"" << scaleCoordsToSvgSize(lon, svg_width, max_longitude);
        std::cout << "\" cy=\"" << svg_height - scaleCoordsToSvgSize(lat, svg_height, max_latitude) << "\" ";
        std::cout << "r=\"" << round(svg_width / 500) << "\" />" << std::endl;
        
    }
}


static void print_svg(          std::vector<Object *> objects, 
                                int svg_width, 
                                int svg_height, 
                                double max_longitude,
                                double max_latitude,
                                double (*scale)(double, int, double),
                                double origin_lon,
                                double origin_lat ) { 

    std::cout << "\t\t<svg height=\"" << svg_height << "\" width=\"" << svg_width << "\">\n";

    // Make region polygons 
    for (int i = 0; i < objects.size(); i++) {
        make_svg_polygons(objects[i], svg_width, svg_height, max_longitude, max_latitude, scale);
    }

    // Make Capitals points
    print_svg_capitals(svg_width, svg_height, max_longitude,max_latitude, origin_lon, origin_lat);


    std::cout << "\t\t</svg>\n";
}

static void print_html_tail() {
    std::cout << "\t</body>\n";
    std::cout << "</html>\n";
}


static void print_html(         std::vector<Object *> objects, 
                                int svg_width, 
                                int svg_height, 
                                double max_longitude,
                                double max_latitude,
                                double origin_lon,
                                double origin_lat) { 

    print_html_header();

    print_svg(objects, svg_width, svg_height, max_longitude, max_latitude, scaleCoordsToSvgSize, origin_lon, origin_lat);
    //print_svg(objects, svg_width, svg_height, max_lon, max_lat, scaleCoordsToSvgSizeQ);

    print_html_tail();

}


int main(int argc, char** argv) {
    using namespace std;

    long int svg_width; // svg_height will calculated from ratio

    if (argc  == 2 && IsNumeric(argv[1])) {
        svg_width = strtol(argv[1], NULL, 10);
        
    } else  {
        svg_width = DEFAULT_SVG_WIDTH;
    }


    string input_str;
    vector<Object *> objects;

    string val1, val2;
    stringstream ss;

    Object *obj = nullptr;

    InputType boundaryType = NONE;
    bool newInnerBoundaryFlag = false; int currInnerBoundaryIndex = -1; // this will eq to 0, when we meet inner boundary


    // READ FROM STDIN TO ARRAYLIST STAGE
    while(!cin.eof()) {
        getline(cin, input_str);
            
        if (input_str.size() > 0) {
          //  cout << input_str << endl;
            ss.str(input_str);
            ss >> val1 >> val2; // we do expect string with 2 words in a line

            InputType it = GetInputType(val1, val2);
            switch (it) {
                case NAME:
                    //cout << "Name" << endl;
                    if (obj) { // Then it's a new object, and old we add to the list 
                        //obj->print();
                        objects.push_back(obj);
                        obj = nullptr;
                        newInnerBoundaryFlag = false;
                        currInnerBoundaryIndex = -1;
                    }
                    obj = new Object();
                    if (!obj) {
                        cout << "Error to allocate memory for the object\n";
                    }   
                    obj->name = val2;
                    break;
                case OUTER_BOUNDARY:
                    boundaryType = OUTER_BOUNDARY;
                    //cout << "Outer" << endl;
                    break;
                case INNER_BOUNDARY:
                    //cout << "Inner" << endl;
                    boundaryType = INNER_BOUNDARY;
                    newInnerBoundaryFlag = true;
                    break;
                case COORDINATES:
                    //cout << "Coords" << endl;

                    // Coords must to belong one of the boundary type
                    // we also expect that obj pointer is not null here
                    if (boundaryType == OUTER_BOUNDARY) {
                        if (obj) {
                            obj->outerBoundary->longitude.push_back(strtod(val1.c_str(), NULL));
                            obj->outerBoundary->latitude.push_back(strtod(val2.c_str(), NULL));
                        }
                    }
                    if (boundaryType == INNER_BOUNDARY) {
                        if (obj) {
                            if (newInnerBoundaryFlag) {
                                obj->innerBoundaries.push_back(new MCoordinates);
                                currInnerBoundaryIndex++;
                                newInnerBoundaryFlag = false;
                            }
                            //cout << "currInner index: " << currInnerBoundaryIndex << endl;
                            obj->innerBoundaries[currInnerBoundaryIndex]->longitude.push_back(strtod(val1.c_str(), NULL));
                            obj->innerBoundaries[currInnerBoundaryIndex]->latitude.push_back(strtod(val2.c_str(), NULL));
                        }
                    }

                    break;
                case NONE:
                    cout << "Unkown string format" << ss.str() << endl;
                    break;
                default:
                    cout << "Unkown string format" << ss.str() << endl;
            }
            ss.clear();
        }
    }

// at the end obj still in memory but saved in the list so: 
    if (obj != nullptr) {
        objects.push_back(obj);
    }

    if (objects.size() == 0) {
        return 1;
    }
    

    //Find minimums to set origin
    double min_lon = LONGITUDE_MAX;
    double min_lat = LATITUDE_MAX;

    findMinForObjects(objects, min_lon, min_lat);

    // CALC GEO TO PXF AND OUTPUT FROM STDOUT 
    
    transformGeo2PxfForObjects(objects, min_lon, min_lat);

    // GETTING MAX LON LAT TO SCALE SVG
    double max_lon = 0.0;
    double max_lat = 0.0;
    double ratio;
    double svg_height;

    findMaxForObjects(objects, max_lon, max_lat);

    ratio = max_lat / max_lon;
    svg_height = svg_width * ratio;


    //printObjects(objects);

    //cout << "svg width : " << svg_width << " svg_height : " << svg_height << endl;
    //cout << "max_lon : " << max_lon << " max_lat : " <<  max_lat << endl;

    
    print_html(objects, svg_width, svg_height, max_lon, max_lat, min_lon, min_lat);


    return 0;
}





