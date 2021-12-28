#include "mvTextures.h"
#include "mvSandbox.h"
#include "mvMath.h"
#include <filesystem>
#include <cmath>
#include "mvPipeline.h"
#include "mvBuffers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

mvTexture
create_texture(std::vector<unsigned char> data)
{
	mvTexture texture{};

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

	if (stbi_is_hdr_from_memory(data.data(), data.size()))
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_hdr_to_ldr_scale(gamma_scale);
	}
	else
	{
		stbi_ldr_to_hdr_gamma(gamma);
		stbi_ldr_to_hdr_scale(gamma_scale);
	}

	// Load Image
	i32 texWidth, texHeight, texNumChannels;
	i32 texForceNumChannels = 4;
	unsigned char* testTextureBytes = stbi_load_from_memory(data.data(), data.size(), &texWidth, &texHeight,
		&texNumChannels, texForceNumChannels);
	assert(testTextureBytes);
	

	if (texNumChannels > 3)
		texture.alpha = true;

	//i32 texWidth, texHeight, texNumChannels;
	//stbi_info_from_memory(data.data(), data.size(), &texWidth, &texHeight, &texNumChannels);
	//if (texNumChannels > 3)
	//	texture.alpha = true;
	i32 texBytesPerRow = texForceNumChannels * texWidth;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, texture.texture.GetAddressOf());
	GContext->graphics.imDeviceContext->UpdateSubresource(texture.texture.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	GContext->graphics.device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.textureView.GetAddressOf());
	GContext->graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

	free(testTextureBytes);

	return texture;
}

mvTexture
create_texture(const std::string& path)
{
    mvTexture texture{};

    std::filesystem::path fpath = path;

    if (!std::filesystem::exists(path))
    {
        assert(false && "File not found.");
        return texture;
    }

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

	if (stbi_is_hdr(path.c_str()))
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_hdr_to_ldr_scale(gamma_scale);
	}
	else
	{
		stbi_ldr_to_hdr_gamma(gamma);
		stbi_ldr_to_hdr_scale(gamma_scale);
	}

    // Load Image
    i32 texWidth, texHeight, texNumChannels;
    i32 texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load(path.c_str(), &texWidth, &texHeight,
        &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    i32 texBytesPerRow = 4 * texWidth;

    if (texNumChannels > 3)
        texture.alpha = true;

    // Create Texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = texWidth;
    textureDesc.Height = texHeight;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, texture.texture.GetAddressOf());
    GContext->graphics.imDeviceContext->UpdateSubresource(texture.texture.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    GContext->graphics.device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.textureView.GetAddressOf());
    GContext->graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

    free(testTextureBytes);

    return texture;
}

mvCubeTexture
create_cube_texture(const std::string& path)
{
	mvCubeTexture texture{};

	// Load Image
	int texWidth, texHeight, texNumChannels, textBytesPerRow;

	// load 6 surfaces for cube faces
	unsigned char* surfaces[6];
	int texForceNumChannels = 4;

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

	// right
	{
		std::string file = path + "\\right.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[0] = testTextureBytes;
	}

	// left
	{
		std::string file = path + "\\left.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[1] = testTextureBytes;
	}

	// top
	{
		std::string file = path + "\\top.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[2] = testTextureBytes;
	}

	// bottom
	{
		std::string file = path + "\\bottom.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[3] = testTextureBytes;
	}

	// front
	{
		std::string file = path + "\\front.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[4] = testTextureBytes;
	}

	// back
	{
		std::string file = path + "\\back.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[5] = testTextureBytes;
	}

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = textBytesPerRow;
		data[i].SysMemSlicePitch = 0;
	}
	// create the texture resource
	mvComPtr<ID3D11Texture2D> pTexture;
	GContext->graphics.device->CreateTexture2D(&textureDesc, data, &pTexture);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &texture.textureView);

	return texture;
}

mvCubeTexture
create_cube_map(const std::string& path)
{
	mvCubeTexture texture{};

	mvVec4* surfaces[6];

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

	if (stbi_is_hdr(path.c_str()))
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_hdr_to_ldr_scale(gamma_scale);
	}
	else
	{
		stbi_ldr_to_hdr_gamma(gamma);
		stbi_ldr_to_hdr_scale(gamma_scale);
	}

	// Load Image
	i32 texWidth, texHeight, texNumChannels;
	i32 texForceNumChannels = 4;
	float* testTextureBytes = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texNumChannels, texForceNumChannels);
	assert(testTextureBytes);

	i32 pixels = texWidth * texHeight * 4 * 4;

	mvVec2 inUV = { 0.0f, 0.0f };
	i32 currentPixel = 0;

	i32 res = 512;
	f32 xinc = 1.0f / (f32)res;
	f32 yinc = 1.0f / (f32)res;
	for (int i = 0; i < 6; i++)
		surfaces[i] = new mvVec4[res * res];

	mvComputeShader shader = create_compute_shader(GContext->IO.shaderDirectory + "panorama_to_cube.hlsl");
	mvBuffer inputBuffer = create_buffer(testTextureBytes, texWidth * texHeight * 4 * sizeof(float), (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(surfaces[i], res * res * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		i32 resolution;
		u32 width;
		u32 height;
		u32 padding;
	};

	MetaData mdata{};
	mdata.resolution = res;
	mdata.width = texWidth;
	mdata.height = texHeight;

	mvConstBuffer cbuffer = create_const_buffer(&mdata, sizeof(MetaData));
	update_const_buffer(cbuffer, &mdata);
	ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, &cbuffer.buffer);
	ctx->CSSetUnorderedAccessViews(0u, 1u, &inputBuffer.unorderedAccessView, nullptr);
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i + 1u, 1u, &faces[i].unorderedAccessView, nullptr);

	ctx->CSSetShader(shader.shader, nullptr, 0);
	ctx->Dispatch(res / 16, res / 16, 2u);

	for (int i = 0; i < 6; i++)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		faces[i].buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;


		HRESULT hResult = GContext->graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		GContext->graphics.imDeviceContext->CopyResource(stagingBuffer, faces[i].buffer);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		GContext->graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);

		memcpy(surfaces[i], MappedResource.pData, res * res * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = res;
	textureDesc.Height = res;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = res * 4 * 4;
		data[i].SysMemSlicePitch = 0;
	}
	// create the texture resource
	mvComPtr<ID3D11Texture2D> pTexture;
	GContext->graphics.device->CreateTexture2D(&textureDesc, data, &pTexture);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &texture.textureView);
	//GContext->graphics.imDeviceContext->GenerateMips(texture.textureView);


	inputBuffer.buffer->Release();
	inputBuffer.unorderedAccessView->Release();
	cbuffer.buffer->Release();


	for (int i = 0; i < 6; i++)
	{
		faces[i].buffer->Release();
		faces[i].unorderedAccessView->Release();
	}
	shader.shader->Release();

	for (int i = 0; i < 6; i++)
		delete[] surfaces[i];
	return texture;

}

mvCubeTexture
create_environment_map(const std::string& path, mvCubeTexture& cubemap)
{

	int miplevels = 1;

	mvCubeTexture texture{};

	mvVec4** surfaces = new mvVec4*[6 * miplevels];

	// Load Image
	i32 texWidth, texHeight, texNumChannels;
	i32 texForceNumChannels = 4;
	stbi_info(path.c_str(), &texWidth, &texHeight, &texNumChannels);

	i32 pixels = texWidth * texHeight * 4 * 4;

	mvVec2 inUV = { 0.0f, 0.0f };
	i32 currentPixel = 0;

	i32 res = 1024;
	f32 xinc = 1.0f / (f32)res;
	f32 yinc = 1.0f / (f32)res;
	for (int i = 0; i < 6*miplevels; i++)
		surfaces[i] = new mvVec4[res * res];

	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(surfaces[i], res * res * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		i32 resolution;
		u32 width;
		u32 height;
		f32 roughness;

		u32 sampleCount;
		u32 currentMipLevel;
		f32 lodBias;
		u32 distribution;
	};

	uint32_t maxMipLevels = 0u;
	for (uint32_t m = res; m > 0; m = m >> 1, ++maxMipLevels) {}

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = res;
	textureDesc.Height = res;
	textureDesc.MipLevels = miplevels;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// filtering
	mvComputeShader filtershader = create_compute_shader(GContext->IO.shaderDirectory + "filter_environment.hlsl");
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = 0.0;
	samplerDesc.MaxLOD = maxMipLevels+1;
	ID3D11SamplerState* sampler;
	GContext->graphics.device->CreateSamplerState(&samplerDesc, &sampler);

	MetaData mdata{};
	mdata.resolution = res;
	mdata.width = res;
	mdata.height = res;
	mdata.sampleCount = 1024;
	mdata.distribution = 0;
	mdata.roughness = 0.0f;
	mdata.currentMipLevel = 0;
	mdata.lodBias = 1.0;

	mvConstBuffer cbuffer = create_const_buffer(&mdata, sizeof(MetaData));
	update_const_buffer(cbuffer, &mdata);
	ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, &cbuffer.buffer);
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i, 1u, &faces[i].unorderedAccessView, nullptr);
	ctx->CSSetShaderResources(0u, 1, &cubemap.textureView);
	ctx->CSSetSamplers(0u, 1, &sampler);
	ctx->CSSetShader(filtershader.shader, nullptr, 0);

	for (int level = miplevels - 1; level != -1; level--)
	{
		mdata.roughness = (f32)level / (f32)(miplevels - 1);
		mdata.currentMipLevel = level;
		//mdata.resolution = res >> level;
		update_const_buffer(cbuffer, &mdata);
		ctx->Dispatch(res / 16, res / 16, 2u);

		for (int i = level*6; i < level*6+6; i++)
		{
			ID3D11Buffer* stagingBuffer;

			D3D11_BUFFER_DESC cbd;
			faces[i%6].buffer->GetDesc(&cbd);
			cbd.BindFlags = 0;
			cbd.MiscFlags = 0;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			cbd.Usage = D3D11_USAGE_STAGING;

			HRESULT hResult = GContext->graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
			assert(SUCCEEDED(hResult));

			GContext->graphics.imDeviceContext->CopyResource(stagingBuffer, faces[i%6].buffer);

			D3D11_MAPPED_SUBRESOURCE MappedResource;
			hResult = GContext->graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
			assert(SUCCEEDED(hResult));

			memcpy(surfaces[i], MappedResource.pData, res * res * sizeof(float) * 4);

			stagingBuffer->Release();
		}
	}

	// subresource data
	D3D11_SUBRESOURCE_DATA* data = new D3D11_SUBRESOURCE_DATA[6*miplevels];
	for (int i = 0; i < 6 * miplevels; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = res * 4 * 4;
		data[i].SysMemSlicePitch = 0;
	}

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	mvComPtr<ID3D11Texture2D> pTexture;
	GContext->graphics.device->CreateTexture2D(&textureDesc, data, &pTexture);
	GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &texture.textureView);

	cbuffer.buffer->Release();
	sampler->Release();
	
	for (int i = 0; i < 6; i++)
	{
		faces[i].buffer->Release();
		faces[i].unorderedAccessView->Release();
	}
	filtershader.shader->Release();

	for (int i = 0; i < 6; i++)
		delete[] surfaces[i];
	return texture;

}

mvSampler
create_sampler(D3D11_FILTER mode, D3D11_TEXTURE_ADDRESS_MODE addressing, b8 hwPcf)
{
    mvSampler sampler{};

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };

	samplerDesc.AddressU = addressing;
	samplerDesc.AddressV = addressing;
	samplerDesc.AddressW = addressing;
	samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.Filter = mode;
    samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

    if (hwPcf)
    {
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    }

	HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, sampler.state.GetAddressOf());
	assert(SUCCEEDED(hResult));

    return sampler;
}
