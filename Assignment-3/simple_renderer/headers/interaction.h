#pragma once

#include "vec.h"

// Forward declaration of BSDF class
class BSDF;

struct Interaction {
    // Position of interaction
    Vector3f p;
    // Normal of the surface at interaction
    Vector3f n;
    // The uv co-ordinates at the intersection point
    Vector2f uv;
    // The viewing direction in local shading frame
    Vector3f wi; 
    // Distance of intersection point from origin of the ray
    float t = 1e30f; 
    // Used for light intersection, holds the radiance emitted by the emitter.
    Vector3f emissiveColor = Vector3f(0.f, 0.f, 0.f);
    // BSDF at the shading point
    BSDF* bsdf;
    // Vectors defining the orthonormal basis
    Vector3f a, b, c;

    bool didIntersect = false;

    Vector3f toWorld(Vector3f w) {
        // TODO: Implement this
        Vector3f nw;
        int min_ind,i,j;
        float Nmin;
        Nmin = abs(n[0]);
        min_ind = 0;
        i=1;
        j=2;
        if(abs(n[1]) < Nmin){
            Nmin = abs(n[1]);
            min_ind = 1;
            i=0;
        }
        if(abs(n[2]) < Nmin){
            min_ind = 2;
            i=0;
            j=1;
        }
        b[min_ind] = 0;
        b[i] = -n[j];
        b[j] = n[i];
        c = n;
        a = Normalize(Cross(b,c));
        // Taking transpose
        nw.x = Dot(Vector3f(a.x, b.x, c.x),w);
        nw.y = Dot(Vector3f(a.y, b.y, c.y),w);
        nw.z = Dot(Vector3f(a.z, b.z, c.z),w);
    
        return nw;
    }

    Vector3f toLocal(Vector3f w) {
        // TODO: Implement this
        Vector3f nw;
        nw.x = Dot(a, w);
        nw.y = Dot(b, w);
        nw.z = Dot(c, w);
        return nw;
    }
};