#pragma once

#include "camera.h"
#include "surface.h"


#include <bits/stdc++.h>
using namespace std;

/*For question -2.2*/
struct TlbvhNode{
    Vector3f MinPt,MaxPt;
    struct TlbvhNode* leftPtr;
    struct TlbvhNode* rightPtr;
    bool IsLeaf;
    vector<int> BoxIndices;
};

typedef struct TlbvhNode TlbvhNode;
typedef struct TlbvhNode* TlbvhPtr;

/*For question -2.1*/
typedef struct AAbb {
    Vector3f Minpoint;
    Vector3f Maxpoint;
    Surface* surface;
    Vector3f Centre;
    bvhPtr Root;
}AAbb;

bool slabTest(Ray& ray, Vector3f bmin, Vector3f bmax );



struct Scene {
    std::vector<Surface> surfaces;
    std::vector<AAbb> AABboxes;
    Camera camera;
    Vector2i imageResolution;
    

    Scene() {};
    Scene(std::string sceneDirectory, std::string sceneJson);
    Scene(std::string pathToJson);
    
    void parse(std::string sceneDirectory, nlohmann::json sceneConfig);

    /*Q -2.1*/
    void CreateAAbb();
    Interaction IsRayIntersect(Ray& ray);

    /*Q -2.2*/
    TlbvhPtr TlBvhRoot;
    void CreateTlBvh(int option);
    TlbvhPtr DivideTlBvh(TlbvhPtr &node);
    void BuildTlBvh(TlbvhPtr &root,int option);
    Interaction GetInterRecursive(Ray& ray,TlbvhPtr &node,int option);
    

    /*Q -2.3*/
    void CreateBvh(vector<int> BoxIndices);
    Interaction BvhTraversal(TlbvhPtr &node,Ray& ray);

    Interaction rayIntersect(Ray& ray);

};


