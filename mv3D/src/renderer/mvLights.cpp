#include "mvLights.h"
#include "mv3D_internal.h"
#include "mvAssetManager.h"

mvPointLight
mvCreatePointLight(mvAssetManager* manager, mvVec3 pos)
{
	mvPointLight light{};
	light.info.viewLightPos = { pos.x, pos.y, pos.z, 1.0f };
	light.buffer = mvCreateConstBuffer(&light.info, sizeof(mvPointLightInfo));

	mvMesh mesh = mvCreateTexturedCube(*manager, 0.25f);

	mesh.phongMaterialID = mvGetPhongMaterialAsset(manager, "Phong_VS.hlsl", "Solid_PS.hlsl", true, false, false, false);
	manager->phongMaterials[mesh.phongMaterialID].material.data.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	mvAssetID mesh_id = mvRegistryMeshAsset(manager, mesh);

	light.mesh = &manager->meshes[mesh_id].mesh;

	return light;
}

mvDirectionLight
mvCreateDirectionLight(mvVec3 dir)
{
	mvDirectionLight light{};
	light.info.viewLightDir = dir;
	light.buffer = mvCreateConstBuffer(&light.info, sizeof(mvDirectionLight));
	return light;
}

void
mvBindSlot_bPS(u32 slot, mvPointLight& light, mvMat4 viewMatrix)
{
	mvVec4 posCopy = light.info.viewLightPos;

	mvVec4 out = viewMatrix * light.info.viewLightPos;
	light.info.viewLightPos.x = out.x;
	light.info.viewLightPos.y = out.y;
	light.info.viewLightPos.z = out.z;

	mvUpdateConstBuffer(light.buffer, &light.info);
	GContext->graphics.imDeviceContext->PSSetConstantBuffers(slot, 1u, light.buffer.buffer.GetAddressOf());
	light.info.viewLightPos = posCopy;
}

void
mvBindSlot_bPS(u32 slot, mvDirectionLight& light, mvMat4 viewMatrix)
{

	mvVec3 posCopy = light.info.viewLightDir;

	mvVec4 out = viewMatrix * mvVec4{ 
		light.info.viewLightDir.x,
		light.info.viewLightDir.y,
		light.info.viewLightDir.z,
		0.0f };
	light.info.viewLightDir.x = out.x;
	light.info.viewLightDir.y = out.y;
	light.info.viewLightDir.z = out.z;

	mvUpdateConstBuffer(light.buffer, &light.info);
	GContext->graphics.imDeviceContext->PSSetConstantBuffers(slot, 1u, light.buffer.buffer.GetAddressOf());
	light.info.viewLightDir = posCopy;
}
