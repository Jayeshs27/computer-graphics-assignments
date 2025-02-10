#include "light.h"

std::vector<Light> LoadLightData(nlohmann::json sceneConfig){
    std::vector<Light> lights;
    /*Direction Lights*/
    try {
        auto dir_lights = sceneConfig["directionalLights"];
        for(auto light : dir_lights){
            Light lite;
            lite.type = DIRECTIONAL_LIGHT;  
            lite.direction = Vector3f(light["direction"][0],light["direction"][1],light["direction"][2]);
            lite.radiance = Vector3f(light["radiance"][0],light["radiance"][1],light["radiance"][2]);
            lights.push_back(lite);
        }
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "\"output\" directional light not defined" << std::endl;
        exit(1);
    }
    /*Point Lights*/
    try {
        auto point_lights = sceneConfig["pointLights"];
        for(auto light : point_lights){
            Light lite;
            lite.type = POINT_LIGHT;  
            lite.position = Vector3f(light["location"][0],light["location"][1],light["location"][2]);
            lite.radiance = Vector3f(light["radiance"][0],light["radiance"][1],light["radiance"][2]);
            lights.push_back(lite);
        }
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "\"output\" points light not defined" << std::endl;
        exit(1);
    }
    return lights;
}
