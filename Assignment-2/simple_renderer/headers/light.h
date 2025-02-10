#pragma once

#include "common.h"

enum LightType {
	POINT_LIGHT=0,
	DIRECTIONAL_LIGHT=1,
	NUM_LIGHT_TYPES
};


struct Light {
	LightType type;
	Vector3f position;
	Vector3f direction;
	Vector3f radiance;
	Ray GetRay(Vector3f pt);
	Vector3f GetShade();
};

typedef struct Light Light;
std::vector<struct Light> LoadLightData(nlohmann::json sceneConfig);