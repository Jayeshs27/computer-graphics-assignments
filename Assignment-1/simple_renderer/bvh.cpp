#include "scene.h"


bvhPtr CreateNode(){
    bvhPtr ptr = new bvhNode;
    ptr->leftPtr = NULL;
    ptr->rightPtr = NULL;
    ptr->IsLeaf = true;
    return ptr;
}

bvhPtr Surface::DivideBvh(bvhPtr &node){
    if(node->faces.size() <= 3){
        return NULL;
    }
    node->leftPtr = CreateNode();
    node->rightPtr = CreateNode();
    Vector3f diff = node->MaxPt - node->MinPt;
    int splitAxis = 0;
    if (diff[0] < diff[1]) splitAxis = 1;
    if (diff[splitAxis] < diff[2]) splitAxis = 2;

    float border = node->MinPt[splitAxis] + (diff[splitAxis] / 2);
    int leftCk = 0;
    int rightCk = 0;
    node->leftPtr->MaxPt = Vector3f(-1e30f, -1e30f, -1e30f);
    node->rightPtr->MaxPt = Vector3f(-1e30f, -1e30f, -1e30f);
    node->leftPtr->MinPt = Vector3f(1e30f, 1e30f, 1e30f);
    node->rightPtr->MinPt = Vector3f(1e30f, 1e30f, 1e30f);

    for (auto& face : node->faces) {
        float center = 0;
        for(int j = 0 ; j < 3 ; ++j){
        
            center += this->vertices[face[j]][splitAxis];
        }
        center /= 3;

        if(center < border){
            node->leftPtr->faces.push_back(face);
            for(int i = 0 ; i < 3 ; ++i){
                for(int j = 0 ; j < 3 ; ++j){
                    node->leftPtr->MaxPt[j] = fmaxf(this->vertices[face[i]][j],node->leftPtr->MaxPt[j]);
                    node->leftPtr->MinPt[j] = fminf(this->vertices[face[i]][j],node->leftPtr->MinPt[j]);
                }
            }
            leftCk = 1;
        }
        else{   
            node->rightPtr->faces.push_back(face);
            for(int i = 0 ; i < 3 ; ++i){
                for(int j = 0 ; j < 3 ; ++j){
                    node->rightPtr->MaxPt[j] = fmaxf(this->vertices[face[i]][j],node->rightPtr->MaxPt[j]);
                    node->rightPtr->MinPt[j] = fminf(this->vertices[face[i]][j],node->rightPtr->MinPt[j]);
                }
            }
            rightCk = 1;
        }
    }

    if((leftCk + rightCk) != 2){
        delete node->leftPtr;
        node->leftPtr = NULL;
        delete node->rightPtr;
        node->rightPtr = NULL;
        return NULL;
    }
    return node;
}
void Surface::BuildBvh(bvhPtr &node){
     if(DivideBvh(node) == NULL){
        node->IsLeaf = true;
        return;
     }
     node->IsLeaf = false;

     BuildBvh(node->leftPtr);
     BuildBvh(node->rightPtr);
}

void Scene::CreateBvh(vector<int> BoxIndices){
    for(auto& index : BoxIndices){
        bvhPtr nodePtr = CreateNode();
        Surface* surf = this->AABboxes[index].surface;
        nodePtr->MaxPt = Vector3f(-1e30f, -1e30f, -1e30f);
        nodePtr->MinPt = Vector3f(1e30f, 1e30f, 1e30f);
        for(auto face : surf->indices){
           nodePtr->faces.push_back(face);
        }
        for(auto& vertex : surf->vertices){
            for(int i = 0 ; i < 3 ; ++i){
                nodePtr->MaxPt[i] = fmaxf(nodePtr->MaxPt[i],vertex[i]);
                nodePtr->MinPt[i] = fminf(nodePtr->MinPt[i],vertex[i]);
            }
        }
        
        this->AABboxes[index].Root = nodePtr;
        this->AABboxes[index].surface->BuildBvh(nodePtr);
    }
}

Interaction Scene::BvhTraversal(TlbvhPtr &Tlnode,Ray &ray){
    Interaction siFinal,si;
    siFinal.didIntersect = false;
    float tmin = ray.t;
    for(auto& index : Tlnode->BoxIndices){
        bvhPtr& node = this->AABboxes[index].Root;
        if(slabTest(ray,node->MinPt,node->MaxPt)){
            si = this->AABboxes[index].surface->TraverseRecursive(node,ray);
            if(si.t < tmin){
                tmin = si.t;
                siFinal = si;
            }
        }
    }
    return siFinal;
}

Interaction Surface::TraverseRecursive(bvhPtr &node,Ray &ray){
    if(node->IsLeaf){
        Interaction siFinal;
        float tmin = ray.t;
        if(slabTest(ray,node->MinPt,node->MaxPt)){
            for(auto& face : node->faces){
                Vector3f p1 = this->vertices[face.x];
                Vector3f p2 = this->vertices[face.y];
                Vector3f p3 = this->vertices[face.z];

                Vector3f n1 = this->normals[face.x];
                Vector3f n2 = this->normals[face.y];
                Vector3f n3 = this->normals[face.z];  
                Vector3f n = Normalize(n1 + n2 + n3);

                Interaction si = this->rayTriangleIntersect(ray, p1, p2, p3, n);
                if (si.t <= tmin && si.didIntersect) {
                    siFinal = si;
                    tmin = si.t;
                }
            }
        }
        return siFinal;
    }

    Interaction si,sir;
    if(slabTest(ray,node->MinPt,node->MaxPt)){
        si = TraverseRecursive(node->leftPtr, ray);
        sir = TraverseRecursive(node->rightPtr, ray);
        if(sir.didIntersect && sir.t < si.t){
            si = sir;
        }
    } 
    return si;
}




