#include "Inspector.h"

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <memory>

#include <QFile>
#include <QStringList>

#include <OsmAndCore/Common.h>
#include <OsmAndCore/Data/ObfReader.h>
#include <OsmAndCore/Utilities.h>

#include <OsmAndCore/Data/Model/MapObject.h>
#include <OsmAndCore/Data/Model/Street.h>
#include <OsmAndCore/Data/Model/StreetIntersection.h>
#include <OsmAndCore/Data/Model/StreetGroup.h>
#include <OsmAndCore/Data/Model/Building.h>

OsmAnd::Inspector::Configuration::Configuration()
    : bbox(90, -180, -90, 180)
{
    verboseAddress = false;
    verboseStreetGroups = false;
    verboseStreets = false;
    verboseBuildings = false;
    verboseIntersections = false;
    verboseMap = false;
    verboseMapObjects = false;
    verbosePoi = false;
    verboseAmenities = false;
    verboseTrasport = false;
    zoom = 15;
}

OsmAnd::Inspector::Configuration::Configuration(const QString& fileName)
    : bbox(90, -180, -90, 180)
{
    this->fileName = fileName;
    verboseAddress = false;
    verboseStreetGroups = false;
    verboseStreets = false;
    verboseBuildings = false;
    verboseIntersections = false;
    verboseMap = false;
    verbosePoi = false;
    verboseAmenities = false;
    verboseTrasport = false;
    zoom = 15;
}

// Forward declarations
#if defined(_UNICODE) || defined(UNICODE)
void dump(std::wostream &output, const QString& filePath, const OsmAnd::Inspector::Configuration& cfg);
void printAddressDetailedInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfAddressSection* section);
void printPOIDetailInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfPoiSection* section);
void printMapDetailInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfMapSection* section);
std::wstring formatBounds(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom);
std::wstring formatGeoBounds(double l, double r, double t, double b);
#else
void dump(std::ostream &output, const QString& filePath, const OsmAnd::Inspector::Configuration& cfg);
void printAddressDetailedInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfAddressSection* section);
void printPOIDetailInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfPoiSection* section);
void printMapDetailInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfMapSection* section);
std::string formatBounds(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom);
std::string formatGeoBounds(double l, double r, double t, double b);
#endif

OSMAND_CORE_UTILS_API void OSMAND_CORE_UTILS_CALL OsmAnd::Inspector::dumpToStdOut( const Configuration& cfg )
{
#if defined(_UNICODE) || defined(UNICODE)
    dump(std::wcout, cfg.fileName, cfg);
#else
    dump(std::cout, cfg.fileName, cfg);
#endif
}

OSMAND_CORE_UTILS_API QString OSMAND_CORE_UTILS_CALL OsmAnd::Inspector::dumpToString( const Configuration& cfg )
{
#if defined(_UNICODE) || defined(UNICODE)
    std::wostringstream output;
    dump(output, cfg.fileName, cfg);
    return QString::fromStdWString(output.str());
#else
    std::ostringstream output;
    dump(output, cfg.fileName, cfg);
    return QString::fromStdString(output.str());
#endif
}

OSMAND_CORE_UTILS_API bool OSMAND_CORE_UTILS_CALL OsmAnd::Inspector::parseCommandLineArguments( const QStringList& cmdLineArgs, Configuration& cfg, QString& error )
{
    for(auto itArg = cmdLineArgs.begin(); itArg != cmdLineArgs.end(); ++itArg)
    {
        auto arg = *itArg;
        if(arg == "-vaddress")
            cfg.verboseAddress = true;
        else if(arg == "-vstreets")
            cfg.verboseStreets = true;
        else if(arg == "-vstreetgroups")
            cfg.verboseStreetGroups = true;
        else if(arg == "-vbuildings")
            cfg.verboseBuildings = true;
        else if(arg == "-vintersections")
            cfg.verboseIntersections = true;
        else if(arg == "-vmap")
            cfg.verboseMap = true;
        else if(arg == "-vmapObjects")
            cfg.verboseMapObjects = true;
        else if(arg == "-vpoi")
            cfg.verbosePoi = true;
        else if(arg == "-vamenities")
            cfg.verboseAmenities = true;
        else if(arg == "-vtransport")
            cfg.verboseTrasport = true;
        else if(arg.startsWith("-zoom="))
            cfg.zoom = arg.mid(strlen("-zoom=")).toInt();
        else if(arg.startsWith("-bbox="))
        {
            auto values = arg.mid(strlen("-bbox=")).split(",");
            cfg.bbox.left = values[0].toDouble();
            cfg.bbox.top = values[1].toDouble();
            cfg.bbox.right = values[2].toDouble();
            cfg.bbox.bottom =  values[3].toDouble();
        }
        else if(arg.startsWith("-obf="))
        {
            cfg.fileName = arg.mid(strlen("-obf="));
        }
    }

    if(cfg.fileName.isEmpty())
    {
        error = "OBF file not defined";
        return false;
    }
    return true;
}

#if defined(_UNICODE) || defined(UNICODE)
void dump(std::wostream &output, const QString& filePath, const OsmAnd::Inspector::Configuration& cfg)
#else
void dump(std::ostream &output, const QString& filePath, const OsmAnd::Inspector::Configuration& cfg)
#endif
{
    std::shared_ptr<QFile> file(new QFile(filePath));
    if(!file->exists())
    {
        output << xT("Binary OsmAnd index ") << QStringToStlString(filePath) << xT(" was not found.") << std::endl;
        return;
    }

    if(!file->open(QIODevice::ReadOnly))
    {
        output << xT("Failed to open file ") << QStringToStlString(file->fileName()) << std::endl;
        return;
    }

    OsmAnd::ObfReader obfMap(file);
    output << xT("Binary index ") << QStringToStlString(file->fileName()) << xT(" version = ") << obfMap.version << std::endl;
    int idx = 1;
    for(auto itSection = obfMap.sections.begin(); itSection != obfMap.sections.end(); ++itSection, idx++)
    {
        OsmAnd::ObfSection* section = *itSection;

        auto sectionType = xT("unknown");
        if(dynamic_cast<OsmAnd::ObfMapSection*>(section))
            sectionType = xT("Map");
        else if(dynamic_cast<OsmAnd::ObfTransportSection*>(section))
            sectionType = xT("Transport");
        else if(dynamic_cast<OsmAnd::ObfRoutingSection*>(section))
            sectionType = xT("Routing");
        else if(dynamic_cast<OsmAnd::ObfPoiSection*>(section))
            sectionType = xT("Poi");
        else if(dynamic_cast<OsmAnd::ObfAddressSection*>(section))
            sectionType = xT("Address");

        output << idx << xT(". ") << sectionType << xT(" data ") << QStringToStlString(section->name) << xT(" - ") << section->length << xT(" bytes") << std::endl;

        if(dynamic_cast<OsmAnd::ObfTransportSection*>(section))
        {
            auto transportSection = dynamic_cast<OsmAnd::ObfTransportSection*>(section);
            int sh = (31 - OsmAnd::ObfTransportSection::StopZoom);
            output << "\tBounds " << formatBounds(transportSection->area24.left << sh, transportSection->area24.right << sh, transportSection->area24.top << sh, transportSection->area24.bottom << sh) << std::endl;
        }
        else if(dynamic_cast<OsmAnd::ObfRoutingSection*>(section))
        {
            auto routingSection = dynamic_cast<OsmAnd::ObfRoutingSection*>(section);
            double lonLeft = 180;
            double lonRight = -180;
            double latTop = -90;
            double latBottom = 90;
            for(auto itSubsection = routingSection->_subsections.begin(); itSubsection != routingSection->_subsections.end(); ++itSubsection)
            {
                auto subsection = itSubsection->get();

                lonLeft = std::min(lonLeft, OsmAnd::Utilities::get31LongitudeX(subsection->area31.left));
                lonRight = std::max(lonRight, OsmAnd::Utilities::get31LongitudeX(subsection->area31.right));
                latTop = std::max(latTop, OsmAnd::Utilities::get31LatitudeY(subsection->area31.top));
                latBottom = std::min(latBottom, OsmAnd::Utilities::get31LatitudeY(subsection->area31.bottom));
            }
            output << xT("\tBounds ") << formatGeoBounds(lonLeft, lonRight, latTop, latBottom) << std::endl;
        }
        else if(dynamic_cast<OsmAnd::ObfMapSection*>(section))
        {
            auto mapSection = dynamic_cast<OsmAnd::ObfMapSection*>(section);
            int levelIdx = 1;
            for(auto itLevel = mapSection->mapLevels.begin(); itLevel != mapSection->mapLevels.end(); ++itLevel, levelIdx++)
            {
                auto level = (*itLevel);
                output << xT("\t") << idx << xT(".") << levelIdx << xT(" Map level minZoom = ") << level->minZoom << xT(", maxZoom = ") << level->maxZoom << std::endl;
                output << xT("\t\tBounds ") << formatBounds(level->area31.left, level->area31.right, level->area31.top, level->area31.bottom) << std::endl;
            }

            if(cfg.verboseMap)
                printMapDetailInfo(output, cfg, &obfMap, mapSection);
        }
        else if(dynamic_cast<OsmAnd::ObfPoiSection*>(section) && cfg.verbosePoi)
        {
            printPOIDetailInfo(output, cfg, &obfMap, dynamic_cast<OsmAnd::ObfPoiSection*>(section));
        }
        else if (dynamic_cast<OsmAnd::ObfAddressSection*>(section) &&cfg. verboseAddress)
        {
            printAddressDetailedInfo(output, cfg, &obfMap, dynamic_cast<OsmAnd::ObfAddressSection*>(section));
        }
    }

    file->close();
}

#if defined(_UNICODE) || defined(UNICODE)
void printMapDetailInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfMapSection* section)
#else
void printMapDetailInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfMapSection* section)
#endif
{
    QList< std::shared_ptr<OsmAnd::Model::MapObject> > mapObjects;
    OsmAnd::AreaI bbox31;
    bbox31.top = OsmAnd::Utilities::get31TileNumberY(cfg.bbox.top);
    bbox31.bottom = OsmAnd::Utilities::get31TileNumberY(cfg.bbox.bottom);
    bbox31.left = OsmAnd::Utilities::get31TileNumberX(cfg.bbox.left);
    bbox31.right = OsmAnd::Utilities::get31TileNumberX(cfg.bbox.right);
    OsmAnd::ObfMapSection::loadMapObjects(reader, section, cfg.zoom, &bbox31, &mapObjects);
    output << xT("\tTotal map objects: ") << mapObjects.count() << std::endl;
    if(cfg.verboseMapObjects)
    {
        for(auto itMapObject = mapObjects.begin(); itMapObject != mapObjects.end(); ++itMapObject)
        {
            auto mapObject = *itMapObject;
            output << xT("\t\t") << mapObject->id << std::endl;
            if(mapObject->names.count() > 0)
            {
                output << xT("\t\t\tNames:");
                for(auto itName = mapObject->names.begin(); itName != mapObject->names.end(); ++itName)
                    output << QStringToStlString(itName.value()) << xT(", ");
                output << std::endl;
            }
            else
                output << xT("\t\t\tNames: [none]") << std::endl;
        }
    }
}

#if defined(_UNICODE) || defined(UNICODE)
void printPOIDetailInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfPoiSection* section)
#else
void printPOIDetailInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfPoiSection* section)
#endif
{
    output << xT("\tBounds ") << formatBounds(section->area31.left, section->area31.right, section->area31.top, section->area31.bottom) << std::endl;

    QList< std::shared_ptr<OsmAnd::Model::Amenity::Category> > categories;
    OsmAnd::ObfPoiSection::loadCategories(reader, section, categories);
    output << xT("\tCategories:") << std::endl;
    for(auto itCategory = categories.begin(); itCategory != categories.end(); ++itCategory)
    {
        auto category = *itCategory;
        output << xT("\t\t") << QStringToStlString(category->name) << std::endl;
        for(auto itSubcategory = category->subcategories.begin(); itSubcategory != category->subcategories.end(); ++itSubcategory)
            output << xT("\t\t\t") << QStringToStlString(*itSubcategory) << std::endl;
    }

    QList< std::shared_ptr<OsmAnd::Model::Amenity> > amenities;
    OsmAnd::AreaI bbox31;
    bbox31.top = OsmAnd::Utilities::get31TileNumberY(cfg.bbox.top);
    bbox31.bottom = OsmAnd::Utilities::get31TileNumberY(cfg.bbox.bottom);
    bbox31.left = OsmAnd::Utilities::get31TileNumberX(cfg.bbox.left);
    bbox31.right = OsmAnd::Utilities::get31TileNumberX(cfg.bbox.right);
    OsmAnd::ObfPoiSection::loadAmenities(reader, section, cfg.zoom, 3, &bbox31, nullptr, &amenities);
    output << xT("\tAmenities, ") << amenities.count() << xT(" item(s)");
    if(!cfg.verboseAmenities)
    {
        output << std::endl;
        return;
    }
    output << ":" << std::endl;;
    for(auto itAmenity = amenities.begin(); itAmenity != amenities.end(); ++itAmenity)
    {
        auto amenity = *itAmenity;

        output << xT("\t\t") <<
            QStringToStlString(amenity->latinName) << xT(" [") << amenity->id << xT("], ") <<
            QStringToStlString(categories[amenity->categoryId]->name) << xT(":") << QStringToStlString(categories[amenity->categoryId]->subcategories[amenity->subcategoryId]) <<
            xT(", lat ") << OsmAnd::Utilities::get31LatitudeY(amenity->point31.y) << xT(" lon ") << OsmAnd::Utilities::get31LongitudeX(amenity->point31.x) << std::endl;
    }
}

#if defined(_UNICODE) || defined(UNICODE)
void printAddressDetailedInfo(std::wostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfAddressSection* section)
#else
void printAddressDetailedInfo(std::ostream& output, const OsmAnd::Inspector::Configuration& cfg, OsmAnd::ObfReader* reader, OsmAnd::ObfAddressSection* section)
#endif
{
    OsmAnd::ObfAddressSection::AddressBlocksSection::Type types[] = {
        OsmAnd::ObfAddressSection::AddressBlocksSection::Type::CitiesOrTowns,
        OsmAnd::ObfAddressSection::AddressBlocksSection::Type::Villages,
        OsmAnd::ObfAddressSection::AddressBlocksSection::Type::Postcodes,
    };
    char* strTypes[] = {
        "Cities/Towns section",
        "Villages section",
        "Postcodes section",
    };
    for(int typeIdx = 0; typeIdx < sizeof(types)/sizeof(types[0]); typeIdx++)
    {
        auto type = types[typeIdx];

        QList< std::shared_ptr<OsmAnd::Model::StreetGroup> > streetGroups;
        OsmAnd::ObfAddressSection::loadStreetGroups(reader, section, &streetGroups, nullptr, nullptr, 1 << types[typeIdx]);

        output << xT("\t") << strTypes[typeIdx] << xT(", ") << streetGroups.size() << xT(" group(s)");
        if(!cfg.verboseStreetGroups)
        {
            output << std::endl;
            continue;
        }
        output << ":" << std::endl;
        for(auto itStreetGroup = streetGroups.begin(); itStreetGroup != streetGroups.end(); itStreetGroup++)
        {
            auto g = *itStreetGroup;

            QList< std::shared_ptr<OsmAnd::Model::Street> > streets;
            OsmAnd::ObfAddressSection::loadStreetsFromGroup(reader, g.get(), &streets);
            output << xT("\t\t'") << QStringToStlString(g->_latinName) << xT("' [") << g->_id << xT("], ") << streets.size() << xT(" street(s)");
            if(!cfg.verboseStreets)
            {
                output << std::endl;
                continue;
            }
            output << xT(":") << std::endl;
            for(auto itStreet = streets.begin(); itStreet != streets.end(); ++itStreet)
            {
                auto s = *itStreet;
                //TODO: proper filter
                /*
                const bool isInside =
                    cfg.latTop >= s->latitude &&
                    cfg.latBottom <= s->latitude &&
                    cfg.lonLeft <= s->longitude &&
                    cfg.lonRight >= s->longitude;
                if(!isInside)
                    continue;
                */
                QList< std::shared_ptr<OsmAnd::Model::Building> > buildings;
                OsmAnd::ObfAddressSection::loadBuildingsFromStreet(reader, s.get(), &buildings);
                QList< std::shared_ptr<OsmAnd::Model::StreetIntersection> > intersections;
                OsmAnd::ObfAddressSection::loadIntersectionsFromStreet(reader, s.get(), &intersections);
                output << xT("\t\t\t'") << QStringToStlString(s->latinName) << xT("' [") << s->id << xT("], ") << buildings.size() << xT(" building(s), ") << intersections.size() << xT(" intersection(s)") << std::endl;
                if(cfg.verboseBuildings && buildings.size() > 0)
                {
                    output << xT("\t\t\t\tBuildings:") << std::endl;
                    for(auto itBuilding = buildings.begin(); itBuilding != buildings.end(); ++itBuilding)
                    {
                        auto building = *itBuilding;

                        if(building->_interpolationInterval != 0)
                            output << xT("\t\t\t\t\t") << QStringToStlString(building->_latinName) << xT("-") << QStringToStlString(building->_latinName2) << xT(" (+") << building->_interpolationInterval << xT(") [") << building->_id << xT("]") << std::endl;
                        else if(building->_interpolation != OsmAnd::Model::Building::Interpolation::Invalid)
                            output << xT("\t\t\t\t\t") << QStringToStlString(building->_latinName) << xT("-") << QStringToStlString(building->_latinName2) << xT(" (") << building->_interpolation << xT(") [") << building->_id << xT("]") << std::endl;
                    }
                }
                if(cfg.verboseIntersections && intersections.size() > 0)
                {
                    output << xT("\t\t\t\tIntersects with:") << std::endl;
                    for(auto itIntersection = intersections.begin(); itIntersection != intersections.end(); ++itIntersection)
                    {
                        auto intersection = *itIntersection;

                        output << xT("\t\t\t\t\t") << QStringToStlString(intersection->latinName) << std::endl;
                    }
                }
            }
        }
    }
}

#if defined(_UNICODE) || defined(UNICODE)
std::wstring formatBounds(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
#else
std::string formatBounds(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
#endif
{
    double l = OsmAnd::Utilities::get31LongitudeX(left);
    double r = OsmAnd::Utilities::get31LongitudeX(right);
    double t = OsmAnd::Utilities::get31LatitudeY(top);
    double b = OsmAnd::Utilities::get31LatitudeY(bottom);
    return formatGeoBounds(l, r, t, b);
}

#if defined(_UNICODE) || defined(UNICODE)
std::wstring formatGeoBounds(double l, double r, double t, double b)
#else
std::string formatGeoBounds(double l, double r, double t, double b)
#endif
{
#if defined(_UNICODE) || defined(UNICODE)
    std::wostringstream oStream;
#else
    std::ostringstream oStream;
#endif
    oStream << xT("(left top - right bottom) : ") << l << xT(", ") << t << xT(" NE - ") << r << xT(", ") << b << xT(" NE");
    return oStream.str();
}
