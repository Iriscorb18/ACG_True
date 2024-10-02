#include "depthshader.h"

#include "../core/utils.h"

DepthShader::DepthShader() :
    color(Vector3D(1, 0, 0))
{ }

DepthShader::DepthShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D DepthShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        double distance2camera = (r.o - its.itsPoint).length(); //camera distance - intersection point
       
        return color*(1 - distance2camera / 7.0);
    }
    else {
        return bgColor;
    }
    return color;
}
