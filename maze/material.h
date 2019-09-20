#ifndef MATERIAL_H
#define MATERIAL_H

class Material {
public:
    float r, g, b;
    float kd, ks, shininess;
    unsigned int diffTex;
    unsigned int normTex;
    unsigned int specTex;
    float texCoordFactor;
    Material() {}
    Material(float r, float g, float b, float kd, float ks, float shininess,
            unsigned int diffTex = 0, unsigned int normTex = 0, unsigned int specTex = 0,
            float texCoordFactor = 1.0f) :
        r(r), g(g), b(b), kd(kd), ks(ks), shininess(shininess),
        diffTex(diffTex), normTex(normTex), specTex(specTex),
        texCoordFactor(texCoordFactor)
    {}
};

#endif // MATERIAL_H
