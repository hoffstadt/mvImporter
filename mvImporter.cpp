#include "mvImporter.h"
#include "sJsonParser.h"

static bool
isDataURI(const std::string& in)
{
	std::string header = "data:application/octet-stream;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/jpeg;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/png;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/bmp;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/gif;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:text/plain;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:application/gltf-buffer;base64,";
	if (in.find(header) == 0)
		return true;

	return false;
}

static inline bool
is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string
base64_decode(std::string const& encoded_string)
{
	int in_len = static_cast<int>(encoded_string.size());
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	while (in_len-- && (encoded_string[in_] != '=') &&
		is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_];
		in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] =
				static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] =
				(char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] =
				((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) ret += char_array_3[i];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++) char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] =
			static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] =
			((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

static bool
DecodeDataURI(unsigned char* out, std::string& mime_type, const std::string& in, size_t reqBytes)
{
	std::string header = "data:application/octet-stream;base64,";
	std::string data;
	if (in.find(header) == 0) {
		data = base64_decode(in.substr(header.size()));  // cut mime string.
	}

	if (data.empty()) {
		header = "data:image/jpeg;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/jpeg";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/png;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/png";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/bmp;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/bmp";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/gif;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/gif";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:text/plain;base64,";
		if (in.find(header) == 0) {
			mime_type = "text/plain";
			data = base64_decode(in.substr(header.size()));
		}
	}

	if (data.empty()) {
		header = "data:application/gltf-buffer;base64,";
		if (in.find(header) == 0) {
			data = base64_decode(in.substr(header.size()));
		}
	}

	// TODO(syoyo): Allow empty buffer? #229
	if (data.empty()) 
	{
		return false;
	}

	//out->resize(data.size());

	std::copy(data.begin(), data.end(), out);
	return true;
}

namespace mvImp {

	static std::string*
	_LoadExtensions(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("extensionsUsed"))
			return nullptr;

		unsigned extensionCount = j["extensionsUsed"].childCount;

		std::string* extensions = new std::string[extensionCount];

		for (int i = 0; i < extensionCount; i++)
		{
			sJsonObject& jExtension = j["extensionsUsed"][i];
			extensions[i] = jExtension.value;
			size++;
		}

		return extensions;
	}

	static mvGLTFAnimation*
	_LoadAnimations(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("animations"))
			return nullptr;

		unsigned animationCount = j["animations"].childCount;

		mvGLTFAnimation* animations = new mvGLTFAnimation[animationCount];

		for (int i = 0; i < animationCount; i++)
		{
			sJsonObject& janimation = j["animations"][i];
			mvGLTFAnimation& animation = animations[i];
			animation.channel_count = 0u;
			animation.channels = nullptr;
			animation.sampler_count = 0u;
			animation.samplers = nullptr;
			animation.name = janimation.getStringMember("name", "");

			if (janimation.doesMemberExist("samplers"))
			{
				unsigned samplerCount = janimation["samplers"].childCount;
				animation.samplers = new mvGLTFAnimationSampler[samplerCount];
				animation.sampler_count = samplerCount;

				for (int i = 0; i < samplerCount; i++)
				{
					sJsonObject& jsampler = janimation["samplers"][i];
					mvGLTFAnimationSampler& sampler = animation.samplers[i];
					sampler.input = jsampler.getIntMember("input", -1);
					sampler.output = jsampler.getIntMember("output", -1);
					sampler.interpolation = jsampler.getStringMember("interpolation", "LINEAR");

				}
			}

			if (janimation.doesMemberExist("channels"))
			{
				unsigned channelCount = janimation["channels"].childCount;
				animation.channels = new mvGLTFAnimationChannel[channelCount];
				animation.channel_count = channelCount;

				for (int i = 0; i < channelCount; i++)
				{
					sJsonObject& jchannel = janimation["channels"][i];
					mvGLTFAnimationChannel& channel = animation.channels[i];

					channel.sampler = jchannel.getIntMember("sampler", -1);

					if (jchannel.doesMemberExist("target"))
					{

						sJsonObject& jchanneltarget = *jchannel.getMember("target");
						mvGLTFAnimationChannelTarget target{};
						channel.target.node = jchanneltarget.getIntMember("node", -1);
						channel.target.path = jchanneltarget.getStringMember("path");

					}
				}
			}

			size++;
		}

		return animations;
	}

	static mvGLTFCamera*
	_LoadCameras(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("cameras"))
			return nullptr;

		unsigned cameraCount = j["cameras"].childCount;

		mvGLTFCamera* cameras = new mvGLTFCamera[cameraCount];

		for (int i = 0; i < cameraCount; i++)
		{
			sJsonObject& jcamera = j["cameras"][i];
			mvGLTFCamera& camera = cameras[i];
			camera.name = jcamera.getStringMember("name");
			std::string type = jcamera.getStringMember("type", "perspective");

			if (type == "perspective")
			{
				camera.type = MV_IMP_PERSPECTIVE;
			}
			else
			{
				camera.type = MV_IMP_ORTHOGRAPHIC;
			}

			if (jcamera.doesMemberExist("perspective"))
			{
				sJsonObject& perspective = jcamera["perspective"];
				camera.perspective.aspectRatio = perspective.getFloatMember("aspectRatio", 0.0f);
				camera.perspective.yfov = perspective.getFloatMember("yfov", 0.0f);
				camera.perspective.zfar = perspective.getFloatMember("zfar", 0.0f);
				camera.perspective.znear = perspective.getFloatMember("znear", 0.0f);

			}

			if (jcamera.doesMemberExist("orthographic"))
			{
				sJsonObject& orthographic = jcamera["orthographic"];
				camera.orthographic.xmag = orthographic.getFloatMember("xmag", 0.0f);
				camera.orthographic.ymag = orthographic.getFloatMember("ymag", 0.0f);
				camera.orthographic.zfar = orthographic.getFloatMember("zfar", 0.0f);
				camera.orthographic.znear = orthographic.getFloatMember("znear", 0.0f);
			}

			size++;
		}

		return cameras;
	}

	static mvGLTFScene*
	_LoadScenes(sJsonObject& j, unsigned& size)
	{

		if (!j.doesMemberExist("scenes"))
			return nullptr;

		unsigned count = j["scenes"].childCount;
		mvGLTFScene* scenes = new mvGLTFScene[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jscene = j["scenes"][0];
			mvGLTFScene& scene = scenes[size];
			scene.node_count = 0u;
			scene.nodes = nullptr;

			if (jscene.doesMemberExist("nodes"))
			{
				unsigned nodeCount = jscene["nodes"].childCount;
				scene.nodes = new unsigned[nodeCount];
				for (int j = 0; j < nodeCount; j++)
				{
					int node = jscene["nodes"][j].asInt();
					scene.nodes[scene.node_count] = node;
					scene.node_count++;
				}
			}

			size++;
		}

		return scenes;
	}

	static mvGLTFNode*
	_LoadNodes(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("nodes"))
			return nullptr;

		unsigned count = j["nodes"].childCount;
		mvGLTFNode* nodes = new mvGLTFNode[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jnode = j["nodes"][i];
			mvGLTFNode& node = nodes[size];
			node.child_count = 0u;
			node.children = nullptr;
			for(int i = 0; i < 16; i++)
				node.matrix[i] = 0.0f;
			node.matrix[0] = 1.0f;
			node.matrix[5] = 1.0f;
			node.matrix[10] = 1.0f;
			node.matrix[15] = 1.0f;
			node.rotation[0] = 0.0f;
			node.rotation[1] = 0.0f;
			node.rotation[2] = 0.0f;
			node.rotation[3] = 1.0f;
	        node.scale[0] = 1.0f;
	        node.scale[1] = 1.0f;
	        node.scale[2] = 1.0f;
	        node.translation[0] = 0.0f;
	        node.translation[1] = 0.0f;
	        node.translation[2] = 0.0f;
	        node.hadMatrix = false;
			node.name = jnode.getStringMember("name", "");
			node.mesh_index = jnode.getIntMember("mesh", -1);
			node.camera_index = jnode.getIntMember("camera", -1);
			node.skin_index = jnode.getIntMember("skin", -1);

			if (jnode.doesMemberExist("children"))
			{
				unsigned childCount = jnode["children"].childCount;
				node.children = new unsigned[childCount];

				for (int j = 0; j < childCount; j++)
				{
					unsigned child = jnode["children"][j].asFloat();
					node.children[node.child_count] = child;
					node.child_count++;
				}
			}

			if (jnode.doesMemberExist("translation"))
			{
				unsigned compCount = jnode["translation"].childCount;

				for (int j = 0; j < compCount; j++)
				{
					node.translation[j] = jnode["translation"][j].asFloat();
				}
			}

			if (jnode.doesMemberExist("scale"))
			{
				unsigned compCount = jnode["scale"].childCount;

				for (int j = 0; j < compCount; j++)
				{
					node.scale[j] = jnode["scale"][j].asFloat();
				}
			}

			if (jnode.doesMemberExist("rotation"))
			{
				unsigned compCount = jnode["rotation"].childCount;

				for (int j = 0; j < compCount; j++)
				{
					node.rotation[j] = jnode["rotation"][j].asFloat();
				}
			}

			if (jnode.doesMemberExist("matrix"))
			{
				node.hadMatrix = true;
				unsigned compCount = jnode["matrix"].childCount;

				for (int j = 0; j < compCount; j++)
				{
					node.matrix[j] = jnode["matrix"][j].asFloat();
				}
			}

			size++;
		}

		return nodes;
	}

	static mvGLTFMesh*
	_LoadMeshes(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("meshes"))
			return nullptr;

		unsigned meshCount = j["meshes"].childCount;

		mvGLTFMesh* meshes = new mvGLTFMesh[meshCount];

		for (int i = 0; i < meshCount; i++)
		{
			sJsonObject& jmesh = j["meshes"][i];
			mvGLTFMesh& mesh = meshes[i];
			mesh.primitives = nullptr;
			mesh.primitives_count = 0u;
			mesh.weights = nullptr;
			mesh.weights_count = 0u;
			mesh.name = jmesh.getStringMember("name");

			if (jmesh.doesMemberExist("weights"))
			{
				mesh.weights_count = jmesh["weights"].childCount;
				mesh.weights = new float[mesh.weights_count];
				for (int j = 0; j < mesh.weights_count; j++)
				{
					sJsonObject m = jmesh["weights"][j];
					mesh.weights[j] = m.asFloat();
				}
			}

			if (jmesh.doesMemberExist("primitives"))
			{

				mesh.primitives_count = jmesh["primitives"].childCount;
				mesh.primitives = new mvGLTFMeshPrimitive[mesh.primitives_count];

				for (int j = 0; j < mesh.primitives_count; j++)
				{

					mvGLTFMeshPrimitive& primitive = mesh.primitives[j];

					sJsonObject jprimitive = jmesh["primitives"][j];
					primitive.mode = MV_IMP_TRIANGLES;
					primitive.attributes = nullptr;
					primitive.attribute_count = 0u;
					primitive.targets = nullptr;
					primitive.target_count = 0u;
					primitive.indices_index = jprimitive.getIntMember("indices", -1);
					primitive.material_index = jprimitive.getIntMember("material", -1);

					if (jprimitive.doesMemberExist("attributes"))
					{
						sJsonObject jattributes = jprimitive["attributes"];
						unsigned attrCount = jattributes.childCount;
						primitive.attributes = new mvGLTFAttribute[attrCount];
						

						for (int k = 0; k < attrCount; k++)
						{
							sJsonObject& m = jattributes.children[k];
							memcpy(primitive.attributes[primitive.attribute_count].semantic, m.name, MV_IMPORTER_MAX_NAME_LENGTH);
							primitive.attributes[primitive.attribute_count].index = m.asInt();
							primitive.attribute_count++;
						}

					}

					if (jprimitive.doesMemberExist("targets"))
					{
						sJsonObject jtargets = jprimitive["targets"];
						unsigned targetCount = jtargets.childCount;
						primitive.targets = new mvGLTFMorphTarget[targetCount];

						for (int k = 0; k < targetCount; k++)
						{
							mvGLTFMorphTarget& target = primitive.targets[k];

							sJsonObject& jtarget = jtargets.children[k];
							
							unsigned attrCount = jtarget.childCount;
							target.attributes = new mvGLTFAttribute[attrCount];
							for (int x = 0; x < attrCount; x++)
							{
								sJsonObject& jattribute = jtarget.children[x];
								memcpy(target.attributes[target.attribute_count].semantic, jattribute.name, MV_IMPORTER_MAX_NAME_LENGTH);
								target.attributes[target.attribute_count].index = jattribute.asInt();
								target.attribute_count++;
							}

							primitive.target_count++;
						}

					}


				}
			}

			size++;
		}

		return meshes;
	}

	static mvGLTFMaterial*
	_LoadMaterials(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("materials"))
			return nullptr;

		unsigned count = j["materials"].childCount;
		mvGLTFMaterial* materials = new mvGLTFMaterial[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jmaterial = j["materials"][i];
			mvGLTFMaterial& material = materials[size];
			material.name = jmaterial.getStringMember("name", "");
			material.double_sided = jmaterial.getBoolMember("doubleSided", false);
			material.alphaCutoff = jmaterial.getFloatMember("alphaCutoff", 0.5f);
			material.pbrMetallicRoughness = false;
	        material.clearcoat_extension = false;
			material.metallic_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("metallicFactor", 1.0f);
			material.roughness_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("roughnessFactor", 1.0f);
	        material.normal_texture_scale = 1.0f;
	        material.clearcoat_normal_texture_scale = 1.0f;
	        material.occlusion_texture_strength = 1.0f;
	        material.metallic_factor = 1.0f;
	        material.roughness_factor = 1.0f;
	        material.base_color_factor[0] = 1.0f;
	        material.base_color_factor[1] = 1.0f;
	        material.base_color_factor[2] = 1.0f;
	        material.base_color_factor[3] = 1.0f;
	        material.emissive_factor[0] = 0.0f;
	        material.emissive_factor[1] = 0.0f;
	        material.emissive_factor[2] = 0.0f;
	        material.alphaCutoff = 0.5f;
	        material.double_sided = false;
	        material.alphaMode = MV_ALPHA_MODE_OPAQUE;
	        material.clearcoat_factor = 0.0f;
	        material.clearcoat_roughness_factor = 0.0f;
			material.clearcoat_texture = -1;
			material.clearcoat_roughness_texture = -1;
			material.clearcoat_normal_texture = -1;

			if (jmaterial.doesMemberExist("alphaMode"))
			{
				std::string alphaMode = jmaterial.getStringMember("alphaMode");
				if (alphaMode == "OPAQUE")
					material.alphaMode = MV_ALPHA_MODE_OPAQUE;
				else if(alphaMode == "MASK")
					material.alphaMode = MV_ALPHA_MODE_MASK;
				else
					material.alphaMode = MV_ALPHA_MODE_BLEND;
			}

			if (jmaterial.doesMemberExist("pbrMetallicRoughness"))
			{
				material.pbrMetallicRoughness = true;
				material.metallic_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("metallicFactor", 1.0f);
				material.roughness_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("roughnessFactor", 1.0f);


				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["baseColorTexture"].doesMemberExist("index"))
						material.base_color_texture = jmaterial["pbrMetallicRoughness"]["baseColorTexture"].getIntMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0].asFloat();
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1].asFloat();
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2].asFloat();
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3].asFloat();
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getIntMember("index", -1);
				}

			}

			if (jmaterial.doesMemberExist("normalTexture"))
			{
				material.normal_texture = jmaterial["normalTexture"].getIntMember("index", -1);
				material.normal_texture_scale = jmaterial["normalTexture"].getFloatMember("scale", 1.0f);
			}

			if (jmaterial.doesMemberExist("occlusionTexture"))
			{
				material.occlusion_texture = jmaterial["occlusionTexture"].getIntMember("index", -1);
				material.occlusion_texture_strength = jmaterial["occlusionTexture"].getFloatMember("strength", 1.0f);
			}

			if (jmaterial.doesMemberExist("emissiveTexture"))
			{
				material.emissive_texture = jmaterial["emissiveTexture"].getIntMember("index", -1);
			}

			if (jmaterial.doesMemberExist("emissiveFactor"))
			{

				material.emissive_factor[0] = jmaterial["emissiveFactor"][0].asFloat();
				material.emissive_factor[1] = jmaterial["emissiveFactor"][1].asFloat();
				material.emissive_factor[2] = jmaterial["emissiveFactor"][2].asFloat();
			}		

			if (jmaterial.doesMemberExist("extensions"))
			{
				if (jmaterial["extensions"].doesMemberExist("KHR_materials_clearcoat"))
				{
					material.clearcoat_extension = true;
					material.clearcoat_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getFloatMember("clearcoatFactor", 0.0f);
					material.clearcoat_roughness_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getIntMember("clearcoatRoughnessFactor", 0.0f);

					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatTexture"))
					{
						material.clearcoat_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getIntMember("index", -1);
						material.clearcoat_normal_texture_scale = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getFloatMember("scale", 1.0f);
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatRoughnessTexture"))
					{
						material.clearcoat_roughness_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatRoughnessTexture"].getIntMember("index", -1);
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatNormalTexture"))
					{
						material.clearcoat_normal_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatNormalTexture"].getIntMember("index", -1);
					}
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0].asFloat();
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1].asFloat();
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2].asFloat();
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3].asFloat();
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getIntMember("index", -1);
				}
			}

			size++;
		}

		return materials;
	}

	static mvGLTFTexture*
	_LoadTextures(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("textures"))
			return nullptr;

		unsigned count = j["textures"].childCount;
		mvGLTFTexture* textures = new mvGLTFTexture[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jtexture = j["textures"][i];
			mvGLTFTexture& texture = textures[size];
			texture.sampler_index = jtexture.getIntMember("sampler", -1);
			texture.image_index = jtexture.getIntMember("source", -1);
			texture.name = jtexture.getStringMember("name", "");
			size++;
		}

		return textures;
	}

	static mvGLTFSampler*
	_LoadSamplers(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("samplers"))
			return nullptr;

		unsigned count = j["samplers"].childCount;
		mvGLTFSampler* samplers = new mvGLTFSampler[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jsampler = j["samplers"][i];
			mvGLTFSampler& sampler = samplers[size];
			sampler.name = jsampler.getStringMember("name", "");
			sampler.mag_filter = jsampler.getIntMember("magFilter", -1);
			sampler.min_filter = jsampler.getIntMember("minFilter", -1);
			sampler.wrap_s = jsampler.getIntMember("wrapS", MV_IMP_WRAP_REPEAT);
			sampler.wrap_t = jsampler.getIntMember("wrapT", MV_IMP_WRAP_REPEAT);
			size++;
		}
		return samplers;
	}

	static mvGLTFImage*
	_LoadImages(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("images"))
			return nullptr;

		unsigned count = j["images"].childCount;
		mvGLTFImage* images = new mvGLTFImage[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jimage = j["images"][i];
			mvGLTFImage& image = images[size];
			image.uri = jimage.getStringMember("uri", "");
			image.mimeType = jimage.getStringMember("mimeType", "");
			image.buffer_view_index = jimage.getIntMember("bufferView", -1);
			image.data = nullptr;
			image.dataCount = 0u;
			image.embedded = false; // ?
			size++;
		}
		return images;
	}

	static mvGLTFBuffer*
	_LoadBuffers(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("buffers"))
			return nullptr;

		unsigned count = j["buffers"].childCount;
		mvGLTFBuffer* buffers = new mvGLTFBuffer[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jbuffer = j["buffers"][i];
			mvGLTFBuffer& buffer = buffers[size];
			buffer.uri = jbuffer.getStringMember("uri", "");
			buffer.byte_length = jbuffer.getUIntMember("byteLength", 0u);
			buffer.data = nullptr;
			buffer.dataCount = 0u;
			size++;
		}

		return buffers;
	}

	static mvGLTFBufferView*
	_LoadBufferViews(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("bufferViews"))
			return nullptr;

		unsigned count = j["bufferViews"].childCount;
		mvGLTFBufferView* bufferviews = new mvGLTFBufferView[count];

		for (int i = 0; i < count; i++)
		{

			sJsonObject& jbufferview = j["bufferViews"][i];
			mvGLTFBufferView& bufferview = bufferviews[size];
			bufferview.name = jbufferview.getStringMember("name", "");
			bufferview.buffer_index = jbufferview.getIntMember("buffer", -1);
			bufferview.byte_offset = jbufferview.getIntMember("byteOffset", 0);
			bufferview.byte_length = jbufferview.getIntMember("byteLength", -1);
			bufferview.byte_stride = jbufferview.getIntMember("byteStride", -1);
			size++;
		}

		return bufferviews;
	}

	static mvGLTFAccessor*
	_LoadAccessors(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("accessors"))
			return nullptr;

		unsigned count = j["accessors"].childCount;
		mvGLTFAccessor* accessors = new mvGLTFAccessor[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jaccessor = j["accessors"][i];
			mvGLTFAccessor& accessor = accessors[i];
			accessor.name = jaccessor.getStringMember("name", "");
			accessor.byteOffset = jaccessor.getIntMember("byteOffset", 0);
			accessor.count = jaccessor.getIntMember("count", -1);
			accessor.component_type = (mvGLTFComponentType)jaccessor.getIntMember("componentType", MV_IMP_FLOAT);
			accessor.buffer_view_index = jaccessor.getIntMember("bufferView", -1);
			accessor.type = MV_IMP_SCALAR;

			if (jaccessor.doesMemberExist("type"))
			{
				std::string strtype = jaccessor.getStringMember("type");
				if (strtype == "SCALAR")    accessor.type = MV_IMP_SCALAR;
				else if (strtype == "VEC2") accessor.type = MV_IMP_VEC2;
				else if (strtype == "VEC3") accessor.type = MV_IMP_VEC3;
				else if (strtype == "VEC4") accessor.type = MV_IMP_VEC4;
				else if (strtype == "MAT2") accessor.type = MV_IMP_MAT2;
				else if (strtype == "MAT3") accessor.type = MV_IMP_MAT3;
				else if (strtype == "MAT4") accessor.type = MV_IMP_MAT4;
			}

			if (jaccessor.doesMemberExist("max"))
			{

				unsigned min_count = jaccessor["max"].childCount;
				for (unsigned min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.maxes[min_entry] = jaccessor["max"][min_entry].asFloat();
				}
			}

			if (jaccessor.doesMemberExist("min"))
			{

				unsigned min_count = jaccessor["min"].childCount;
				for (unsigned min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.mins[min_entry] = jaccessor["min"][min_entry].asFloat();
				}
			}

			size++;
		}

		return accessors;
	}

	static mvGLTFSkin*
	_LoadSkins(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("skins"))
			return nullptr;

		unsigned count = j["skins"].childCount;
		mvGLTFSkin* skins = new mvGLTFSkin[count];

		for (int i = 0; i < count; i++)
		{
			sJsonObject& jnode = j["skins"][i];
			mvGLTFSkin& skin = skins[size];
			skin.name = jnode.getStringMember("name", "");
			skin.inverseBindMatrices = jnode.getIntMember("inverseBindMatrices", -1);
			skin.skeleton = jnode.getIntMember("skeleton", -1);
			skin.joints = nullptr;
			skin.joints_count = 0u;

			if (jnode.doesMemberExist("joints"))
			{
				unsigned childCount = jnode["joints"].childCount;
				skin.joints = new unsigned[childCount];

				for (int j = 0; j < childCount; j++)
				{
					unsigned child = jnode["joints"][j].asFloat();
					skin.joints[skin.joints_count] = child;
					skin.joints_count++;
				}
			}

			size++;
		}

		return skins;
	}

	static char*
	_ReadFile(const char* file, unsigned& size, const char* mode)
	{
		FILE* dataFile = fopen(file, mode);

		if (dataFile == nullptr)
		{
			assert(false && "File not found.");
			return nullptr;
		}

		else
		{
			// obtain file size:
			fseek(dataFile, 0, SEEK_END);
			size = ftell(dataFile);
			fseek(dataFile, 0, SEEK_SET);

			// allocate memory to contain the whole file:
			char* data = new char[size];

			// copy the file into the buffer:
			size_t result = fread(data, sizeof(char), size, dataFile);
			if (result != size)
			{
				if (feof(dataFile))
					printf("Error reading test.bin: unexpected end of file\n");
				else if (ferror(dataFile)) {
					perror("Error reading test.bin");
				}
				assert(false && "File not read.");
			}

			fclose(dataFile);

			return data;
		}
	}
}

static mvGLTFModel
mvLoadBinaryGLTF(const char* root, const char* file)
{

	mvGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = (char*)mvImp::_ReadFile(file, dataSize, "rb");

	unsigned magic = *(unsigned*)&data[0];
	unsigned version = *(unsigned*)&data[4];
	unsigned length = *(unsigned*)&data[8];

	unsigned chunkLength = *(unsigned*)&data[12];
	unsigned chunkType = *(unsigned*)&data[16];
	char* chunkData = &data[20];

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	sJsonObject& rootObject = *ParseJSON(chunkData, chunkLength);

	if (rootObject.doesMemberExist("scene"))
	{
		for (int i = 0; i < rootObject.childCount; i++)
		{
			if (strcmp("scene", rootObject.children[i].name) == 0)
			{
				model.scene = std::stoi(rootObject.children[i].value);
			}
		}
	}

	model.scenes = mvImp::_LoadScenes(rootObject, model.scene_count);
	model.nodes = mvImp::_LoadNodes(rootObject, model.node_count);
	model.materials = mvImp::_LoadMaterials(rootObject, model.material_count);
	model.meshes = mvImp::_LoadMeshes(rootObject, model.mesh_count);
	model.textures = mvImp::_LoadTextures(rootObject, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(rootObject, model.sampler_count);
	model.images = mvImp::_LoadImages(rootObject, model.image_count);
	model.buffers = mvImp::_LoadBuffers(rootObject, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(rootObject, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(rootObject, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(rootObject, model.camera_count);
	model.animations = mvImp::_LoadAnimations(rootObject, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(rootObject, model.extension_count);
	model.skins = mvImp::_LoadSkins(rootObject, model.skin_count);

	if (chunkLength + 20 != length)
	{
		unsigned datachunkLength = *(unsigned*)&data[20 + chunkLength];
		unsigned datachunkType = *(unsigned*)&data[24 + chunkLength];
		char* datachunkData = &data[28 + chunkLength];

		model.buffers[0].dataCount = model.buffers[0].byte_length;
		model.buffers[0].data = new unsigned char[model.buffers[0].dataCount];
		memcpy(model.buffers[0].data, datachunkData, model.buffers[0].byte_length);
		int a = 6;
	}

	for (unsigned i = 0; i < model.image_count; i++)
	{
		mvGLTFImage& image = model.images[i];

		if (image.buffer_view_index > -1)
		{
			image.embedded = true;
			//image.data = model.buffers
			mvGLTFBufferView bufferView = model.bufferviews[image.buffer_view_index];
			char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data;
			char* bufferRawSection = &bufferRawData[bufferView.byte_offset]; // start of buffer section
			image.dataCount = bufferView.byte_length;
			image.data = new unsigned char[image.dataCount];
			memcpy(image.data, bufferRawSection, bufferView.byte_length);
			continue;
		}

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(image.data, mime_type, image.uri, 0))
			{
				assert(false && "here");
			}
		}
		else
		{
			image.embedded = false;
		}

	}

	for (unsigned i = 0; i < model.buffer_count; i++)
	{
		mvGLTFBuffer& buffer = model.buffers[i];

		if (buffer.data)
			continue;

		if (isDataURI(buffer.uri))
		{
			std::string mime_type;
			if (!DecodeDataURI(buffer.data, mime_type, buffer.uri, buffer.byte_length))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* bufferdata = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.dataCount = dataSize;
			buffer.data = new unsigned char[buffer.dataCount];
			memcpy(buffer.data, bufferdata, dataSize);
			delete[] bufferdata;
		}

	}

	delete[] data;
	return model;
}

mvGLTFModel
mvLoadGLTF(const char* root, const char* file)
{

	size_t len = strlen(file);
	if (file[len - 1] == 'b')
		return mvLoadBinaryGLTF(root, file);

	mvGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = mvImp::_ReadFile(file, dataSize, "rb");

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	sJsonObject& rootObject = *ParseJSON(data, dataSize);
	delete[] data;
	if(rootObject.doesMemberExist("scene"))
	{
		sJsonObject& sceneObject = *rootObject.getMember("scene");
		model.scene = atoi(sceneObject.value);
	}

	model.scenes = mvImp::_LoadScenes(rootObject, model.scene_count);
	model.nodes = mvImp::_LoadNodes(rootObject, model.node_count);
	model.materials = mvImp::_LoadMaterials(rootObject, model.material_count);
	model.meshes = mvImp::_LoadMeshes(rootObject, model.mesh_count);
	model.textures = mvImp::_LoadTextures(rootObject, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(rootObject, model.sampler_count);
	model.images = mvImp::_LoadImages(rootObject, model.image_count);
	model.buffers = mvImp::_LoadBuffers(rootObject, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(rootObject, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(rootObject, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(rootObject, model.camera_count);
	model.animations = mvImp::_LoadAnimations(rootObject, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(rootObject, model.extension_count);
	model.skins = mvImp::_LoadSkins(rootObject, model.skin_count);

	for (unsigned i = 0; i < model.image_count; i++)
	{
		mvGLTFImage& image = model.images[i];

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(image.data, mime_type, image.uri, 0))
			{
				assert(false && "here");
			}
		}
		else
		{
			image.embedded = false;
		}

	}

	for (unsigned i = 0; i < model.buffer_count; i++)
	{
		mvGLTFBuffer& buffer = model.buffers[i];

		if (isDataURI(buffer.uri))
		{
			std::string mime_type;
			if (!DecodeDataURI(buffer.data, mime_type, buffer.uri, buffer.byte_length))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* data = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.dataCount = dataSize;
			buffer.data = new unsigned char[buffer.dataCount];
			memcpy(buffer.data, data, dataSize);
		}

	}

	return model;
}

void
mvCleanupGLTF(mvGLTFModel& model)
{

	for (unsigned i = 0; i < model.mesh_count; i++)
	{
		for (unsigned j = 0; j < model.meshes[i].primitives_count; j++)
		{
			
			for (unsigned k = 0; k < model.meshes[i].primitives[j].target_count; k++)
			{
				delete[] model.meshes[i].primitives[j].targets[k].attributes;
			}
			delete[] model.meshes[i].primitives[j].targets;
			delete[] model.meshes[i].primitives[j].attributes;
		}
		delete[] model.meshes[i].weights;
		delete[] model.meshes[i].primitives;
	}


	for (unsigned i = 0; i < model.node_count; i++)
	{
		if (model.nodes[i].children)
			delete[] model.nodes[i].children;
	}

	for (unsigned i = 0; i < model.scene_count; i++)
	{
		if (model.scenes[i].nodes)
			delete[] model.scenes[i].nodes;
	}

	for (unsigned i = 0; i < model.animation_count; i++)
	{
		if (model.animations[i].sampler_count > 0)
			delete[] model.animations[i].samplers;
		if (model.animations[i].channel_count > 0)
			delete[] model.animations[i].channels;
	}

	for (unsigned i = 0; i < model.skin_count; i++)
	{
		if (model.skins[i].joints_count > 0)
			delete[] model.skins[i].joints;
	}

	delete[] model.scenes;
	delete[] model.nodes;
	delete[] model.meshes;
	delete[] model.materials;
	delete[] model.textures;
	delete[] model.samplers;
	delete[] model.images;
	delete[] model.buffers;
	delete[] model.bufferviews;
	delete[] model.accessors;
	delete[] model.cameras;
	delete[] model.animations;
	delete[] model.extensions;
	delete[] model.skins;

	model.scenes = nullptr;
	model.nodes = nullptr;
	model.meshes = nullptr;
	model.materials = nullptr;
	model.textures = nullptr;
	model.samplers = nullptr;
	model.images = nullptr;
	model.buffers = nullptr;
	model.bufferviews = nullptr;
	model.accessors = nullptr;
	model.cameras = nullptr;
	model.animations = nullptr;
	model.extensions = nullptr;
	model.skins = nullptr;

	model.scene_count = 0u;
	model.node_count = 0u;
	model.mesh_count = 0u;
	model.material_count = 0u;
	model.texture_count = 0u;
	model.sampler_count = 0u;
	model.image_count = 0u;
	model.buffer_count = 0u;
	model.bufferview_count = 0u;
	model.accessor_count = 0u;
	model.camera_count = 0u;
	model.animation_count = 0u;
	model.extension_count = 0u;
	model.skin_count = 0u;
}