#pragma once

#include "scene.h"

struct Integrator {
    Integrator(Scene& scene);

    long long render(int spp,int sam_method);

    Scene scene;
    Texture outputImage;
};