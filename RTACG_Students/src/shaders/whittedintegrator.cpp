
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

Vector3D WhittedIntshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
   
    Intersection its;
    

    if (Utils::getClosestIntersection(r, objList, its)) { //Loop through all objects to see their closest intersection
        Vector3D WhittedColor = Vector3D(0.0, 0.0, 0.0);


        if (its.shape->getMaterial().hasSpecular()) { //Check if material is specular


            Vector3D wi = -r.d; 
            Vector3D N = its.normal.normalized(); 

            Vector3D wr = Utils::computeReflectionDirection(wi, N); // Calling function to compute reflection vector
            Ray reflectray = Ray(its.itsPoint, wr, r.depth + 1);           
            WhittedColor += computeColor(reflectray, objList, lsList); //Adding it to the WhittedColor
    
        }

        else if (its.shape->getMaterial().hasTransmission()){ // Transmissive
        
            Vector3D wt;
            Vector3D n = its.normal.normalized();
            Vector3D wo = r.d;
            double ratio = 0.7;

            if (dot(wo, n) > 0) //Check if the ray is "inside" of the object

            {
                ratio = 1 / ratio;  
                n = -n;
            }

            if (1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2))) >= 0){  //radical is higher than zero
                Vector3D a1 = -sqrt(1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2)))) + ratio * dot(-wo, n); 
                Vector3D a2 = (-wo) * ratio; 
                wt = n * a1 - a2; 
            }

            else
            {
                wt = Utils::computeReflectionDirection(wo, n); //Total internal reflection if radical is smaller than zero
            }

            Ray refractray = Ray(its.itsPoint, wt, r.depth);
            WhittedColor += computeColor(refractray, objList, lsList);

        }


        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {

            Vector3D at = Vector3D(0.15, 0.15, 0.15); //Ambient term

            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance();



            int Vs = 1;
           

            for (int i = 0; i < lsList.size(); i++) { 
              
                    Vector3D wi = (lsList.at(i)->sampleLightPosition() - its.itsPoint).normalized();
                    Vector3D n = its.normal.normalized();
                    Vector3D wo = -r.d;
                    Vector3D reflectance = its.shape->getMaterial().getReflectance(n, wo, wi);   //Phong
                    Ray robj;
                    robj.o = its.itsPoint;
                    robj.d = wi;

                    robj.maxT = (robj.o - lsList.at(i)->sampleLightPosition()).length(); //We need to make a maximum to avoid this on to be infinite 
                    if (Utils::hasIntersection(robj, objList)) {  //Not having visibility
                        Vs = 0;
                    }
                    WhittedColor += reflectance * lsList.at(i)->getIntensity() * Vs * dot(n, wi);

                
                
            } 
            WhittedColor  = WhittedColor + at * pd;  


        }

        
        return WhittedColor ;     
        
    }
       
    else {
        return bgColor;
    }


}

