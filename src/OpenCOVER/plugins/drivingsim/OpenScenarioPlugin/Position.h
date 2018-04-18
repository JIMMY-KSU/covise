#ifndef POSITION_H
#define POSITION_H

#include<string>
#include<vector>
#include <osg/Vec3>
#include <OpenScenario/schema/oscPosition.h>
#include <OpenScenario/schema/oscShape.h>
#include "Trajectory.h"
#include <TrafficSimulation/AgentVehicle.h>
#include "../RoadTerrain/RoadTerrainPlugin.h"
#include <Entity.h>
#include <TrafficSimulation/CarGeometry.h>
#include <VehicleUtil/RoadSystem/RoadSystem.h>
#include <math.h>       /* sin */
#include <iostream>

class Road;
class ReferencePosition;

class Position : public OpenScenario::oscPosition
{

public:
    Position();
    ~Position();

    //shapeInfo
    OpenScenario::oscShape* shapeInfo;

    // Road Coordinates
    std::string roadId;
    int laneId;
    double offset;
    double s;
    double t;

    // Relative Coordinates (Road/Object)
    double dx;
    double dy;
    double dz;
    osg::Vec3 relPosition;
    std::string relObject;

    //absolute Coordinates
    double hdg;
    double x;
    double y;
    double z;
    osg::Vec3 absPosition;
    ReferencePosition* referencePosition;
    ReferencePosition* newReferencePosition;

    ReferencePosition *getAbsolutePosition(Entity* currentEntity, std::list<Entity*> entityList);
    osg::Vec3 getAbsoluteFromRoad(::Road* road, double s, int landeId);
    osg::Vec3 getAbsoluteFromRoad(::Road* road, double s, double t);

    ReferencePosition* getRelObjectPos(std::string relObject, Entity* currentEntity, std::list<Entity*> entityList);
    double getHdg();
    osg::Vec3 getAbsoluteWorld();



};

#endif // POSITION_H
