#include "shader.h"
#include "areadirectshader.h"
#include "../materials/phong.h"
#include "../core/utils.h"
#include "../lightsources/lightsource.h"

AreaDirectshader::AreaDirectshader() :
    ADColor(Vector3D(1, 0, 0))
{ }

AreaDirectshader::AreaDirectshader(Vector3D ADColor_, Vector3D bgColor_) :
    Shader(bgColor_), ADColor(ADColor_)
{ }

Vector3D AreaDirectshader::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    
    if (Utils::getClosestIntersection(r, objList, its)) { //Loop through all objects to see their closest intersection
        Vector3D finalColor = Vector3D(0.0, 0.0, 0.0);

        // Handle specular materials
        if (its.shape->getMaterial().hasSpecular()) { //Check if material is specular
            Vector3D wi = -r.d.normalized();
            Vector3D N = its.normal.normalized(); 
            Vector3D wr = Utils::computeReflectionDirection(wi, N); // Calling function to compute reflection vector
            Ray reflectRay = Ray(its.itsPoint, wr, r.depth + 1);           
            finalColor = computeColor(reflectRay, objList, lsList); //Adding it to the finalColor
        }

        // Handle transmissive materials (refraction)
        else if (its.shape->getMaterial().hasTransmission()){
            Vector3D wt;
            Vector3D n = its.normal.normalized();
            Vector3D wo = r.d.normalized();
            double ratio = 0.7;

            // Invert ratio and normal for internal rays
            if (dot(wo, n) > 0) { 
                ratio = 1 / ratio;  
                n = -n;
            }

            // Check for total internal reflection
            if (1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2))) >= 0){  //radical is higher than zero
                Vector3D a1 = -sqrt(1 - ratio * ratio * (1 - (pow(dot(-wo, n), 2)))) + ratio * dot(-wo, n); 
                Vector3D a2 = (-wo) * ratio; 
                wt = n * a1 - a2; 
            } else {
                wt = Utils::computeReflectionDirection(wo, n); //Total internal reflection if radical is smaller than zero
            }

            Ray refractray = Ray(its.itsPoint, wt, r.depth + 1);
            finalColor = computeColor(refractray, objList, lsList);
        }
        else if (its.shape->getMaterial().isEmissive()) {
            finalColor = its.shape->getMaterial().getEmissiveRadiance();
        }

        // Handle diffuse or glossy materials
        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {

            Vector3D at = Vector3D(0.15); // Ambient term
            Vector3D pd = its.shape->getMaterial().getDiffuseReflectance(); // Diffuse reflectance
            Vector3D finalDirectIllumination(0.0, 0.0, 0.0);

            double numSamples = 256;

            for (int j = 0; j < lsList.size(); j++) {
                // Monte Carlo integration: sample points on the area light

                double A = lsList.at(j)->getArea();  // Area of the light source
                for (int i = 0; i < numSamples; i++) {
                    // Sample a random point on the area light
                    Vector3D y = (lsList.at(j)->sampleLightPosition());

                    int Vs = 1;//keep changing the value at each sample since we have initialized before the usage of samples, making the first time being zero, always zero

                    // Compute wi (direction from intersection point x to sampled light point y)
                    Vector3D wi = (y - its.itsPoint).normalized();
                    double distanceSquared = (y - its.itsPoint).lengthSq();

                    // Compute visibility (check if the point on the light is visible from the intersection point)
                    Ray shadowRay(its.itsPoint, wi);

                    //(denoted as its.itsPoint) towards a sampled light position y, you want to know whether any object obstructs the line of sight between the two points.
                    shadowRay.maxT = (y - its.itsPoint).length() - 0.01; //We need to make a maximum to avoid this on to be infinite 

                    if (Utils::hasIntersection(shadowRay, objList)) {  //Not having visibility
                        Vs = 0;
                    }
                    if (Vs == 1) {
                        // Get the radiance emitted from the light at point y
                        Vector3D Le = lsList.at(j)->getIntensity();

                        // Compute the geometric term G(x, y)
                        Vector3D n = its.normal.normalized();
                        Vector3D ny = lsList.at(j)->getNormal();  // Normal of the light's surface
                        double G = (dot(n, wi) * dot(-wi, ny)) / distanceSquared;

                        // Get BRDF (Phong reflection or other)
                        Vector3D wo = -r.d;  // Outgoing direction
                        Vector3D reflectance = its.shape->getMaterial().getReflectance(n, -wi, wo);  // BRDF function

                        // Monte Carlo estimator for direct illumination
                        finalDirectIllumination += Le * reflectance * G * Vs / (1 / A);
                    }
                }
            }
            finalColor = finalDirectIllumination * (1 / numSamples) + at * pd;
        }
        return finalColor;
    }
    else {
        return bgColor;
    }
}