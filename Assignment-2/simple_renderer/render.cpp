#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

long long Integrator::render(int option)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay,option);
            if (si.didIntersect){
                Vector3f color,ptclr;
                color = Vector3f(0.f, 0.f, 0.f);
                for (auto light : this->scene.lights){
                    Vector3f product = Vector3f(si.texture[0]*light.radiance[0], si.texture[1]*light.radiance[1], si.texture[2]*light.radiance[2]);
                    if(light.type == DIRECTIONAL_LIGHT){
                        Ray shadow_ray = Ray(si.p + 1e-5*si.n, light.direction);
                        Interaction shd_si = this->scene.rayIntersect(shadow_ray,option);
                        if(!shd_si.didIntersect){
                            float cos = Dot(light.direction,si.n);
                            color += product*cos/M_PI;
                        }
                    }
                    else if(light.type == POINT_LIGHT){
                        Vector3f vect = light.position - si.p;
                        Vector3f vectNorm = Normalize(vect);
                        Ray shadow_ray = Ray(si.p + 1e-5*si.n, vectNorm);
                        Interaction shd_si = this->scene.rayIntersect(shadow_ray,option);
                        if(!shd_si.didIntersect || shd_si.t > vect.Length()){
                            float cos = Dot(vectNorm, si.n);
                            color += product*cos/(M_PI*vect.LengthSquared());
                        }
                    }
                }
                this->outputImage.writePixelColor(color, x, y);
            }
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <interpolation_variant>";
        return 1;
    }
    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    auto renderTime = rayTracer.render(atoi(argv[3]));
    
    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
