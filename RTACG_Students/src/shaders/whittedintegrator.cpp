
#include "shader.h"
#include "whittedintegrator.h"
#include "../materials/phong.h"
#include "../core/utils.h"
#include "../lightsources/pointlightsource.h"



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

    if (Utils::getClosestIntersection(r, objList, its)) { //normalizar direccions
        Vector3D WhittedColor = Vector3D(0.0, 0.0, 0.0);
        int Vs = 1;
        for (int i = 0; i < lsList.size(); i++) {
            Vector3D wi = (lsList.at(i)->sampleLightPosition() - its.itsPoint).normalized();
            Vector3D n = its.normal;
            Vector3D wo = -r.d;
            Vector3D reflectance = its.shape->getMaterial().getReflectance(n, wo, wi);
            Ray robj;
            robj.o = its.itsPoint;
            robj.d = wi;
         
            robj.maxT = (robj.o - lsList.at(i)->sampleLightPosition()).length();
            if (Utils::hasIntersection(robj, objList)) {
                Vs = 0;
            }
            WhittedColor += reflectance * lsList.at(i)->getIntensity() * n * wi* Vs;
        }
        return WhittedColor;
    }
       
    else {
        return bgColor;
    }


}

/*
* Intersection its;

if (Utils::getClosestIntersection(r, objList, its)) {
   
    Vector3D total_whittedcolor = new Vector3D(0.0, 0.0, 0.0);
    //Vector3D phongreflectance = Phong::getReflectance(???);


    for (int s = 0; s < lsList.size(); s++) {

        Ray ray = new Ray();

        PointLightSource* Li = dynamic_cast<PointLightSource*>(lsList[s]);

        Vector3D wi = (Li.sampleLightPosition() - its.itsPoint); //vector direction from x to the position of the point light source Ls

        int Vs = Utils::hasIntersection(ray, objList, its) ? 0 : 1; // visible? 

        Vector3D n = its.normal;

        Vector3D wo = -r.d;

        Vector3D reflectance = its.shape->getMaterial().getReflectance(n, wo, r.d);

        total_whittedcolor += reflectance * lsList.at(i).getIntensity(its.itsPoint) * Vs;

    }
    return total_whittedcolor;
}
*/