#include "mvScene.h"
#include "mvSandbox.h"
#include <imgui.h>

mvScene
mvCreateScene()
{
	mvScene scene{};
	scene.buffer = mvCreateConstBuffer(&scene.info, sizeof(mvSceneInfo));
	return scene;
}
