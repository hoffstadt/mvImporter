#include "mvLights.h"
#include "mvAssetManager.h"


mvPointLight
create_point_light(mvAssetManager& am)
{
    mvPointLight light;

    light.camera = create_perspective_camera(
        light.info.viewLightPos.xyz(),
        (f32)M_PI_4,
        1.0f,
        0.1f,
        400.0f
    );

    // create mesh
    light.mesh = create_cube(am, 0.25f);

    // create constant buffer
    light.buffer = create_const_buffer(&light.info, sizeof(mvPointLightInfo));

    register_asset(&am, "pointlight_buffer", light.buffer);

    return light;
}

mvDirectionalLight
create_directional_light(mvAssetManager& am)
{
    mvDirectionalLight light;

    light.buffer = create_const_buffer(&light.info, sizeof(mvDirectionLightInfo));

    register_asset(&am, "directionallight_buffer", light.buffer);

    return light;
}