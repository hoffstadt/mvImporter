#include "mvLights.h"
#include "mvAssetManager.h"


mvPointLight
create_point_light(mvAssetManager& am)
{
    mvPointLight light;

    light.camera = mvCreatePerspectiveCamera(
        light.info.viewLightPos.xyz(),
        (f32)M_PI_4,
        1.0f,
        0.1f,
        400.0f
    );

    // create mesh
    light.mesh = mvCreateCube(am, 0.25f);

    // create constant buffer
    light.buffer = mvCreateConstBuffer(&light.info, sizeof(PointLightInfo));

    mvRegisterAsset(&am, "pointlight_buffer", light.buffer);

    return light;
}

mvDirectionalLight
create_directional_light(mvAssetManager& am)
{
    mvDirectionalLight light;

    light.buffer = mvCreateConstBuffer(&light.info, sizeof(DirectionLightInfo));

    mvRegisterAsset(&am, "directionallight_buffer", light.buffer);

    return light;
}