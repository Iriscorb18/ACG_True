#include "shader.h"
#include "../materials/phong.h"
#include "../core/utils.h"


WhittedIntshader::WhittedIntshader() :
    WhittedColor(Vector3D(1, 0, 0))
{ }

WhittedIntshader::WhittedIntshader(Vector3D wColor_, Vector3D bgColor_) :
    Shader(bgColor_), WhittedColor(wColor_)
{ }


//no completat, revisar
Vector3D WhittedIntshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    //(FILL..)
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D total_whittedcolor = new Vector3D(0.0, 0.0, 0.0);
        Vector3D phongreflectance = Phong::getReflectance(???);

        for (int i = 0; i < lsList.size(); i++) {
            total_whittedcolor = i * phongreflectance * (wi * n);
            if (not_visible) {
                total_whittedcolor = 0;
            }
        }
        return WhittedColor;
    }
    else {
        return bgColor;
    }


}