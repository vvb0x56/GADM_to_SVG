#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/base/vec3.h"
#include "kml/engine.h"

#include "gadmextract.hh"

using kmldom::ContainerPtr;
using kmldom::ElementPtr;
using kmldom::FeaturePtr;
using kmldom::GeometryPtr;
using kmldom::KmlPtr;
using kmldom::MultiGeometryPtr;
using kmldom::PolygonPtr;
using kmldom::OuterBoundaryIsPtr;
using kmldom::InnerBoundaryIsPtr;
using kmldom::LinearRingPtr;
using kmldom::CoordinatesPtr;
using kmldom::PlacemarkPtr;
using kmldom::ExtendedDataPtr;
using kmldom::SimpleDataPtr;
using kmldom::SchemaDataPtr;
using kmlbase::Vec3;
using kmlengine::KmzFile;
using kmlengine::KmzFilePtr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

//vector<RegionObject *> region_objects;
vector<RegionObject *> objects;

static vector<MPlaceMark *> mplacemark_list;

static const int SHORTNAME_LENGTH = 5;

//-------------------- KML PART -----------------

static const string NAME_1 = "NAME_1";
static const string NAME_2 = "NAME_2";
// we should get this value from cli arguments:
//static const string DESIRED_REGION = "Kalmyk";
//static string desired_region = "Kalmyk";
static string desired_region;

static const FeaturePtr getRootFeature(const ElementPtr& root);
static void handleFeature(const FeaturePtr& feature);
static void handleContainer(const ContainerPtr& container);
static void handlePlacemark(const PlacemarkPtr& placemark);
static void handleDesiredPlacemark(const PlacemarkPtr& placemark, const string name); 
static void handleMultiGeometry(const MultiGeometryPtr& multigeometry, const string name); 
static void handlePolygon(const PolygonPtr& polygon, const string name);
static void handleOuterBoundaryIs(const OuterBoundaryIsPtr& outerboundaryis, RegionObject* region);
static void handleInnerBoundaryIs(const InnerBoundaryIsPtr& innerboundaryis, RegionObject* region);


static const FeaturePtr getRootFeature(const ElementPtr& root) {
    const KmlPtr kml = kmldom::AsKml(root); 
    if (kml && kml->has_feature()) 
        return kml->get_feature();

    return kmldom::AsFeature(root); 
}       

 // HERE WE ARE TRYING TO WALK THOUGH ALL THE FEATURE IN THE DOM
 // AND DIFF CONTAINER AND PLACEMARK
static void handleFeature(const FeaturePtr& feature) {

    if (feature) {

        if(const ContainerPtr container = kmldom::AsContainer(feature)) {
            handleContainer(container); 
        }
        else if (const PlacemarkPtr placemark = kmldom::AsPlacemark(feature)) {
            handlePlacemark(placemark);
        }
        else {
            std::cout << "Unknown feature type!!!\n";
        }

    }
}

static void handleContainer(const ContainerPtr& container) {
    
    for (int i = 0; i < container->get_feature_array_size(); i++) {

        handleFeature(container->get_feature_array_at(i));    
    }
}

static void handlePlacemark(const PlacemarkPtr& placemark) {

    if ( placemark->has_extendeddata() ) { // then we can found which region it is 
        
        ExtendedDataPtr extendeddata = placemark->get_extendeddata();


        for (int scdi = 0; scdi < extendeddata->get_schemadata_array_size(); scdi++) {
            if (scdi == 0) {
                SchemaDataPtr schemadata = extendeddata->get_schemadata_array_at(scdi);
                
                bool desired_region_flag = false;
                string name_of_area = desired_region;
        
                
                SimpleDataPtr simpledata = nullptr;
                for (int sidi = 0; sidi < schemadata->get_simpledata_array_size(); sidi++) {
                    simpledata = schemadata->get_simpledata_array_at(sidi);
                    if (NAME_1.compare(simpledata->get_name()) == 0) {
                        if (desired_region.compare(simpledata->get_text()) == 0) {
                            desired_region_flag = true;
                        }
                    } 
                    else if (NAME_2.compare(simpledata->get_name()) == 0) {
                        // if there name of the area of level 2, then 
                        // we;ll use that name 
                        name_of_area = simpledata->get_text(); 
                    }
                }

                if (desired_region_flag) { // we found desired region, so let'w work with it 
                    handleDesiredPlacemark(placemark, name_of_area);
                }
            }
            else {
                cout << "Unkown extended data schemadata.\n";
            }
        }
    }
}

static void handleDesiredPlacemark(const PlacemarkPtr& placemark, const string name) {
    GeometryPtr geometry = placemark->get_geometry();
    if ( !geometry )
        return;

    switch (geometry->Type()) {
        case kmldom::Type_MultiGeometry:
            handleMultiGeometry(kmldom::AsMultiGeometry(geometry), name);
            break;
        default: 
            cout << "Geometry type is not MultiGeometry, skip\n";
            break;
    }
}



static void handleMultiGeometry(const MultiGeometryPtr& multigeometry, const string name) {

    for (int pi = 0; pi < multigeometry->get_geometry_array_size(); pi++) {
        PolygonPtr polygon = kmldom::AsPolygon(multigeometry->get_geometry_array_at(pi));
        handlePolygon(polygon, name);
    }
}

static void handlePolygon(const PolygonPtr& polygon, const string name) {
    
    RegionObject *regionObject = new RegionObject();
    regionObject->name = name;

    if (polygon->has_outerboundaryis()) {
        OuterBoundaryIsPtr outerboundaryis = polygon->get_outerboundaryis();
        handleOuterBoundaryIs(outerboundaryis, regionObject);
        for (int i = 0; i < polygon->get_innerboundaryis_array_size(); i++) {
            handleInnerBoundaryIs(polygon->get_innerboundaryis_array_at(i), regionObject);
        }
    }

    objects.push_back(regionObject);


}

static void handleOuterBoundaryIs(const OuterBoundaryIsPtr& outerboundaryis, RegionObject* regionObject) {
    if (outerboundaryis->has_linearring()) {
        LinearRingPtr linearring = outerboundaryis->get_linearring();
        if (linearring->has_coordinates()) {

            regionObject->outerboundary  = new MCoordinates();

            CoordinatesPtr coordinates = linearring->get_coordinates();
            for (int i = 0; i < coordinates->get_coordinates_array_size(); i++) {
                Vec3 coords = coordinates->get_coordinates_array_at(i);
                regionObject->outerboundary->longitude.push_back(coords.get_longitude());
                regionObject->outerboundary->latitude.push_back(coords.get_latitude());
            }
        }
    }
}

static void handleInnerBoundaryIs(const InnerBoundaryIsPtr& innerboundaryis, RegionObject* regionObject) {
    if (innerboundaryis->has_linearring()) {
        LinearRingPtr linearring = innerboundaryis->get_linearring();
        if (linearring->has_coordinates()) {
            CoordinatesPtr coordinates = linearring->get_coordinates();
            MCoordinates* inner = new MCoordinates();
            for (int i = 0; i < coordinates->get_coordinates_array_size(); i++) {
                Vec3 coords = coordinates->get_coordinates_array_at(i);
                inner->longitude.push_back(coords.get_longitude());
                inner->latitude.push_back(coords.get_latitude());
            }
            regionObject->innerboundaries.push_back(inner);
        }
    }

}

// ------------------- PRINT --------

static void print() {
    for (int i = 0; i < objects.size(); i++) {

        cout << ">name " << objects[i]->name << "\n";
        
        cout << ">>outer " << "OuterBoundary:\n";
        for (int lli = 0; lli < objects[i]->outerboundary->longitude.size(); lli++) {
            cout << objects[i]->outerboundary->longitude[lli] << " ";
            cout << objects[i]->outerboundary->latitude[lli] << "\n";
        }

        for (int ibi = 0; ibi < objects[i]->innerboundaries.size(); ibi++) {
            cout << ">>inner " << "InnerBoundary" << ibi << ":\n";
            for (int llj = 0; llj < objects[i]->innerboundaries[ibi]->longitude.size(); llj++) {
                cout << objects[i]->innerboundaries[ibi]->longitude[llj] << " "; 
                cout << objects[i]->innerboundaries[ibi]->latitude[llj] << "\n"; 
            }
        }
    }    
}

// ------------------ CLEAN ----------------------------------

static void clean() {

    for (int i = 0; i < objects.size(); i++) {
        delete(objects[i]->outerboundary);
        for (int j = 0; j < objects[i]->innerboundaries.size(); j++) {
            delete(objects[i]->innerboundaries[j]);
        }
    }    

}



int main(int argc, char **argv) {
    
    // handle arguments 

    if (argc != 3) { 
        cout << "Usage: ./app file region\n";
        return 1;
    }
    KmzFilePtr kmzfile(KmzFile::OpenFromFile(argv[1]));
    if (!kmzfile) { 
        cout << "error: " << argv[1] << " is not a valid kmz file\n";
        return 1;
    }

    string kmldata;
    if (!kmzfile->ReadKml(&kmldata)) {
        cout << "error: no data read from " << argv[1] << endl; 
        return 1;
    }
//    std::string kmldata;
//    if( !kmlbase::File::ReadFileToString(argv[1], &kmldata) ) {
//        cout << "Error while openining file: " << argv[1] << endl;   
//    };
//
    desired_region = string(argv[2]); // region which  we looking for 


    //PARSE DATA
    std::string errors;
    ElementPtr root  = kmldom::Parse(kmldata, &errors);
    if ( !root ) {
        cout << "Parsing errors: " <<  errors << endl;
        // if parsing error, i think we should exit for now 
        return 1;
    }

    //WORK WITH DOM
    FeaturePtr feature = getRootFeature(root);

    handleFeature(feature); 

    print();

    clean();

    return 0;
}
