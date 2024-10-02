#include "shader.h"

class Normalshader : public Shader
{
public:
    Normalshader();
    Normalshader(Vector3D normalColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D normalColor;
};


