#include "shader.h"
#include "../core/scene.h"
#include "../core/hemisphericalsampler.h"


class Neeshader : public Shader
{
public:
    Neeshader();
    Neeshader(Vector3D PTColor, Vector3D bgColor_);
    virtual Vector3D computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const;
    virtual Vector3D ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList, int max_depth, Intersection itsscene, HemisphericalSampler hs, Vector3D wi, double A) const;

    Vector3D NEEColor;
};