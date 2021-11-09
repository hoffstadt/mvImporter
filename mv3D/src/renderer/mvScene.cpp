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

void
mvBindSlot_bPS(uint32_t slot, mvScene& scene)
{
	mvUpdateConstBuffer(scene.buffer, &scene.info);
	GContext->graphics.imDeviceContext->PSSetConstantBuffers(slot, 1u, scene.buffer.buffer.GetAddressOf());
}

void
mvShowControls(mvScene& scene)
{
	ImGui::Begin("Scene");
	ImGui::ColorEdit3("Ambient Color", &scene.info.ambientColor.x);
	ImGui::Checkbox("Use Shadows", (bool*)&scene.info.useShadows);
	ImGui::Checkbox("Use Skybox", (bool*)&scene.info.useSkybox);
	ImGui::End();
}
