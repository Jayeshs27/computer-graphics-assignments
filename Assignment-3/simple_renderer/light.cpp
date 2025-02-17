#include "light.h"
#include "surface.h"

Light::Light(LightType type, nlohmann::json config) {
    switch (type) {
        case LightType::POINT_LIGHT:
            this->position = Vector3f(config["location"][0], config["location"][1], config["location"][2]);
            break;
        case LightType::DIRECTIONAL_LIGHT:
            this->direction = Vector3f(config["direction"][0], config["direction"][1], config["direction"][2]);
            break;
        case LightType::AREA_LIGHT:
            // my implementation
            this->center = Vector3f(config["center"][0],config["center"][1],config["center"][2]);
            this->normal = Vector3f(config["normal"][0],config["normal"][1],config["normal"][2]);
            this->vx = Vector3f(config["vx"][0],config["vx"][1],config["vx"][2]);
            this->vy = Vector3f(config["vy"][0],config["vy"][1],config["vy"][2]); 
            break;
        default:
            std::cout << "WARNING: Invalid light type detected";
            break;
    }

    this->radiance = Vector3f(config["radiance"][0], config["radiance"][1], config["radiance"][2]);
    this->type = type;
}


std::pair<Vector3f, LightSample> Light::sample(Interaction *si) {
    LightSample ls;
    memset(&ls, 0, sizeof(ls));

    Vector3f radiance;
    switch (type) {
        case LightType::POINT_LIGHT:
            ls.wo = (position - si->p);
            ls.d = ls.wo.Length();
            ls.wo = Normalize(ls.wo);
            radiance = (1.f / (ls.d * ls.d)) * this->radiance;
            break;
        case LightType::DIRECTIONAL_LIGHT:
            ls.wo = Normalize(direction);
            ls.d = 1e10;
            radiance = this->radiance;
            break;
        case LightType::AREA_LIGHT:
            // TODO: Implement this
            Vector3f randomP = this->center + 2*(next_float() - 0.5f)*this->vx + 2*(next_float() - 0.5f)*this->vy;
            ls.wo = (randomP - si->p);
            ls.d = ls.wo.Length();
            ls.wo = Normalize(ls.wo);
            if(Dot(this->normal,-ls.wo) > 0)  
                radiance = this->radiance;
            break;
    }
    return { radiance, ls };
}

Interaction Light::intersectLight(Ray *ray) {
    Interaction si;
    memset(&si, 0, sizeof(si));
    si.didIntersect = false;

    if (type == LightType::AREA_LIGHT) {
        // my implementation
        Vector3f v1,v2,v3,v4;
        v1 = this->center + this->vx + this->vy;
        v2 = this->center - this->vx + this->vy;
        v3 = this->center - this->vx - this->vy;
        v4 = this->center + this->vx - this->vy;

        Surface surf;
        si = surf.rayTriangleIntersect(*ray,v1,v2,v3,this->normal);
        if(!si.didIntersect){
            si = surf.rayTriangleIntersect(*ray,v3,v4,v1,this->normal);
        }
        if(Dot(this->normal,ray->d) < 0 && si.didIntersect){
            si.emissiveColor = this->radiance;
        }
    }

    return si;
}