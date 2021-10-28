#include "mvScene.h"
#include "mv3D_internal.h"

mvScene
mvCreateScene()
{
	mvScene scene{};
	scene.buffer = mvCreateConstBuffer(&scene.info, sizeof(mvSceneInfo));
	return scene;
}

void
mvBindSlot_b(uint32_t slot, mvScene& scene)
{
	mvUpdateConstBuffer(scene.buffer, &scene.info);
	GContext->graphics.imDeviceContext->PSSetConstantBuffers(slot, 1u, scene.buffer.buffer.GetAddressOf());
}
