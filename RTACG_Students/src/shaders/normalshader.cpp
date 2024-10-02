#include "normalshader.h"
#include "../core/utils.h"

Normalshader::Normalshader() :
    normalColor(Vector3D(1, 0, 0))
{ }

Normalshader::Normalshader(Vector3D nColor_, Vector3D bgColor_) :
    Shader(bgColor_), normalColor(nColor_)
{ }

Vector3D Normalshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    //(FILL..)

    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        return (its.normal + Vector3D(1.0, 1.0, 1.0)) / 2;
    }
    else {
        return bgColor;
    }

}
