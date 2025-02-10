#include "scene.h"

using namespace std;

Vector3f CameraPos;

Scene::Scene(std::string sceneDirectory, std::string sceneJson)
{
    nlohmann::json sceneConfig;
    try {
        sceneConfig = nlohmann::json::parse(sceneJson);
    }
    catch (std::runtime_error e) {
        std::cerr << "Could not parse json." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

Scene::Scene(std::string pathToJson)
{
    std::string sceneDirectory;

#ifdef _WIN32
    const size_t last_slash_idx = pathToJson.rfind('\\');
#else
    const size_t last_slash_idx = pathToJson.rfind('/');
#endif

    if (std::string::npos != last_slash_idx) {
        sceneDirectory = pathToJson.substr(0, last_slash_idx);
    }

    nlohmann::json sceneConfig;
    try {
        std::ifstream sceneStream(pathToJson.c_str());
        sceneStream >> sceneConfig;
    }
    catch (std::runtime_error e) {
        std::cerr << "Could not load scene .json file." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

void Scene::parse(std::string sceneDirectory, nlohmann::json sceneConfig)
{
    // Output
    try {
        auto res = sceneConfig["output"]["resolution"];
        this->imageResolution = Vector2i(res[0], res[1]);
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "\"output\" field with resolution, filename & spp should be defined in the scene file." << std::endl;
        exit(1);
    }

    // Cameras
    try {
        auto cam = sceneConfig["camera"];
        CameraPos = Vector3f(cam["from"][0], cam["from"][1], cam["from"][2]);
        this->camera = Camera(
            Vector3f(cam["from"][0], cam["from"][1], cam["from"][2]) - CameraPos,
            Vector3f(cam["to"][0], cam["to"][1], cam["to"][2]) - CameraPos,
            Vector3f(cam["up"][0], cam["up"][1], cam["up"][2]),
            float(cam["fieldOfView"]),
            this->imageResolution
        );
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "No camera(s) defined. Atleast one camera should be defined." << std::endl;
        exit(1);
    }

    // Surface
    try {
        auto surfacePaths = sceneConfig["surface"];

        uint32_t surfaceIdx = 0;
        for (std::string surfacePath : surfacePaths) {
            surfacePath = sceneDirectory + "/" + surfacePath;

            auto surf = createSurfaces(surfacePath, /*isLight=*/false, /*idx=*/surfaceIdx);
            this->surfaces.insert(this->surfaces.end(), surf.begin(), surf.end());

            surfaceIdx = surfaceIdx + surf.size();
        }
    }
    catch (nlohmann::json::exception e) {
        std::cout << "No surfaces defined." << std::endl;
    }
}

Interaction Scene::rayIntersect(Ray& ray)
{
    Interaction siFinal;
    for (auto& surface : this->surfaces) {
        Interaction si = surface.rayIntersect(ray);
        if (si.t <= ray.t) {    
            siFinal = si;
            ray.t = si.t;
        }
    }

    return siFinal;
}


void Scene::CreateAAbb()
{
    for (auto& surface : this->surfaces) {
        AAbb AaBbox;
 
        AaBbox.Maxpoint = Vector3f(-1e30f,-1e30f,-1e30f);
        AaBbox.Minpoint = Vector3f(1e30f,1e30f,1e30f);

        AaBbox.surface = &surface;
        int vtx_cnt = 0;
        Vector3f Center = Vector3f(0,0,0);
        for(auto& vertex : surface.vertices){
            for(int i = 0 ; i < 3 ; ++i){
                Center[i] += vertex[i];
                AaBbox.Maxpoint[i] = fmaxf(AaBbox.Maxpoint[i],vertex[i]);
                AaBbox.Minpoint[i] = fminf(AaBbox.Minpoint[i],vertex[i]);
            }
            vtx_cnt++;
        }
        for(int i = 0 ; i < 3 ; ++i){
            Center[i] = Center[i]/vtx_cnt;
        }
        AaBbox.Centre = Center;
        this->AABboxes.push_back(AaBbox);
    }
}

bool slabTest(Ray& ray,Vector3f bmin,Vector3f bmax )
{
    float tx1 = (bmin.x - ray.o.x) / ray.d.x, tx2 = (bmax.x - ray.o.x) / ray.d.x;
    float tmin = min( tx1, tx2 ), tmax = max( tx1, tx2 );
    float ty1 = (bmin.y - ray.o.y) / ray.d.y, ty2 = (bmax.y - ray.o.y) / ray.d.y;
    tmin = max( tmin, min( ty1, ty2 ) ), tmax = min( tmax, max( ty1, ty2 ) );
    float tz1 = (bmin.z - ray.o.z) / ray.d.z, tz2 = (bmax.z - ray.o.z) / ray.d.z;
    tmin = max( tmin, min( tz1, tz2 ) ), tmax = min( tmax, max( tz1, tz2 ) );
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

bool IsIntersectAABB(Ray& ray,AAbb& aabBox){
     return slabTest(ray, aabBox.Minpoint, aabBox.Maxpoint);
}


Interaction Scene::IsRayIntersect(Ray& ray){

    Interaction sInFinal;
    for (auto& box : this->AABboxes) {
        if(IsIntersectAABB(ray, box)){
            Interaction sIn = box.surface->rayIntersect(ray);
            if (sIn.t <= ray.t) {    
                sInFinal = sIn;
                ray.t = sIn.t;
            }

        }
    }
    return sInFinal;
}