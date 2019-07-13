#ifndef GADMEXTRACT_HH
#define GADMEXTRACT_HH 

struct MCoordinates {
    std::vector<double> longitude;
    std::vector<double> latitude;
};

struct RegionObject {
    std::string name;
    std::vector<double> lat;
    std::vector<double> lon;
    std::vector<double> x;
    std::vector<double> y;
    MCoordinates *outerboundary;
    std::vector<MCoordinates *> innerboundaries;
};

struct MSimpleData {
    std::string name;
    std::string text;
};

struct MSchemaData {
    std::vector<MSimpleData> schemaData;
};

struct MExtendedData {
    MSchemaData schemaData;
};

struct MColor {
    std::string color;
};

struct MLineStyle {
    MColor color;
};

struct MStyle {
    MLineStyle lineStyle;
};

struct MLinearRing {
    MCoordinates coordinates;
};

struct MOuterBoundaryIs {
    MLinearRing linearRing;
};

struct MInnerBoundaryIs {
    MLinearRing linearRing;
};

struct MPolygon {
    MOuterBoundaryIs outerBoundary;
    std::vector<MInnerBoundaryIs> innerBoundaries;
};

struct MPlaceMark {
    MStyle style;
    MExtendedData extendedData;
    MPolygon polygon;
};

#endif 
