#include "shader.h"
#include "../core/scene.h"
#include "../core/hemisphericalsampler.h"


class Neeshader : public Shader
{
public:
    // Constructors
    Neeshader();
    Neeshader(Vector3D PTColor, Vector3D bgColor_);

    // Override function to compute color (final radiance computation)
    virtual Vector3D computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const override;

    // Function to compute radiance with next event estimation
    virtual Vector3D ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int depth, Intersection its) const;

    // Member variable to store the NEE color
    Vector3D NEEColor;

    // Function to calculate reflected radiance
    Vector3D ReflectedRadiance(const Intersection& its, const Vector3D& wo, int depth, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const;

    // Function to calculate direct radiance
    Vector3D DirectRadiance(const Intersection& its, const Vector3D& wo, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const;

    // Function to calculate indirect radiance (path tracing)
    Vector3D IndirectRadiance(const Intersection& its, const Vector3D& wo, int depth, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const;
};
