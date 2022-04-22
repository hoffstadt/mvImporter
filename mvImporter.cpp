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
DecodeDataURI(mvVector<unsigned char>* out, std::string& mime_type,
	const std::string& in, size_t reqBytes, bool checkSize)
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
	if (data.empty()) {
		return false;
	}

	if (checkSize) {
		if (data.size() != reqBytes) {
			return false;
		}
		out->resize(reqBytes);
	}
	else {
		out->resize(data.size());
	}
	std::copy(data.begin(), data.end(), out->begin());
	return true;
}

namespace mvImp {

	static std::string*
	_LoadExtensions(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("extensionsUsed"))
			return nullptr;

		unsigned extensionCount = j["extensionsUsed"].members.size;

		std::string* extensions = new std::string[extensionCount];

		for (int i = 0; i < extensionCount; i++)
		{
			//mvJsonObject& jExtension = j["extensionsUsed"][i];
			extensions[i] = j["extensionsUsed"][i];
			size++;
		}

		return extensions;
	}

	static mvGLTFAnimation*
	_LoadAnimations(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("animations"))
			return nullptr;

		unsigned animationCount = j["animations"].members.size;

		mvGLTFAnimation* animations = new mvGLTFAnimation[animationCount];

		for (int i = 0; i < animationCount; i++)
		{
			mvJsonObject& janimation = j["animations"][i];
			mvGLTFAnimation& animation = animations[i];

			if (janimation.doesMemberExist("name"))
			{
				animation.name = janimation.getMember("name");
			}

			if (janimation.doesMemberExist("samplers"))
			{
				unsigned samplerCount = janimation["samplers"].members.size;
				animation.samplers = new mvGLTFAnimationSampler[samplerCount];
				animation.sampler_count = samplerCount;

				for (int i = 0; i < samplerCount; i++)
				{
					mvJsonObject& jsampler = janimation["samplers"][i];
					mvGLTFAnimationSampler& sampler = animation.samplers[i];

					if (jsampler.doesMemberExist("input"))
					{
						sampler.input = jsampler.getMember("input");
					}

					if (jsampler.doesMemberExist("output"))
					{
						sampler.output = jsampler.getMember("output");
					}

					if (jsampler.doesMemberExist("interpolation"))
					{
						sampler.interpolation = jsampler.getMember("interpolation");
					}
				}
			}

			if (janimation.doesMemberExist("channels"))
			{
				unsigned channelCount = janimation["channels"].members.size;
				animation.channels = new mvGLTFAnimationChannel[channelCount];
				animation.channel_count = channelCount;

				for (int i = 0; i < channelCount; i++)
				{
					mvJsonObject& jchannel = janimation["channels"][i];
					mvGLTFAnimationChannel& channel = animation.channels[i];

					if (jchannel.doesMemberExist("sampler"))
					{
						channel.sampler = jchannel.getMember("sampler");
					}

					if (jchannel.doesMemberExist("target"))
					{

						mvJsonObject& jchanneltarget = jchannel.getMember("target");
						mvGLTFAnimationChannelTarget target{};

						if (jchanneltarget.doesMemberExist("node"))
						{
							channel.target.node = jchanneltarget.getMember("node");
						}

						if (jchanneltarget.doesMemberExist("path"))
						{
							channel.target.path = jchanneltarget.getMember("path");
						}

					}
				}
			}

			size++;
		}

		return animations;
	}

	static mvGLTFCamera*
	_LoadCameras(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("cameras"))
			return nullptr;

		unsigned cameraCount = j["cameras"].members.size;

		mvGLTFCamera* cameras = new mvGLTFCamera[cameraCount];

		for (int i = 0; i < cameraCount; i++)
		{
			mvJsonObject& jcamera = j["cameras"][i];
			mvGLTFCamera& camera = cameras[i];

			if (jcamera.doesMemberExist("name"))
			{
				camera.name = jcamera.getMember("name");
			}

			if (jcamera.doesMemberExist("type"))
			{

				std::string type = jcamera.getMember("type");
				if (type == "perspective")
				{
					camera.type = MV_IMP_PERSPECTIVE;
				}
				else
				{
					camera.type = MV_IMP_ORTHOGRAPHIC;
				}
			}

			if (jcamera.doesMemberExist("perspective"))
			{

				mvJsonObject perspective = jcamera["perspective"];

				if (perspective.doesMemberExist("aspectRatio"))
					camera.perspective.aspectRatio = perspective.getMember("aspectRatio");

				if (perspective.doesMemberExist("yfov"))
					camera.perspective.yfov = perspective.getMember("yfov");

				if (perspective.doesMemberExist("zfar"))
					camera.perspective.zfar = perspective.getMember("zfar");

				if (perspective.doesMemberExist("znear"))
					camera.perspective.znear = perspective.getMember("znear");

			}

			if (jcamera.doesMemberExist("orthographic"))
			{

				mvJsonObject orthographic = jcamera["orthographic"];

				if (orthographic.doesMemberExist("xmag"))
					camera.orthographic.xmag = orthographic.getMember("xmag");

				if (orthographic.doesMemberExist("ymag"))
					camera.orthographic.ymag = orthographic.getMember("ymag");

				if (orthographic.doesMemberExist("zfar"))
					camera.orthographic.zfar = orthographic.getMember("zfar");

				if (orthographic.doesMemberExist("znear"))
					camera.orthographic.znear = orthographic.getMember("znear");

			}

			size++;
		}

		return cameras;
	}

	static mvGLTFScene*
	_LoadScenes(mvJsonDocument& j, unsigned& size)
	{

		if (!j.doesMemberExist("scenes"))
			return nullptr;

		unsigned count = j["scenes"].members.size;
		mvGLTFScene* scenes = new mvGLTFScene[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jscene = j["scenes"][0];
			mvGLTFScene& scene = scenes[size];

			if (jscene.doesMemberExist("nodes"))
			{
				unsigned nodeCount = jscene["nodes"].members.size;
				scene.nodes = new unsigned[nodeCount];
				for (int j = 0; j < nodeCount; j++)
				{
					int node = jscene["nodes"][j];
					scene.nodes[scene.node_count] = node;
					scene.node_count++;
				}
			}

			size++;
		}

		return scenes;
	}

	static mvGLTFNode*
	_LoadNodes(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("nodes"))
			return nullptr;

		unsigned count = j["nodes"].members.size;
		mvGLTFNode* nodes = new mvGLTFNode[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jnode = j["nodes"][i];
			mvGLTFNode& node = nodes[size];

			if (jnode.doesMemberExist("name"))
				node.name = jnode.getMember("name");

			if (jnode.doesMemberExist("mesh"))
				node.mesh_index = jnode.getMember("mesh");

			if (jnode.doesMemberExist("camera"))
				node.camera_index = jnode.getMember("camera");

			if (jnode.doesMemberExist("skin"))
				node.skin_index = jnode.getMember("skin");

			if (jnode.doesMemberExist("children"))
			{
				unsigned childCount = jnode["children"].members.size;
				node.children = new unsigned[childCount];

				for (int j = 0; j < childCount; j++)
				{
					unsigned child = jnode["children"][j];
					node.children[node.child_count] = child;
					node.child_count++;
				}
			}

			if (jnode.doesMemberExist("translation"))
			{
				unsigned compCount = jnode["translation"].members.size;

				for (int j = 0; j < compCount; j++)
				{
					node.translation[j] = jnode["translation"][j];
				}
			}

			if (jnode.doesMemberExist("scale"))
			{
				unsigned compCount = jnode["scale"].members.size;

				for (int j = 0; j < compCount; j++)
				{
					node.scale[j] = jnode["scale"][j];
				}
			}

			if (jnode.doesMemberExist("rotation"))
			{
				unsigned compCount = jnode["rotation"].members.size;

				for (int j = 0; j < compCount; j++)
				{
					node.rotation[j] = jnode["rotation"][j];
				}
			}

			if (jnode.doesMemberExist("matrix"))
			{
				node.hadMatrix = true;
				unsigned compCount = jnode["matrix"].members.size;

				for (int j = 0; j < compCount; j++)
				{
					node.matrix[j] = jnode["matrix"][j];
				}
			}

			size++;
		}

		return nodes;
	}

	static mvGLTFMesh*
	_LoadMeshes(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("meshes"))
			return nullptr;

		unsigned meshCount = j["meshes"].members.size;

		mvGLTFMesh* meshes = new mvGLTFMesh[meshCount];

		for (int i = 0; i < meshCount; i++)
		{
			mvJsonObject& jmesh = j["meshes"][i];
			mvGLTFMesh& mesh = meshes[i];

			if (jmesh.doesMemberExist("name"))
				mesh.name = jmesh.getMember("name");

			if (jmesh.doesMemberExist("weights"))
			{
				mesh.weights_count = jmesh["weights"].members.size;
				mesh.weights = new float[mesh.weights_count];
				for (int j = 0; j < mesh.weights_count; j++)
				{
					mvJsonMember m = jmesh["weights"][j];
					//mvGLTFMeshPrimitive& primitive = mesh.primitives[j];
					mesh.weights[j] = m;
				}
			}

			if (jmesh.doesMemberExist("primitives"))
			{

				mesh.primitives_count = jmesh["primitives"].members.size;
				mesh.primitives = new mvGLTFMeshPrimitive[mesh.primitives_count];

				for (int j = 0; j < mesh.primitives_count; j++)
				{

					mvGLTFMeshPrimitive& primitive = mesh.primitives[j];

					mvJsonObject jprimitive = jmesh["primitives"][j];

					if (jprimitive.doesMemberExist("indices"))
						primitive.indices_index = jprimitive.getMember("indices");

					if (jprimitive.doesMemberExist("material"))
						primitive.material_index = jprimitive.getMember("material");

					if (jprimitive.doesMemberExist("attributes"))
					{
						mvJsonObject jattributes = jprimitive["attributes"];
						unsigned attrCount = jattributes.members.size;
						primitive.attributes = new mvGLTFAttribute[attrCount];
						

						for (int k = 0; k < attrCount; k++)
						{
							mvJsonMember m = jattributes.members[k];
							
							//primitive.attributes[primitive.attribute_count] = { m.name , (int)m };
							//primitive.attributes[primitive.attribute_count].semantic = m.name;
							memcpy(primitive.attributes[primitive.attribute_count].semantic, m.name, MV_IMPORTER_MAX_NAME_LENGTH);
							primitive.attributes[primitive.attribute_count].index = (int)m;
							primitive.attribute_count++;
						}

					}

					if (jprimitive.doesMemberExist("targets"))
					{
						mvJsonObject jtargets = jprimitive["targets"];
						unsigned targetCount = jtargets.members.size;
						primitive.targets = new mvGLTFMorphTarget[targetCount];

						for (int k = 0; k < targetCount; k++)
						{
							mvGLTFMorphTarget& target = primitive.targets[k];

							mvJsonObject jtarget = jtargets.members[k];
							
							unsigned attrCount = jtarget.members.size;
							target.attributes = new mvGLTFAttribute[attrCount];
							for (int x = 0; x < attrCount; x++)
							{
								mvJsonMember jattribute = jtarget.members[x];
								memcpy(target.attributes[target.attribute_count].semantic, jattribute.name, MV_IMPORTER_MAX_NAME_LENGTH);
								target.attributes[target.attribute_count].index = (int)jattribute;
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
	_LoadMaterials(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("materials"))
			return nullptr;

		unsigned count = j["materials"].members.size;
		mvGLTFMaterial* materials = new mvGLTFMaterial[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jmaterial = j["materials"][i];
			mvGLTFMaterial& material = materials[size];

			if (jmaterial.doesMemberExist("name"))
				material.name = jmaterial.getMember("name");

			if (jmaterial.doesMemberExist("alphaMode"))
			{
				std::string alphaMode = jmaterial.getMember("alphaMode");
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

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["baseColorTexture"].doesMemberExist("index"))
						material.base_color_texture = jmaterial["pbrMetallicRoughness"]["baseColorTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0];
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1];
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2];
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3];
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].doesMemberExist("index"))
						material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicFactor"))
					material.metallic_factor = jmaterial["pbrMetallicRoughness"].getMember("metallicFactor");

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("roughnessFactor"))
					material.roughness_factor = jmaterial["pbrMetallicRoughness"].getMember("roughnessFactor");
			}

			if (jmaterial.doesMemberExist("normalTexture"))
			{
				if (jmaterial["normalTexture"].doesMemberExist("index"))
					material.normal_texture = jmaterial["normalTexture"].getMember("index");

				if (jmaterial["normalTexture"].doesMemberExist("scale"))
					material.normal_texture_scale = jmaterial["normalTexture"].getMember("scale");
			}

			if (jmaterial.doesMemberExist("occlusionTexture"))
			{
				if (jmaterial["occlusionTexture"].doesMemberExist("index"))
					material.occlusion_texture = jmaterial["occlusionTexture"].getMember("index");

				if (jmaterial["occlusionTexture"].doesMemberExist("scale"))
					material.occlusion_texture_strength = jmaterial["occlusionTexture"].getMember("strength");
			}

			if (jmaterial.doesMemberExist("emissiveTexture"))
			{
				if (jmaterial["emissiveTexture"].doesMemberExist("index"))
					material.emissive_texture = jmaterial["emissiveTexture"].getMember("index");
			}

			if (jmaterial.doesMemberExist("emissiveFactor"))
			{

				material.emissive_factor[0] = jmaterial["emissiveFactor"][0];
				material.emissive_factor[1] = jmaterial["emissiveFactor"][1];
				material.emissive_factor[2] = jmaterial["emissiveFactor"][2];
			}

			if (jmaterial.doesMemberExist("doubleSided"))
				material.double_sided = ((std::string)jmaterial.getMember("doubleSided")).data()[0] == 't';

			if (jmaterial.doesMemberExist("alphaCutoff"))
				material.alphaCutoff = jmaterial.getMember("alphaCutoff");

			if (jmaterial.doesMemberExist("extensions"))
			{
				if (jmaterial["extensions"].doesMemberExist("KHR_materials_clearcoat"))
				{
					material.clearcoat_extension = true;

					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatFactor"))
						material.clearcoat_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getMember("clearcoatFactor");
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatRoughnessFactor"))
						material.clearcoat_roughness_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getMember("clearcoatRoughnessFactor");
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].doesMemberExist("index"))
							material.clearcoat_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getMember("index");
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].doesMemberExist("scale"))
							material.clearcoat_normal_texture_scale = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getMember("scale");
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatRoughnessTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatRoughnessTexture"].doesMemberExist("index"))
							material.clearcoat_roughness_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatRoughnessTexture"].getMember("index");
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatNormalTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatNormalTexture"].doesMemberExist("index"))
							material.clearcoat_normal_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatNormalTexture"].getMember("index");
					}
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0];
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1];
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2];
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3];
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].doesMemberExist("index"))
						material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicFactor"))
					material.metallic_factor = jmaterial["pbrMetallicRoughness"].getMember("metallicFactor");

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("roughnessFactor"))
					material.roughness_factor = jmaterial["pbrMetallicRoughness"].getMember("roughnessFactor");
			}

			size++;
		}

		return materials;
	}

	static mvGLTFTexture*
	_LoadTextures(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("textures"))
			return nullptr;

		unsigned count = j["textures"].members.size;
		mvGLTFTexture* textures = new mvGLTFTexture[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jtexture = j["textures"][i];
			mvGLTFTexture& texture = textures[size];

			if (jtexture.doesMemberExist("sampler"))
				texture.sampler_index = jtexture.getMember("sampler");

			if (jtexture.doesMemberExist("source"))
				texture.image_index = jtexture.getMember("source");

			if (jtexture.doesMemberExist("name"))
				texture.name = jtexture.getMember("name");

			size++;
		}

		return textures;
	}

	static mvGLTFSampler*
	_LoadSamplers(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("samplers"))
			return nullptr;

		unsigned count = j["samplers"].members.size;
		mvGLTFSampler* samplers = new mvGLTFSampler[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jsampler = j["samplers"][i];
			mvGLTFSampler& sampler = samplers[size];

			if (jsampler.doesMemberExist("name"))
				sampler.name = jsampler.getMember("name");

			if (jsampler.doesMemberExist("magFilter"))
				sampler.mag_filter = jsampler.getMember("magFilter");

			if (jsampler.doesMemberExist("minFilter"))
				sampler.min_filter = jsampler.getMember("minFilter");

			if (jsampler.doesMemberExist("wrapS"))
				sampler.wrap_s = jsampler.getMember("wrapS");

			if (jsampler.doesMemberExist("wrapT"))
				sampler.wrap_t = jsampler.getMember("wrapT");

			size++;
		}

		return samplers;
	}

	static mvGLTFImage*
	_LoadImages(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("images"))
			return nullptr;

		unsigned count = j["images"].members.size;
		mvGLTFImage* images = new mvGLTFImage[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jimage = j["images"][i];
			mvGLTFImage& image = images[size];

			if (jimage.doesMemberExist("uri"))
				image.uri = jimage.getMember("uri");

			if (jimage.doesMemberExist("mimeType"))
				image.mimeType = jimage.getMember("mimeType");

			if (jimage.doesMemberExist("bufferView"))
				image.buffer_view_index = jimage.getMember("bufferView");

			size++;
		}

		return images;
	}

	static mvGLTFBuffer*
	_LoadBuffers(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("buffers"))
			return nullptr;

		unsigned count = j["buffers"].members.size;
		mvGLTFBuffer* buffers = new mvGLTFBuffer[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jbuffer = j["buffers"][i];
			mvGLTFBuffer& buffer = buffers[size];

			if (jbuffer.doesMemberExist("uri"))
				buffer.uri = jbuffer.getMember("uri");

			if (jbuffer.doesMemberExist("byteLength"))
				buffer.byte_length = (int)jbuffer.getMember("byteLength");

			size++;
		}

		return buffers;
	}

	static mvGLTFBufferView*
	_LoadBufferViews(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("bufferViews"))
			return nullptr;

		unsigned count = j["bufferViews"].members.size;
		mvGLTFBufferView* bufferviews = new mvGLTFBufferView[count];

		for (int i = 0; i < count; i++)
		{

			mvJsonObject& jbufferview = j["bufferViews"][i];
			mvGLTFBufferView& bufferview = bufferviews[size];

			if (jbufferview.doesMemberExist("name"))
				bufferview.name = jbufferview.getMember("name");

			if (jbufferview.doesMemberExist("buffer"))
				bufferview.buffer_index = jbufferview.getMember("buffer");

			if (jbufferview.doesMemberExist("byteOffset"))
				bufferview.byte_offset = jbufferview.getMember("byteOffset");

			if (jbufferview.doesMemberExist("byteLength"))
				bufferview.byte_length = jbufferview.getMember("byteLength");

			if (jbufferview.doesMemberExist("byteStride"))
				bufferview.byte_stride = jbufferview.getMember("byteStride");

			size++;
		}

		return bufferviews;
	}

	static mvGLTFAccessor*
	_LoadAccessors(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("accessors"))
			return nullptr;

		unsigned count = j["accessors"].members.size;
		mvGLTFAccessor* accessors = new mvGLTFAccessor[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jaccessor = j["accessors"][i];
			mvGLTFAccessor& accessor = accessors[i];

			if (jaccessor.doesMemberExist("name"))
			{
				//strcpy(accessor.name, jaccessor.getMember("name"));
				accessor.name = jaccessor.getMember("name");
			}

			if (jaccessor.doesMemberExist("byteOffset"))
				accessor.byteOffset = jaccessor.getMember("byteOffset");

			if (jaccessor.doesMemberExist("count"))
				accessor.count = jaccessor.getMember("count");

			if (jaccessor.doesMemberExist("componentType"))
				accessor.component_type = (mvGLTFComponentType)(int)jaccessor.getMember("componentType");

			if (jaccessor.doesMemberExist("bufferView"))
				accessor.buffer_view_index = jaccessor.getMember("bufferView");

			if (jaccessor.doesMemberExist("type"))
			{
				std::string strtype = jaccessor.getMember("type");
				if (strtype == "SCALAR")
					accessor.type = MV_IMP_SCALAR;

				else if (strtype == "VEC2")
					accessor.type = MV_IMP_VEC2;

				else if (strtype == "VEC3")
					accessor.type = MV_IMP_VEC3;

				else if (strtype == "VEC4")
					accessor.type = MV_IMP_VEC4;

				else if (strtype == "MAT2")
					accessor.type = MV_IMP_MAT2;

				else if (strtype == "MAT3")
					accessor.type = MV_IMP_MAT3;

				else if (strtype == "MAT4")
					accessor.type = MV_IMP_MAT4;
			}

			if (jaccessor.doesMemberExist("max"))
			{

				unsigned min_count = jaccessor["max"].members.size;
				for (unsigned min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.maxes[min_entry] = jaccessor["max"][min_entry];
				}
			}

			if (jaccessor.doesMemberExist("min"))
			{

				unsigned min_count = jaccessor["min"].members.size;
				for (unsigned min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.mins[min_entry] = jaccessor["min"][min_entry];
				}
			}

			size++;
		}

		return accessors;
	}

	static mvGLTFSkin*
	_LoadSkins(mvJsonDocument& j, unsigned& size)
	{
		if (!j.doesMemberExist("skins"))
			return nullptr;

		unsigned count = j["skins"].members.size;
		mvGLTFSkin* skins = new mvGLTFSkin[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jnode = j["skins"][i];
			mvGLTFSkin& skin = skins[size];

			if (jnode.doesMemberExist("name"))
				skin.name = jnode.getMember("name");

			if (jnode.doesMemberExist("inverseBindMatrices"))
				skin.inverseBindMatrices = jnode.getMember("inverseBindMatrices");

			if (jnode.doesMemberExist("skeleton"))
				skin.skeleton = jnode.getMember("skeleton");

			if (jnode.doesMemberExist("joints"))
			{
				unsigned childCount = jnode["joints"].members.size;
				skin.joints = new unsigned[childCount];

				for (int j = 0; j < childCount; j++)
				{
					unsigned child = jnode["joints"][j];
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

	mvJsonDocument& context = *ParseJSON(chunkData, chunkLength);

	if (context.doesMemberExist("scene"))
	{
		for (int i = 0; i < context.jsonObjects[1].members.size; i++)
		{
			if (strcmp("scene", context.jsonObjects[1].members[i].name) == 0)
			{
				int index = context.jsonObjects[1].members[i].index;
				mvJsonValue value = context.primitiveValues[index];
				model.scene = std::stoi(value.value.data);
			}
		}
	}

	model.scenes = mvImp::_LoadScenes(context, model.scene_count);
	model.nodes = mvImp::_LoadNodes(context, model.node_count);
	model.materials = mvImp::_LoadMaterials(context, model.material_count);
	model.meshes = mvImp::_LoadMeshes(context, model.mesh_count);
	model.textures = mvImp::_LoadTextures(context, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(context, model.sampler_count);
	model.images = mvImp::_LoadImages(context, model.image_count);
	model.buffers = mvImp::_LoadBuffers(context, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(context, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(context, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(context, model.camera_count);
	model.animations = mvImp::_LoadAnimations(context, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(context, model.extension_count);
	model.skins = mvImp::_LoadSkins(context, model.skin_count);

	if (chunkLength + 20 != length)
	{
		unsigned datachunkLength = *(unsigned*)&data[20 + chunkLength];
		unsigned datachunkType = *(unsigned*)&data[24 + chunkLength];
		char* datachunkData = &data[28 + chunkLength];

		model.buffers[0].data.resize(model.buffers[0].byte_length);
		memcpy(model.buffers[0].data.data, datachunkData, model.buffers[0].byte_length);
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
			char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data.data;
			char* bufferRawSection = &bufferRawData[bufferView.byte_offset]; // start of buffer section
			image.data.resize(bufferView.byte_length);
			memcpy(image.data.data, bufferRawSection, bufferView.byte_length);
			continue;
		}

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(&image.data, mime_type, image.uri, 0, false))
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

		if (!buffer.data.empty())
			continue;

		if (isDataURI(buffer.uri))
		{
			std::string mime_type;
			if (!DecodeDataURI(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true))
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
			buffer.data.resize(dataSize);
			memcpy(buffer.data.data, data, dataSize);
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

	mvJsonDocument& context = *ParseJSON(data, dataSize);
	delete[] data;
	
	if (context.doesMemberExist("scene"))
	{
		for (int i = 0; i < context.jsonObjects[1].members.size; i++)
		{
			if (strcmp("scene", context.jsonObjects[1].members[i].name) == 0)
			{
				int index = context.jsonObjects[1].members[i].index;
				mvJsonValue value = context.primitiveValues[index];
				model.scene = std::stoi(value.value.data);
			}
		}
	}



	model.scenes = mvImp::_LoadScenes(context, model.scene_count);
	model.nodes = mvImp::_LoadNodes(context, model.node_count);
	model.materials = mvImp::_LoadMaterials(context, model.material_count);
	model.meshes = mvImp::_LoadMeshes(context, model.mesh_count);
	model.textures = mvImp::_LoadTextures(context, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(context, model.sampler_count);
	model.images = mvImp::_LoadImages(context, model.image_count);
	model.buffers = mvImp::_LoadBuffers(context, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(context, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(context, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(context, model.camera_count);
	model.animations = mvImp::_LoadAnimations(context, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(context, model.extension_count);
	model.skins = mvImp::_LoadSkins(context, model.skin_count);

	for (unsigned i = 0; i < model.image_count; i++)
	{
		mvGLTFImage& image = model.images[i];

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(&image.data, mime_type, image.uri, 0, false))
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
			if (!DecodeDataURI(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true))
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
			buffer.data.resize(dataSize);
			memcpy(buffer.data.data, data, dataSize);
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