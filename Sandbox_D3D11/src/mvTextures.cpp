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
create_texture(u32 width, u32 height, u32 arraySize, f32* data)
{
	mvTexture texture{};

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//HRESULT hResult = GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, &texture.texture);
	//assert(SUCCEEDED(hResult));
	i32 texBytesPerRow = 4 * width * sizeof(f32);

	// subresource data
	mvComPtr<ID3D11Texture2D> textureResource;
	D3D11_SUBRESOURCE_DATA* sdata = new D3D11_SUBRESOURCE_DATA[arraySize];
	for (int i = 0; i < arraySize; i++)
	{
		sdata[i].pSysMem = &data[i* width * height * 4];
		//sdata[i].pSysMem = data;
		sdata[i].SysMemPitch = texBytesPerRow;
		sdata[i].SysMemSlicePitch = width * height * 4 * sizeof(f32);
	}
	// create the texture resource
	HRESULT hResult = GContext->graphics.device->CreateTexture2D(&textureDesc, sdata, textureResource.GetAddressOf());
	assert(SUCCEEDED(hResult));
	delete[] sdata;

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = arraySize;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;

	hResult = GContext->graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = samplerDesc.AddressV;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, texture.sampler.GetAddressOf());
	assert(SUCCEEDED(hResult));

	return texture;
}

mvTexture
create_dynamic_texture(u32 width, u32 height, u32 arraySize)
{
	mvTexture texture{};
	mvComPtr<ID3D11Texture2D> textureResource;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hResult = GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;

	hResult = GContext->graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = samplerDesc.AddressV;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, texture.sampler.GetAddressOf());
	assert(SUCCEEDED(hResult));

	return texture;
}

void
update_dynamic_texture(mvTexture& texture, u32 width, u32 height, f32* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//  Disable GPU access to the vertex buffer data.
	ID3D11Resource* resource;
	texture.textureView->GetResource(&resource);
	GContext->graphics.imDeviceContext->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
	BYTE* buffer = reinterpret_cast<BYTE*>(data);
	for (UINT i = 0; i < height; i++)
	{
		memcpy(mappedData, buffer, width * 4 * sizeof(int));
		mappedData += mappedResource.RowPitch;
		buffer += width * 4 * sizeof(int);
	}
	GContext->graphics.imDeviceContext->Unmap(resource, 0);

	resource->Release();
}

mvTexture
create_texture(mvVector<unsigned char> data)
{
	mvTexture texture{};
	mvComPtr<ID3D11Texture2D> textureResource;

	f32 gamma = 1.0f;
	f32 gamma_scale = 1.0f;

	//if (stbi_is_hdr_from_memory(data.data(), data.size()))
	//{
	//	stbi_hdr_to_ldr_gamma(gamma);
	//	stbi_hdr_to_ldr_scale(gamma_scale);
	//}
	//else
	//{
	//	stbi_ldr_to_hdr_gamma(gamma);
	//	stbi_ldr_to_hdr_scale(gamma_scale);
	//}

	// Load Image
	i32 texWidth, texHeight, texNumChannels;
	i32 texForceNumChannels = 4;
	unsigned char* testTextureBytes = stbi_load_from_memory(data.data, data.size, &texWidth, &texHeight,
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

	GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
	GContext->graphics.imDeviceContext->UpdateSubresource(textureResource.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	GContext->graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	GContext->graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

	free(testTextureBytes);

	return texture;
}

mvTexture
create_texture(const std::string& path)
{
    mvTexture texture{};
	mvComPtr<ID3D11Texture2D> textureResource;

    std::filesystem::path fpath = path;

    if (!std::filesystem::exists(path))
    {
        assert(false && "File not found.");
        return texture;
    }

	f32 gamma = 2.2f;
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

    GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
    GContext->graphics.imDeviceContext->UpdateSubresource(textureResource.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    GContext->graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
    GContext->graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

    free(testTextureBytes);

    return texture;
}

mvCubeTexture
create_cube_texture(const std::string& path)
{
	mvCubeTexture texture{};
	mvComPtr<ID3D11Texture2D> textureResource;

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
	GContext->graphics.device->CreateTexture2D(&textureDesc, data, textureResource.GetAddressOf());

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GContext->graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());

	return texture;
}
