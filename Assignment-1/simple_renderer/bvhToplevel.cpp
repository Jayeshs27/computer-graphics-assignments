#include "scene.h"

TlbvhPtr CreateTlNode() {
    TlbvhPtr ptr = new TlbvhNode;
    ptr->leftPtr = NULL;
    ptr->rightPtr = NULL;
    ptr->IsLeaf = true;
    return ptr;
}

TlbvhPtr Scene::DivideTlBvh(TlbvhPtr& node) {
    if (node->BoxIndices.size() <= 1) {
        return NULL;
    }
    node->leftPtr = CreateTlNode();
    node->rightPtr = CreateTlNode();
    Vector3f diff = node->MaxPt - node->MinPt;
    int splitAxis = 0;
    if (diff[0] < diff[1]) splitAxis = 1;
    if (diff[splitAxis] < diff[2]) splitAxis = 2;

    float border = node->MinPt[splitAxis] + (diff[splitAxis] / 2);
    int leftCk = 0;
    int rightCk = 0;
    for (auto& index : node->BoxIndices) {
        if (this->AABboxes[index].Centre[splitAxis] < border) {
            node->leftPtr->BoxIndices.push_back(index);
            leftCk = 1;
        } else {
            node->rightPtr->BoxIndices.push_back(index);
            rightCk = 1;
        }
    }
    if ((leftCk + rightCk) != 2) {
        delete node->leftPtr;
        node->leftPtr = NULL;
        delete node->rightPtr;
        node->rightPtr = NULL;
        return NULL;
    }
    return node;
}

Vector3f getMaxPoint(vector<AAbb>& boxArr, vector<int> boxIndices) {
    Vector3f MaxPt = Vector3f(-1e30f, -1e30f, -1e30f);
    for (auto& index : boxIndices) {
        for (auto& vertex : boxArr[index].surface->vertices) {
            for (int i = 0; i < 3; ++i) {
                MaxPt[i] = fmaxf(boxArr[index].Maxpoint[i], MaxPt[i]);
            }
        }
    }
    return MaxPt;
}
Vector3f getMinPoint(vector<AAbb>& boxArr, vector<int> boxIndices) {
    Vector3f MinPt = Vector3f(1e30f, 1e30f, 1e30f);
    for (auto& index : boxIndices) {
        for (auto& vertex : boxArr[index].surface->vertices) {
            for (int i = 0; i < 3; ++i) {
                MinPt[i] = fminf(boxArr[index].Minpoint[i], MinPt[i]);
            }
        }
    }
    return MinPt;
}

void Scene::BuildTlBvh(TlbvhPtr& node, int option) {
    if (node == NULL) return;

    node->MaxPt = getMaxPoint(this->AABboxes, node->BoxIndices);
    node->MinPt = getMinPoint(this->AABboxes, node->BoxIndices);

    if (DivideTlBvh(node) == NULL) {
        if (option == 3) {
            CreateBvh(node->BoxIndices);
        }
        node->IsLeaf = true;
        return;
    }
    node->IsLeaf = false;
    BuildTlBvh(node->leftPtr, option);
    BuildTlBvh(node->rightPtr, option);
}

void Scene::CreateTlBvh(int option) {
    this->TlBvhRoot = CreateTlNode();
    for (int i = 0; i < this->AABboxes.size(); ++i) {
        this->TlBvhRoot->BoxIndices.push_back(i);
    }
    BuildTlBvh(this->TlBvhRoot, option);
}

Interaction Scene::GetInterRecursive(Ray& ray, TlbvhPtr& node, int option) {
    if (node->IsLeaf) {
        Interaction sInFinal;
        if (slabTest(ray, node->MinPt, node->MaxPt)) {
            if (option == 3) {
                return BvhTraversal(node, ray);
            } else {
                for (auto& index : node->BoxIndices) {
                    if (slabTest(ray, this->AABboxes[index].Minpoint, this->AABboxes[index].Maxpoint)) {
                        Interaction sIn = this->AABboxes[index].surface->rayIntersect(ray);
                        if (sIn.t <= ray.t) {
                            sInFinal = sIn;
                            ray.t = sIn.t;
                        }
                    }
                }
            }
        }
        return sInFinal;
    }

    Interaction si, sir;
    if (slabTest(ray, node->MinPt, node->MaxPt)) {
        si = GetInterRecursive(ray, node->leftPtr, option);
        sir = GetInterRecursive(ray, node->rightPtr, option);
        if (sir.didIntersect && sir.t < si.t) {
            si = sir;
        }
    }
    return si;
}
