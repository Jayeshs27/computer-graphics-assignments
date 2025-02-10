#pragma once

#include "common.h"
#include "texture.h"


#include <bits/stdc++.h>
using namespace std;

/*For Q -2.3*/
struct bvhNode{
    Vector3f MinPt,MaxPt;
    struct bvhNode* leftPtr;
    struct bvhNode* rightPtr;
    bool IsLeaf;
    vector<Vector3i> faces;
};
typedef struct bvhNode bvhNode;
typedef struct bvhNode* bvhPtr;


struct Surface {
    std::vector<Vector3f> vertices, normals;
    std::vector<Vector3i> indices;
    std::vector<Vector2f> uvs;

    bool isLight;
    uint32_t shapeIdx;

    Vector3f diffuse;
    float alpha;
    
    /*For Q2.3*/
    bvhPtr DivideBvh(bvhPtr &node);
    void BuildBvh(bvhPtr &root);

    Texture diffuseTexture, alphaTexture;

    Interaction rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n);
    Interaction rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n);
    Interaction rayIntersect(Ray ray);
    
    /*For Q2.3*/
    Interaction TraverseRecursive(bvhPtr &node,Ray &ray);

private:
    bool hasDiffuseTexture();
    bool hasAlphaTexture();
};

std::vector<Surface> createSurfaces(std::string pathToObj, bool isLight, uint32_t shapeIdx);