#include "mvObjLoader.h"
#include <fstream>
#include <sstream>
#include <assert.h>

mv_internal void
ProcessMaterialLine(const std::string& line, mvObjMaterial** currentMaterial, std::vector<mvObjMaterial>& materials)
{

	// ignore comments
	if (line.rfind("#", 0) == 0)
		return;

	std::vector<std::string> tokens;

	std::stringstream stream(line);

	for (std::string token; std::getline(stream, token, ' ');)
		tokens.push_back(token);

	// check if line is empty
	if (tokens.size() < 2)
		return;

	// check if new material
	if (tokens[0] == std::string("newmtl"))
	{
		materials.push_back(mvObjMaterial());
		*currentMaterial = &materials.back();
		(*currentMaterial)->name = tokens[1];
	}

	else if (tokens[0] == std::string("Ka"))
	{
		(*currentMaterial)->ambientColor.r = std::stof(tokens[1]);
		(*currentMaterial)->ambientColor.g = std::stof(tokens[2]);
		(*currentMaterial)->ambientColor.b = std::stof(tokens[3]);
	}

	else if (tokens[0] == std::string("Kd"))
	{
		(*currentMaterial)->diffuseColor.r = std::stof(tokens[1]);
		(*currentMaterial)->diffuseColor.g = std::stof(tokens[2]);
		(*currentMaterial)->diffuseColor.b = std::stof(tokens[3]);
	}

	else if (tokens[0] == std::string("Ks"))
	{
		(*currentMaterial)->specularColor.r = std::stof(tokens[1]);
		(*currentMaterial)->specularColor.g = std::stof(tokens[2]);
		(*currentMaterial)->specularColor.b = std::stof(tokens[3]);
	}

	else if (tokens[0] == std::string("Tf"))
	{
		(*currentMaterial)->transmissionFilter.r = std::stof(tokens[1]);
		(*currentMaterial)->transmissionFilter.g = std::stof(tokens[2]);
		(*currentMaterial)->transmissionFilter.b = std::stof(tokens[3]);
	}

	else if (tokens[0] == std::string("Ke"))
	{
		(*currentMaterial)->emissive.r = std::stof(tokens[1]);
		(*currentMaterial)->emissive.g = std::stof(tokens[2]);
		(*currentMaterial)->emissive.b = std::stof(tokens[3]);
	}

	else if (tokens[0] == std::string("Ni"))
		(*currentMaterial)->opticalDensity = std::stof(tokens[1]);

	else if (tokens[0] == std::string("Pr"))
	{
		(*currentMaterial)->roughness = std::stof(tokens[1]);
		(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("Pm"))
	{
		(*currentMaterial)->metallic = std::stof(tokens[1]);
		(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("Ps"))
	{
		(*currentMaterial)->sheen = std::stof(tokens[1]);
		(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("Ns"))
		(*currentMaterial)->specularExponent = std::stof(tokens[1]);

	else if (tokens[0] == std::string("d"))
		(*currentMaterial)->dissolve = std::stof(tokens[1]);

	else if (tokens[0] == std::string("Tr"))
		(*currentMaterial)->dissolve = 1 - std::stof(tokens[1]);

	else if (tokens[0] == std::string("map_Ka"))
		(*currentMaterial)->ambientMap = tokens[1];

	else if (tokens[0] == std::string("map_Kd"))
		(*currentMaterial)->diffuseMap = tokens[1];

	else if (tokens[0] == std::string("map_Kn"))
		(*currentMaterial)->normalMap = tokens[1];

	else if (tokens[0] == std::string("map_Ks"))
		(*currentMaterial)->specularMap = tokens[1];

	else if (tokens[0] == std::string("map_Ke"))
		(*currentMaterial)->emissiveMap = tokens[1];

	else if (tokens[0] == std::string("map_Ns"))
		(*currentMaterial)->specularExponentMap = tokens[1];

	else if (tokens[0] == std::string("map_d"))
		(*currentMaterial)->alphaMap = tokens[1];

	else if (tokens[0] == std::string("map_bump"))
		(*currentMaterial)->bumpMap = tokens[1];

	else if (tokens[0] == std::string("map_Pr"))
	{
		(*currentMaterial)->roughnessMap = tokens[1];
		(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("map_Pm"))
	{
	(*currentMaterial)->metallicMap = tokens[1];
	(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("map_Ps"))
	{
	(*currentMaterial)->sheenMap = tokens[1];
	(*currentMaterial)->pbr = true;
	}

	else if (tokens[0] == std::string("disp"))
	(*currentMaterial)->displacementMap = tokens[1];
}

mv_internal void
ProcessMeshLine(const std::string& line, mvObjMesh** currentMesh, mvObjModel& model)
{

	// ignore comments
	if (line.rfind("#", 0) == 0)
		return;

	std::vector<std::string> tokens;

	std::stringstream stream(line);

	for (std::string token; std::getline(stream, token, ' ');)
		tokens.push_back(token);

	// check if line is empty
	if (tokens.size() < 2)
		return;

	// check if new mesh
	if (tokens[0] == std::string("o"))
	{
		model.rootNode.name = tokens[1];
	}

	else if (tokens[0] == std::string("g"))
	{
		model.meshes.push_back(new mvObjMesh());
		model.meshes.back()->name = tokens[1];

		model.rootNode.children.push_back({});
		model.rootNode.children.back().name = tokens[1];
		model.rootNode.children.back().meshes.push_back(model.meshes.size() - 1);
	}

	else if (tokens[0] == std::string("mtllib"))
		model.materialLib = tokens[1];

	else if (tokens[0] == std::string("usemtl"))
	{
		*currentMesh = model.meshes.back();
		(*currentMesh)->material = tokens[1];
	}

	else if (tokens[0] == std::string("v"))
	{

		model.verticies.push_back(
			{
				std::stof(tokens[1]),
				std::stof(tokens[2]),
				std::stof(tokens[3])
			});
	}

	else if (tokens[0] == std::string("vn"))
	{

		model.normals.push_back(
			{
				std::stof(tokens[1]),
				std::stof(tokens[2]),
				std::stof(tokens[3])
			});

	}

	else if (tokens[0] == std::string("vt"))
	{

		model.textureCoordinates.push_back(
			{
				std::stof(tokens[1]),
				std::stof(tokens[2])
			});

	}

	else if (tokens[0] == std::string("f"))
	{

		assert(tokens.size() == 4);

		for (int i = 1; i < 4; i++)
		{
			std::vector<std::string> subtokens;

			std::stringstream substream(tokens[i]);

			for (std::string subtoken; std::getline(substream, subtoken, '/');)
				subtokens.push_back(subtoken);

			unsigned int vertexIndex = std::stoul(subtokens[0]) - 1;
			unsigned int uvIndex = std::stoul(subtokens[1]) - 1;
			unsigned int normalIndex = std::stoul(subtokens[2]) - 1;

			(*currentMesh)->averticies.push_back({});
			mvObjVertex& vertex = (*currentMesh)->averticies.back();
			vertex.position = model.verticies[vertexIndex];
			vertex.normal = model.normals[normalIndex];
			vertex.uv = model.textureCoordinates[uvIndex];

			(*currentMesh)->indicies.push_back((*currentMesh)->indicies.size());
			(*currentMesh)->vertexCount++;
		}
		(*currentMesh)->triangleCount++;
	}

}

mv_internal void
PostProcess(std::vector<mvObjMesh*>& meshes)
{
	for (auto mesh : meshes)
	{
		for (size_t i = 0; i < mesh->triangleCount; i++)
		{
			size_t i0 = mesh->indicies[i * 3];
			size_t i1 = mesh->indicies[i * 3 + 1];
			size_t i2 = mesh->indicies[i * 3 + 2];

			mvVec3 pos0 = mesh->averticies[i0].position;
			mvVec3 pos1 = mesh->averticies[i1].position;
			mvVec3 pos2 = mesh->averticies[i2].position;

			mvVec2 tex0 = mesh->averticies[i0].uv;
			mvVec2 tex1 = mesh->averticies[i1].uv;
			mvVec2 tex2 = mesh->averticies[i2].uv;

			mvVec3 edge1 = pos1 - pos0;
			mvVec3 edge2 = pos2 - pos0;

			mvVec2 uv1 = tex1 - tex0;
			mvVec2 uv2 = tex2 - tex0;

			float dirCorrection = (uv1.x * uv2.y - uv1.y * uv2.x) < 0.0f ? -1.0f : 1.0f;

			if (uv1.x * uv2.y == uv1.y * uv2.x)
			{
				uv1.x = 0.0f;
				uv1.y = 1.0f;
				uv2.x = 1.0f;
				uv2.y = 0.0f;
			}

			mvVec3 tangent = {
				((edge1.x * uv2.y) - (edge2.x * uv1.y)) * dirCorrection,
				((edge1.y * uv2.y) - (edge2.y * uv1.y)) * dirCorrection,
				((edge1.z * uv2.y) - (edge2.z * uv1.y)) * dirCorrection
			};

			mvVec3 bitangent = {
				((edge1.x * uv2.x) - (edge2.x * uv1.x)) * dirCorrection,
				((edge1.y * uv2.x) - (edge2.y * uv1.x)) * dirCorrection,
				((edge1.z * uv2.x) - (edge2.z * uv1.x)) * dirCorrection
			};

			// project tangent and bitangent into the plane formed by the vertex' normal
			mesh->averticies[i0].tangent = tangent - mesh->averticies[i0].normal * (tangent * mesh->averticies[i0].normal);
			mesh->averticies[i1].tangent = tangent - mesh->averticies[i1].normal * (tangent * mesh->averticies[i1].normal);
			mesh->averticies[i2].tangent = tangent - mesh->averticies[i2].normal * (tangent * mesh->averticies[i2].normal);
			mesh->averticies[i0].bitangent = bitangent - mesh->averticies[i0].normal * (bitangent * mesh->averticies[i0].normal);
			mesh->averticies[i1].bitangent = bitangent - mesh->averticies[i1].normal * (bitangent * mesh->averticies[i1].normal);
			mesh->averticies[i2].bitangent = bitangent - mesh->averticies[i2].normal * (bitangent * mesh->averticies[i2].normal);

			// normalize
			mesh->averticies[i0].tangent = mvNormalize(mesh->averticies[i0].tangent);
			mesh->averticies[i1].tangent = mvNormalize(mesh->averticies[i1].tangent);
			mesh->averticies[i2].tangent = mvNormalize(mesh->averticies[i2].tangent);
			mesh->averticies[i0].bitangent = mvNormalize(mesh->averticies[i0].bitangent);
			mesh->averticies[i1].bitangent = mvNormalize(mesh->averticies[i1].bitangent);
			mesh->averticies[i2].bitangent = mvNormalize(mesh->averticies[i2].bitangent);
		}

		// left hand
		for (size_t i = 0; i < mesh->triangleCount; i++)
		{
			size_t i0 = mesh->indicies[i * 3];
			size_t i1 = mesh->indicies[i * 3 + 1];
			size_t i2 = mesh->indicies[i * 3 + 2];

			mesh->indicies[i * 3] = i2;
			mesh->indicies[i * 3 + 2] = i0;


			mesh->averticies[i0].position.z *= -1.0f;
			mesh->averticies[i1].position.z *= -1.0f;
			mesh->averticies[i2].position.z *= -1.0f;

			mesh->averticies[i0].normal.z *= -1.0f;
			mesh->averticies[i1].normal.z *= -1.0f;
			mesh->averticies[i2].normal.z *= -1.0f;

			mesh->averticies[i0].tangent.z *= -1.0f;
			mesh->averticies[i1].tangent.z *= -1.0f;
			mesh->averticies[i2].tangent.z *= -1.0f;

			mesh->averticies[i0].bitangent.z *= -1.0f;
			mesh->averticies[i1].bitangent.z *= -1.0f;
			mesh->averticies[i2].bitangent.z *= -1.0f;

			mesh->averticies[i0].uv.g = 1 - mesh->averticies[i0].uv.g;
			mesh->averticies[i1].uv.g = 1 - mesh->averticies[i1].uv.g;
			mesh->averticies[i2].uv.g = 1 - mesh->averticies[i2].uv.g;

		}

	}
}

std::vector<mvObjMaterial>
mvLoadObjMaterials(const std::string& file)
{
	mvObjMaterial* currentMaterial = nullptr;
	std::vector<mvObjMaterial> materials;

	std::ifstream inputStream(file);

	for (std::string line; std::getline(inputStream, line);)
		ProcessMaterialLine(line, &currentMaterial, materials);

	return materials;
}

mvObjModel
mvLoadObjModel(const std::string& file)
{
	mvObjModel model{};
	mvObjMesh* currentMesh = nullptr;

	std::ifstream inputStream(file);

	for (std::string line; std::getline(inputStream, line);)
		ProcessMeshLine(line, &currentMesh, model);

	PostProcess(model.meshes);

	return model;
}
