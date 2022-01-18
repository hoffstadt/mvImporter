#include "mvLights.h"

mvPointLight
create_point_light()
{
    mvPointLight light;

    // create mesh
    light.mesh = create_cube(0.25f);

    // create constant buffer
    light.buffer = create_const_buffer(&light.info, sizeof(mvPointLightInfo));

    return light;
}

mvDirectionalLight
create_directional_light()
{
    mvDirectionalLight light;

    light.buffer = create_const_buffer(&light.info, sizeof(mvDirectionLightInfo));

    return light;
}