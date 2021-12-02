#include "mvScene.h"
#include "mv3D_internal.h"
#include <imgui.h>

mvScene
mvCreateScene()
{
	mvScene scene{};
	scene.buffer = mvCreateConstBuffer(&scene.info, sizeof(mvSceneInfo));
	return scene;
}
