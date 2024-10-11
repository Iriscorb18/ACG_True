#include "arealightsource.h"

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    //FILL(...)
    // 
    //New Randam Pos inside Area Lightsource????
    /*Vector3D corner = myAreaLightsource->corner;
    Vector3D max_corner = (myAreaLightsource->v2 + myAreaLightsource->v1)+corner;
    int pos_x = rand()*max_corner.x;
    int pos_y = rand() * max_corner.y;
    int pos_z = rand() * max_corner.z;
    return Vector3D(pos_x,pos_y,pos_z);*/
    return Vector3D(0.0, 0.0, 0.0);
}

