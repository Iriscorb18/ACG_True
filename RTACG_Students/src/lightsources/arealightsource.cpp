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
    // Corner and edge vectors that define the area light
    Vector3D corner = myAreaLightsource->corner;
    Vector3D v1 = myAreaLightsource->v1;
    Vector3D v2 = myAreaLightsource->v2;

    // Generate random floats between 0 and 1
    double u = (double)(rand()) / RAND_MAX;
    double v = (double)(rand()) / RAND_MAX;

    // Generate a random point inside the rectangle by scaling along the edges
    Vector3D randomPoint = corner + v1*u + v2*v;
    
    return randomPoint;
}

