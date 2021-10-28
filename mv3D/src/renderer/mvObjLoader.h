#pragma once

#include <string>
#include <vector>
#include "mvMath.h"
#include "mvTypes.h"

struct mvObjMaterial
{
	std::string name;
	b8          pbr = false;

	mvVec3 ambientColor       = { 1.0f, 1.0f, 1.0f}; // Ka
	mvVec3 diffuseColor       = { 1.0f, 1.0f, 1.0f}; // Kd
	mvVec3 specularColor      = { 1.0f, 1.0f, 1.0f}; // Ks
	mvVec3 transmissionFilter = { 1.0f, 1.0f, 1.0f}; // Tf
	mvVec3 emissive           = { 0.0f, 0.0f, 0.0f}; // Ke

	f32 opticalDensity   = 1.0f; // Ni
	f32 specularExponent = 0.0f; // Ns
	f32 dissolve         = 1.0f; // d

	std::string ambientMap = "";          // map_Ka
	std::string diffuseMap = "";          // map_Kd
	std::string normalMap = "";           // map_Kn
	std::string specularMap = "";         // map_Ks
	std::string specularExponentMap = ""; // map_Ns
	std::string emissiveMap = "";         // map_Ke
	std::string alphaMap = "";            // map_d
	std::string bumpMap = "";             // map_bump
	std::string displacementMap = "";     // disp

	// physically-based rendering
	f32 roughness          = 0.5f;  // Pr
	f32 metallic           = 0.5f;  // Pm
	f32 sheen              = 0.04f; // Ps

	std::string roughnessMap = ""; // map_Pr
	std::string metallicMap = "";  // map_Pm
	std::string sheenMap = "";     // map_Ps

};

struct mvObjVertex
{
	mvVec3 position = {};
	mvVec3 normal = {};
	mvVec2 uv = {};
	mvVec3 tangent = {};
	mvVec3 bitangent = {};
};

struct mvObjMesh
{
	std::string              name;
	std::string              material;
	u32                      vertexCount = 0u;
	u32                      triangleCount = 0u;
	std::vector<mvObjVertex> averticies;
	std::vector<u32>         indicies;
};

struct mvObjNode
{
	std::string            name;
	std::vector<mvObjNode> children;
	std::vector<u32>       meshes;
};

struct mvObjModel
{
	std::vector<mvObjMesh*> meshes;
	mvObjNode               rootNode;
	std::vector<mvVec3>     verticies;
	std::vector<mvVec3>     normals;
	std::vector<mvVec2>     textureCoordinates;
	std::string             materialLib;
};

std::vector<mvObjMaterial> mvLoadObjMaterials(const std::string& file);
mvObjModel                 mvLoadObjModel    (const std::string& file);

