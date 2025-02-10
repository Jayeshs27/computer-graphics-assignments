#include "render.h"

Integrator::Integrator(Scene &scene) {
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

Vector3f UniformSamples(){
    float theta, phi;
    theta = acos(next_float()); 
    phi = 2 * M_PI * next_float();
    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}
Vector3f CosineSamples(){
    float theta, phi;
    theta = acos(sqrt(next_float()));
    phi = 2 * M_PI * next_float();
    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}
Light randomLight(std::vector<Light> lights){
    int rnd_ind = next_float() * lights.size();
    rnd_ind = rnd_ind % lights.size();
    return lights[rnd_ind];
}
long long Integrator::render(int spp, int sam_method) {
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Vector3f result(0, 0, 0);
            Interaction siEmit;
            for (int z = 0; z < spp; z++) {
                Ray cameraRay = this->scene.camera.generateRay(x, y);
                Interaction si = this->scene.rayIntersect(cameraRay);
                if (si.didIntersect) {
                    if(sam_method == 0){
                        // uniform sampling
                        Vector3f wsam = UniformSamples();
                        Ray shadowRay(si.p + 1e-3f * si.n, si.toWorld(wsam));
                        Interaction siLight = this->scene.rayEmitterIntersect(shadowRay);
                        Interaction siShadow = this->scene.rayIntersect(shadowRay);
                        if (!siShadow.didIntersect || siLight.t < siShadow.t) {
                            result += 2 * M_PI * si.bsdf->eval(&si, si.toLocal(wsam)) * siLight.emissiveColor * std::abs(Dot(si.n, si.toWorld(wsam)));
                        }

                    }
                    else if(sam_method == 1){
                        // cosine sampling
                        Vector3f wsam = CosineSamples();
                        Ray shadowRay(si.p + 1e-3f * si.n, si.toWorld(wsam));
                        Interaction siLight = this->scene.rayEmitterIntersect(shadowRay);
                        Interaction siShadow = this->scene.rayIntersect(shadowRay);
                        if (!siShadow.didIntersect || siLight.t < siShadow.t) {
                            result += M_PI * si.bsdf->eval(&si, si.toLocal(wsam)) * siLight.emissiveColor;
                        }
                    }
                    else if(sam_method == 2){
                        // light sampling
                        Light light = randomLight(this->scene.lights);
                        Vector3f radiance;
                        LightSample ls;
                        std::tie(radiance, ls) = light.sample(&si);
                        Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                        Interaction siShadow = this->scene.rayIntersect(shadowRay);
                        if (!siShadow.didIntersect || ls.d < siShadow.t) {
                            result += (light.Area() * si.bsdf->eval(&si, ls.wo) * radiance * std::abs(Dot(si.n, ls.wo)) * Dot(light.getNormal(), -ls.wo)) /(ls.d * ls.d);
                        }
                    }
                }
            }
            result /= spp;

            Interaction siScene,siLight;
            Ray cRay = this->scene.camera.generateRay(x, y);
            siLight = this->scene.rayEmitterIntersect(cRay);
            siScene = this->scene.rayIntersect(cRay);
            if(siLight.t < siScene.t){   // checking if there is not object b/w light and camera
                result += siLight.emissiveColor;
            }

            this->outputImage.writePixelColor(result, x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv) {
    if (argc != 5) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <num_samples> <sampling_strategy>";
        return 1;
    }
    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    int spp = atoi(argv[3]);
    auto renderTime = rayTracer.render(spp,atoi(argv[4]));

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
