#include "mvEnvironment.h"
#include "mvAssetManager.h"
#include "mvSandbox.h"
#include <assert.h>
#include "stb_image.h"
#include "mvShader.h"

static mvCubeTexture
create_cube_map_from_hdr(const std::string& path)
{
	mvCubeTexture texture{};

	mvVec4* surfaces[6];

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

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
	GContext->graphics.device->CreateTexture2D(&textureDesc, data, &texture.texture);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GContext->graphics.device->CreateShaderResourceView(texture.texture, &srvDesc, &texture.textureView);
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

static void
copy_resource_to_cubemap(mvCubeTexture& dst, std::vector<mvVec4*>& surfaces, i32 width, i32 height, i32 mipSlice, i32 mipLevels)
{
	D3D11_BOX sourceRegion;
	for (int i = 0; i < 6; ++i)
	{
		sourceRegion.left = 0;
		sourceRegion.right = width;
		sourceRegion.top = 0;
		sourceRegion.bottom = height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		// Create Texture
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;

		ID3D11Texture2D* texture;
		GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, &texture);
		GContext->graphics.imDeviceContext->UpdateSubresource(texture, 0u, nullptr, surfaces[i], 4 * width * sizeof(float), 0u);

		GContext->graphics.imDeviceContext->CopySubresourceRegion(dst.texture, D3D11CalcSubresource(mipSlice, i, mipLevels), 0, 0, 0, texture, 0, &sourceRegion);

		texture->Release();
	}
}

static std::vector<mvVec4*>*
create_single_specular_map(mvCubeTexture& cubemap, i32 resolution, i32 width, i32 sampleCount, f32 lodBias, i32 currentMipLevel, i32 outputMipLevels)
{

	std::vector<mvVec4*>* surfaces = new std::vector<mvVec4*>(7);
	for (int i = 0; i < 6; i++)
		(*surfaces)[i] = new mvVec4[width * width];

	(*surfaces)[6] = new mvVec4[width * width];

	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer((*surfaces)[i], width * width * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	mvBuffer LutBuffer = create_buffer((*surfaces)[6], width * width * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		i32 resolution;
		i32 width;
		f32 roughness;
		u32 sampleCount;

		u32 currentMipLevel;
		f32 lodBias;
		u32 distribution;
		u32 padding;
	};

	MetaData mdata{};
	mdata.resolution = resolution;
	mdata.width = width;
	mdata.sampleCount = sampleCount;
	mdata.distribution = 1;
	mdata.roughness = (float)currentMipLevel / (float)(outputMipLevels - 1);
	mdata.currentMipLevel = currentMipLevel;
	mdata.lodBias = lodBias;

	mvConstBuffer cbuffer = create_const_buffer(&mdata, sizeof(MetaData));
	update_const_buffer(cbuffer, &mdata);

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = width;
	textureDesc.MipLevels = 1;
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	ID3D11SamplerState* sampler;
	GContext->graphics.device->CreateSamplerState(&samplerDesc, &sampler);

	ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, &cbuffer.buffer);
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i, 1u, &faces[i].unorderedAccessView, nullptr);
	ctx->CSSetUnorderedAccessViews(6, 1u, &LutBuffer.unorderedAccessView, nullptr);
	ctx->CSSetShaderResources(0u, 1, &cubemap.textureView);
	ctx->CSSetSamplers(0u, 1, &sampler);
	ctx->CSSetShader(filtershader.shader, nullptr, 0);
	u32 dispatchCount = width / 16;
	assert(dispatchCount > 0u);
	ctx->Dispatch(dispatchCount, dispatchCount, 2u);

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
		hResult = GContext->graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy((*surfaces)[i], MappedResource.pData, width * width * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	if (currentMipLevel == 0)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		LutBuffer.buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;

		HRESULT hResult = GContext->graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		GContext->graphics.imDeviceContext->CopyResource(stagingBuffer, LutBuffer.buffer);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		hResult = GContext->graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy((*surfaces)[6], MappedResource.pData, width * width * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	for (int i = 0; i < 6; i++)
	{
		faces[i].buffer->Release();
		faces[i].unorderedAccessView->Release();
	}

	LutBuffer.buffer->Release();
	LutBuffer.unorderedAccessView->Release();
	sampler->Release();
	cbuffer.buffer->Release();
	filtershader.shader->Release();

	return surfaces;

}

static mvCubeTexture
create_irradiance_map(mvCubeTexture& cubemap, i32 resolution, i32 sampleCount, f32 lodBias)
{

	mvCubeTexture texture{};

	mvVec4* surfaces[6];
	for (int i = 0; i < 6; i++)
		surfaces[i] = new mvVec4[resolution * resolution];

	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(surfaces[i], resolution * resolution * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		i32 resolution;
		u32 width;
		f32 roughness;
		u32 sampleCount;

		u32 currentMipLevel;
		f32 lodBias;
		u32 distribution;
		u32 padding;
	};

	MetaData mdata{};
	mdata.resolution = resolution;
	mdata.width = resolution;
	mdata.sampleCount = sampleCount;
	mdata.distribution = 0;
	mdata.roughness = 0.0f;
	mdata.currentMipLevel = 0;
	mdata.lodBias = lodBias;

	mvConstBuffer cbuffer = create_const_buffer(&mdata, sizeof(MetaData));
	update_const_buffer(cbuffer, &mdata);

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = resolution;
	textureDesc.Height = resolution;
	textureDesc.MipLevels = 1;
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressU;
	samplerDesc.AddressW = samplerDesc.AddressU;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	ID3D11SamplerState* sampler;
	GContext->graphics.device->CreateSamplerState(&samplerDesc, &sampler);

	ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, &cbuffer.buffer);
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i, 1u, &faces[i].unorderedAccessView, nullptr);
	ctx->CSSetShaderResources(0u, 1, &cubemap.textureView);
	ctx->CSSetSamplers(0u, 1, &sampler);
	ctx->CSSetShader(filtershader.shader, nullptr, 0);
	u32 dispatchCount = resolution / 16;
	assert(dispatchCount > 0u);
	ctx->Dispatch(dispatchCount, dispatchCount, 2u);

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
		hResult = GContext->graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy(surfaces[i], MappedResource.pData, resolution * resolution * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = resolution * 4 * 4;
		data[i].SysMemSlicePitch = 0;
	}

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	GContext->graphics.device->CreateTexture2D(&textureDesc, data, &texture.texture);
	GContext->graphics.device->CreateShaderResourceView(texture.texture, &srvDesc, &texture.textureView);

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

mvEnvironment 
create_environment(const std::string& path, i32 resolution, i32 sampleCount, f32 lodBias, i32 mipLevels)
{

    mvEnvironment environment{};

	environment.skyMap = create_cube_map_from_hdr(path);

    // create environment sampler
    D3D11_SAMPLER_DESC envSamplerDesc{};
    envSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    envSamplerDesc.AddressV = envSamplerDesc.AddressU;
    envSamplerDesc.AddressW = envSamplerDesc.AddressU;
    //envSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    envSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    envSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
    envSamplerDesc.MinLOD = 0.0f;
    envSamplerDesc.MaxLOD = (float)mipLevels + 1.0f;
    GContext->graphics.device->CreateSamplerState(&envSamplerDesc, &environment.sampler);

	// create brdfLUT sampler
	D3D11_SAMPLER_DESC brdfLutSamplerDesc{};
	brdfLutSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	brdfLutSamplerDesc.AddressV = brdfLutSamplerDesc.AddressU;
	brdfLutSamplerDesc.AddressW = brdfLutSamplerDesc.AddressU;
	brdfLutSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	brdfLutSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	GContext->graphics.device->CreateSamplerState(&brdfLutSamplerDesc, &environment.brdfSampler);

	// create irradianceMap
	environment.irradianceMap = create_irradiance_map(environment.skyMap, resolution, sampleCount, 0.0f);

	// initial specular map
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = resolution;
	texDesc.Height = resolution;
	texDesc.MipLevels = mipLevels;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hResult = GContext->graphics.device->CreateTexture2D(&texDesc, nullptr, &environment.specularMap.texture);
	assert(SUCCEEDED(hResult));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	hResult = GContext->graphics.device->CreateShaderResourceView(environment.specularMap.texture, &srvDesc, &environment.specularMap.textureView);
	assert(SUCCEEDED(hResult));

	for (int i = mipLevels - 1; i != -1; i--)
	{
		i32 currentWidth = resolution >> i;
		std::vector<mvVec4*>* faces = create_single_specular_map(environment.skyMap, resolution, currentWidth, sampleCount, lodBias, i, mipLevels);
		copy_resource_to_cubemap(environment.specularMap, *faces, currentWidth, currentWidth, i, mipLevels);

		if (i == 0)
		{
			// Create Texture
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = currentWidth;
			textureDesc.Height = currentWidth;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;

			HRESULT hResult = GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, &environment.brdfLUT.texture);
			assert(SUCCEEDED(hResult));
			GContext->graphics.imDeviceContext->UpdateSubresource(environment.brdfLUT.texture, 0u, nullptr, (*faces)[6], 4 * currentWidth * sizeof(float), 0u);

			// create the resource view on the texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;

			hResult = GContext->graphics.device->CreateShaderResourceView(environment.brdfLUT.texture, &srvDesc, &environment.brdfLUT.textureView);
			assert(SUCCEEDED(hResult));
		}

		for (int i = 0; i < 7; i++)
		{
			delete[](*faces)[i];
		}

		delete faces;

	}

    return environment;
}

void
cleanup_environment(mvEnvironment& environment)
{

	if (environment.skyMap.textureView)
	{
		environment.skyMap.texture->Release();
		environment.skyMap.textureView->Release();
		environment.skyMap.textureView = nullptr;
		environment.skyMap.texture = nullptr;
	}

	if (environment.irradianceMap.textureView)
	{
		environment.irradianceMap.texture->Release();
		environment.irradianceMap.textureView->Release();
		environment.irradianceMap.textureView = nullptr;
		environment.irradianceMap.texture = nullptr;
	}

	if (environment.specularMap.textureView)
	{
		environment.specularMap.texture->Release();
		environment.specularMap.textureView->Release();
		environment.specularMap.textureView = nullptr;
		environment.specularMap.texture = nullptr;
	}

	if (environment.brdfLUT.textureView)
	{
		environment.brdfLUT.texture->Release();
		environment.brdfLUT.textureView->Release();
		environment.brdfLUT.textureView = nullptr;
		environment.brdfLUT.texture = nullptr;
	}

	if (environment.sampler)
	{
		environment.sampler->Release();
		environment.sampler = nullptr;
	}

	if (environment.brdfSampler)
	{
		environment.brdfSampler->Release();
		environment.brdfSampler = nullptr;
	}
}