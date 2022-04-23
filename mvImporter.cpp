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
		sJsonObject* janimations = j.getMember("animations");
		if(janimations == nullptr)
			return nullptr;
		size = janimations->childCount;

		mvGLTFAnimation* animations = new mvGLTFAnimation[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& janimation = (*janimations)[i];
			mvGLTFAnimation& animation = animations[i];
			animation.channel_count = 0u;
			animation.channels = nullptr;
			animation.sampler_count = 0u;
			animation.samplers = nullptr;
			animation.name = janimation.getStringMember("name", "");

			if (sJsonObject* jsamplers = janimation.getMember("samplers"))
			{
				animation.sampler_count = jsamplers->childCount;
				animation.samplers = new mvGLTFAnimationSampler[animation.sampler_count];

				for (int s = 0; s < animation.sampler_count; s++)
				{
					sJsonObject& jsampler = (*jsamplers)[s];
					mvGLTFAnimationSampler& sampler = animation.samplers[s];
					sampler.input = jsampler.getIntMember("input", -1);
					sampler.output = jsampler.getIntMember("output", -1);
					sampler.interpolation = jsampler.getStringMember("interpolation", "LINEAR");
				}
			}

			if (sJsonObject* jchannels = janimation.getMember("channels"))
			{
				animation.channel_count = jchannels->childCount;
				animation.channels = new mvGLTFAnimationChannel[animation.channel_count];

				for (int i = 0; i < animation.channel_count; i++)
				{
					sJsonObject& jchannel = (*jchannels)[i];
					mvGLTFAnimationChannel& channel = animation.channels[i];

					channel.sampler = jchannel.getIntMember("sampler", -1);

					if (sJsonObject* jtarget = jchannel.getMember("target"))
					{
						channel.target.node = jtarget->getIntMember("node", -1);
						channel.target.path = jtarget->getStringMember("path", "");
					}
				}
			}
		}
		return animations;
	}

	static mvGLTFCamera*
	_LoadCameras(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jcameras = j.getMember("cameras");
		if(jcameras == nullptr)
			return nullptr;
		size = jcameras->childCount;

		mvGLTFCamera* cameras = new mvGLTFCamera[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jcamera = (*jcameras)[i];
			mvGLTFCamera& camera = cameras[i];
			camera.name = jcamera.getStringMember("name", "");
			std::string type = jcamera.getStringMember("type", "perspective");

			if (type == "perspective") camera.type = MV_IMP_PERSPECTIVE;
			else                       camera.type = MV_IMP_ORTHOGRAPHIC;

			if (sJsonObject* jperspective = jcamera.getMember("perspective"))
			{
				camera.perspective.aspectRatio = jperspective->getFloatMember("aspectRatio", 0.0f);
				camera.perspective.yfov = jperspective->getFloatMember("yfov", 0.0f);
				camera.perspective.zfar = jperspective->getFloatMember("zfar", 0.0f);
				camera.perspective.znear = jperspective->getFloatMember("znear", 0.0f);
			}

			if (sJsonObject* jorthographic = jcamera.getMember("orthographic"))
			{
				camera.orthographic.xmag = jorthographic->getFloatMember("xmag", 0.0f);
				camera.orthographic.ymag = jorthographic->getFloatMember("ymag", 0.0f);
				camera.orthographic.zfar = jorthographic->getFloatMember("zfar", 0.0f);
				camera.orthographic.znear = jorthographic->getFloatMember("znear", 0.0f);
			}
		}
		return cameras;
	}

	static mvGLTFScene*
	_LoadScenes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jscenes = j.getMember("scenes");
		if(jscenes == nullptr)
			return nullptr;
		size = jscenes->childCount;
		mvGLTFScene* scenes = new mvGLTFScene[size];
		for (int i = 0; i < size; i++)
		{
			sJsonObject& jscene = (*jscenes)[0];
			mvGLTFScene& scene = scenes[i];
			scene.node_count = 0u;
			scene.nodes = nullptr;
			if (sJsonObject* jnodes = jscene.getMember("nodes"))
			{
				scene.node_count = jnodes->childCount;
				scene.nodes = new unsigned[scene.node_count];
				for (int j = 0; j < scene.node_count; j++)
				{
					int node = (*jnodes)[j].asInt();
					scene.nodes[j] = node;
				}
			}
		}
		return scenes;
	}

	static mvGLTFNode*
	_LoadNodes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jnodes = j.getMember("nodes");
		if(jnodes == nullptr)
			return nullptr;
		size = jnodes->childCount;
		mvGLTFNode* nodes = new mvGLTFNode[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jnode = (*jnodes)[i];
			mvGLTFNode& node = nodes[i];
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
			node.name = jnode.getStringMember("name", "");
			node.mesh_index = jnode.getIntMember("mesh", -1);
			node.camera_index = jnode.getIntMember("camera", -1);
			node.skin_index = jnode.getIntMember("skin", -1);
			jnode.getFloatArrayMember("translation", node.translation, 3);
			jnode.getFloatArrayMember("scale", node.scale, 3);
			jnode.getFloatArrayMember("rotation", node.rotation, 4);
			jnode.getFloatArrayMember("matrix", node.matrix, 16);
			node.hadMatrix = jnode.doesMemberExist("matrix");

			if (sJsonObject* jchildren = jnode.getMember("children"))
			{
				node.child_count = jchildren->childCount;
				node.children = new unsigned[node.child_count];
				for (int j = 0; j < node.child_count; j++)
				{
					unsigned child = (*jchildren)[j].asFloat();
					node.children[j] = child;
				}
			}
		}
		return nodes;
	}

	static mvGLTFMesh*
	_LoadMeshes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jmeshes = j.getMember("meshes");
		if(jmeshes == nullptr)
			return nullptr;
		size = jmeshes->childCount;
		mvGLTFMesh* meshes = new mvGLTFMesh[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jmesh = (*jmeshes)[i];
			mvGLTFMesh& mesh = meshes[i];
			mesh.primitives = nullptr;
			mesh.primitives_count = 0u;
			mesh.weights = nullptr;
			mesh.weights_count = 0u;
			mesh.name = jmesh.getStringMember("name", "");

			if (sJsonObject* jweights = jmesh.getMember("weights"))
			{
				mesh.weights_count = (*jweights).childCount;
				mesh.weights = new float[mesh.weights_count];
				for (int j = 0; j < mesh.weights_count; j++)
				{
					sJsonObject m = (*jweights)[j];
					mesh.weights[j] = m.asFloat();
				}
			}

			if (sJsonObject* jprimitives = jmesh.getMember("primitives"))
			{
				mesh.primitives_count = jprimitives->childCount;
				mesh.primitives = new mvGLTFMeshPrimitive[mesh.primitives_count];

				for (int j = 0; j < mesh.primitives_count; j++)
				{
					mvGLTFMeshPrimitive& primitive = mesh.primitives[j];
					sJsonObject jprimitive = (*jprimitives)[j];
					primitive.mode = MV_IMP_TRIANGLES;
					primitive.attributes = nullptr;
					primitive.attribute_count = 0u;
					primitive.targets = nullptr;
					primitive.target_count = 0u;
					primitive.indices_index = jprimitive.getIntMember("indices", -1);
					primitive.material_index = jprimitive.getIntMember("material", -1);

					if (sJsonObject* jattributes = jprimitive.getMember("attributes"))
					{
						primitive.attribute_count = jattributes->childCount;
						primitive.attributes = new mvGLTFAttribute[primitive.attribute_count];
						
						for (int k = 0; k < primitive.attribute_count; k++)
						{
							sJsonObject& m = (*jattributes)[k];
							memcpy(primitive.attributes[k].semantic, m.name, MV_IMPORTER_MAX_NAME_LENGTH);
							primitive.attributes[k].index = m.asInt();
						}
					}

					if (sJsonObject* jtargets = jprimitive.getMember("targets"))
					{
						primitive.target_count = jtargets->childCount;
						primitive.targets = new mvGLTFMorphTarget[primitive.target_count];

						for (int k = 0; k < primitive.target_count; k++)
						{
							mvGLTFMorphTarget& target = primitive.targets[k];
							sJsonObject& jtarget = (*jtargets)[k];
							
							target.attribute_count = jtarget.childCount;
							target.attributes = new mvGLTFAttribute[target.attribute_count];
							for (int x = 0; x < target.attribute_count; x++)
							{
								sJsonObject& jattribute = jtarget[x];
								memcpy(target.attributes[x].semantic, jattribute.name, MV_IMPORTER_MAX_NAME_LENGTH);
								target.attributes[x].index = jattribute.asInt();
							}
						}
					}
				}
			}
		}
		return meshes;
	}

	static mvGLTFMaterial*
	_LoadMaterials(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jmaterials = j.getMember("materials");
		if(jmaterials == nullptr)
			return nullptr;
		size = jmaterials->childCount;
		mvGLTFMaterial* materials = new mvGLTFMaterial[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jmaterial = (*jmaterials)[i];
			mvGLTFMaterial& material = materials[i];
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
	        material.clearcoat_factor = 0.0f;
	        material.clearcoat_roughness_factor = 0.0f;
			material.clearcoat_texture = -1;
			material.clearcoat_roughness_texture = -1;
			material.clearcoat_normal_texture = -1;
			material.base_color_texture = -1;
	        material.metallic_roughness_texture = -1;
	        material.normal_texture = -1;
	        material.occlusion_texture = -1;
	        material.emissive_texture = -1;

			jmaterial.getFloatArrayMember("emissiveFactor", material.emissive_factor, 3);	

			std::string alphaMode = jmaterial.getStringMember("alphaMode", "OPAQUE");
			if     (alphaMode == "OPAQUE") material.alphaMode = MV_ALPHA_MODE_OPAQUE;
			else if(alphaMode == "MASK")   material.alphaMode = MV_ALPHA_MODE_MASK;
			else                           material.alphaMode = MV_ALPHA_MODE_BLEND;

			if (sJsonObject* jrt = jmaterial.getMember("normalTexture"))
			{
				material.normal_texture = jrt->getIntMember("index", -1);
				material.normal_texture_scale = jrt->getFloatMember("scale", 1.0f);
			}

			if (sJsonObject* jot = jmaterial.getMember("occlusionTexture"))
			{
				material.occlusion_texture = jot->getIntMember("index", -1);
				material.occlusion_texture_strength = jot->getFloatMember("strength", 1.0f);
			}

			if (sJsonObject* jet = jmaterial.getMember("emissiveTexture"))
			{
				material.emissive_texture = jet->getIntMember("index", -1);
			}

			if (sJsonObject* jpbrmr = jmaterial.getMember("pbrMetallicRoughness"))
			{
				material.pbrMetallicRoughness = true;
				material.metallic_factor = jpbrmr->getFloatMember("metallicFactor", 1.0f);
				material.roughness_factor = jpbrmr->getFloatMember("roughnessFactor", 1.0f);
				jpbrmr->getFloatArrayMember("baseColorFactor", material.base_color_factor, 4);

				if (sJsonObject* jbaseColorTexture = jpbrmr->getMember("baseColorTexture"))
				{
					material.base_color_texture = jbaseColorTexture->getIntMember("index", -1);
				}
				
				if (sJsonObject* jmrt = jpbrmr->getMember("metallicRoughnessTexture"))
				{
					material.metallic_roughness_texture = jmrt->getIntMember("index", -1);
				}

			}

			if (sJsonObject* jextensions = jmaterial.getMember("extensions"))
			{
				if (sJsonObject* jKHR_cc = jextensions->getMember("KHR_materials_clearcoat"))
				{
					material.clearcoat_extension = true;
					material.clearcoat_factor = jKHR_cc->getFloatMember("clearcoatFactor", 0.0f);
					material.clearcoat_roughness_factor = jKHR_cc->getIntMember("clearcoatRoughnessFactor", 0.0f);

					if (sJsonObject* jcct = jKHR_cc->getMember("clearcoatTexture"))
					{
						material.clearcoat_texture = jcct->getIntMember("index", -1);
						material.clearcoat_normal_texture_scale = jcct->getFloatMember("scale", 1.0f);
					}
					if (sJsonObject* jccrt = jKHR_cc->getMember("clearcoatRoughnessTexture"))
					{
						material.clearcoat_roughness_texture = jccrt->getIntMember("index", -1);
					}
					if (sJsonObject* jccnt = jKHR_cc->getMember("clearcoatNormalTexture"))
					{
						material.clearcoat_normal_texture = jccnt->getIntMember("index", -1);
					}
				}
			}
		}
		return materials;
	}

	static mvGLTFTexture*
	_LoadTextures(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jtextures = j.getMember("textures");
		if(jtextures == nullptr)
			return nullptr;
		size = jtextures->childCount;
		mvGLTFTexture* textures = new mvGLTFTexture[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jtexture = (*jtextures)[i];
			mvGLTFTexture& texture = textures[i];
			texture.sampler_index = jtexture.getIntMember("sampler", -1);
			texture.image_index = jtexture.getIntMember("source", -1);
			texture.name = jtexture.getStringMember("name", "");
		}
		return textures;
	}

	static mvGLTFSampler*
	_LoadSamplers(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jsamplers = j.getMember("samplers");
		if(jsamplers == nullptr)
			return nullptr;
		size = jsamplers->childCount;
		mvGLTFSampler* samplers = new mvGLTFSampler[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jsampler = (*jsamplers)[i];
			mvGLTFSampler& sampler = samplers[i];
			sampler.name = jsampler.getStringMember("name", "");
			sampler.mag_filter = jsampler.getIntMember("magFilter", -1);
			sampler.min_filter = jsampler.getIntMember("minFilter", -1);
			sampler.wrap_s = jsampler.getIntMember("wrapS", MV_IMP_WRAP_REPEAT);
			sampler.wrap_t = jsampler.getIntMember("wrapT", MV_IMP_WRAP_REPEAT);
		}
		return samplers;
	}

	static mvGLTFImage*
	_LoadImages(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jimages = j.getMember("images");
		if(jimages == nullptr)
			return nullptr;
		size = jimages->childCount;
		mvGLTFImage* images = new mvGLTFImage[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jimage = (*jimages)[i];
			mvGLTFImage& image = images[i];
			image.uri = jimage.getStringMember("uri", "");
			image.mimeType = jimage.getStringMember("mimeType", "");
			image.buffer_view_index = jimage.getIntMember("bufferView", -1);
			image.data = nullptr;
			image.dataCount = 0u;
			image.embedded = false; // ?
		}
		return images;
	}

	static mvGLTFBuffer*
	_LoadBuffers(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jbuffers = j.getMember("buffers");
		if(jbuffers == nullptr)
			return nullptr;
		size = jbuffers->childCount;
		mvGLTFBuffer* buffers = new mvGLTFBuffer[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jbuffer = (*jbuffers)[i];
			mvGLTFBuffer& buffer = buffers[i];
			buffer.uri = jbuffer.getStringMember("uri", "");
			buffer.byte_length = jbuffer.getUIntMember("byteLength", 0u);
			buffer.data = nullptr;
			buffer.dataCount = 0u;
		}
		return buffers;
	}

	static mvGLTFBufferView*
	_LoadBufferViews(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jbufferViews = j.getMember("bufferViews");
		if(jbufferViews == nullptr)
			return nullptr;
		size = jbufferViews->childCount;
		mvGLTFBufferView* bufferviews = new mvGLTFBufferView[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jbufferview = (*jbufferViews)[i];
			mvGLTFBufferView& bufferview = bufferviews[i];
			bufferview.name = jbufferview.getStringMember("name", "");
			bufferview.buffer_index = jbufferview.getIntMember("buffer", -1);
			bufferview.byte_offset = jbufferview.getIntMember("byteOffset", 0);
			bufferview.byte_length = jbufferview.getIntMember("byteLength", -1);
			bufferview.byte_stride = jbufferview.getIntMember("byteStride", -1);
		}
		return bufferviews;
	}

	static mvGLTFAccessor*
	_LoadAccessors(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jaccessors = j.getMember("accessors");
		if(jaccessors == nullptr)
			return nullptr;
		size = jaccessors->childCount;
		mvGLTFAccessor* accessors = new mvGLTFAccessor[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jaccessor = (*jaccessors)[i];
			mvGLTFAccessor& accessor = accessors[i];
			accessor.name = jaccessor.getStringMember("name", "");
			accessor.byteOffset = jaccessor.getIntMember("byteOffset", 0);
			accessor.count = jaccessor.getIntMember("count", -1);
			accessor.component_type = (mvGLTFComponentType)jaccessor.getIntMember("componentType", MV_IMP_FLOAT);
			accessor.buffer_view_index = jaccessor.getIntMember("bufferView", -1);
			accessor.type = MV_IMP_SCALAR;
			jaccessor.getFloatArrayMember("max", accessor.maxes, 16);
			jaccessor.getFloatArrayMember("min", accessor.mins, 16);

			std::string accessorType = jaccessor.getStringMember("type", "SCALAR");
			if (accessorType == "SCALAR")    accessor.type = MV_IMP_SCALAR;
			else if (accessorType == "VEC2") accessor.type = MV_IMP_VEC2;
			else if (accessorType == "VEC3") accessor.type = MV_IMP_VEC3;
			else if (accessorType == "VEC4") accessor.type = MV_IMP_VEC4;
			else if (accessorType == "MAT2") accessor.type = MV_IMP_MAT2;
			else if (accessorType == "MAT3") accessor.type = MV_IMP_MAT3;
			else if (accessorType == "MAT4") accessor.type = MV_IMP_MAT4;
		}
		return accessors;
	}

	static mvGLTFSkin*
	_LoadSkins(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jskins = j.getMember("skins");
		if(jskins == nullptr)
			return nullptr;
		size = jskins->childCount;
		mvGLTFSkin* skins = new mvGLTFSkin[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jnode = (*jskins)[i];
			mvGLTFSkin& skin = skins[i];
			skin.name = jnode.getStringMember("name", "");
			skin.inverseBindMatrices = jnode.getIntMember("inverseBindMatrices", -1);
			skin.skeleton = jnode.getIntMember("skeleton", -1);
			skin.joints = nullptr;
			skin.joints_count = 0u;

			if (sJsonObject* jjoints = jnode.getMember("joints"))
			{
				skin.joints_count = jjoints->childCount;
				skin.joints = new unsigned[skin.joints_count];
				jjoints->asUIntArray(skin.joints, skin.joints_count);
			}
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