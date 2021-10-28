#include "mvTextures.h"
#include "mv3D_internal.h"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

mvTexture
mvCreateTexture(const std::string& path)
{
    mvTexture texture{};

    std::filesystem::path fpath = path;

    if (!std::filesystem::exists(path))
    {
        assert(false && "File not found.");
        return texture;
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
mvCreateCubeTexture(const std::string& path)
{
    mvCubeTexture texture{};

	// Load Image
	int texWidth, texHeight, texNumChannels, textBytesPerRow;

	// load 6 surfaces for cube faces
	std::vector<unsigned char*> surfaces;
	int texForceNumChannels = 4;

	// right
	{
		std::string file = path + "\\right.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
	}

	// left
	{
		std::string file = path + "\\left.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
	}

	// top
	{
		std::string file = path + "\\top.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
	}

	// bottom
	{
		std::string file = path + "\\bottom.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
	}

	// front
	{
		std::string file = path + "\\front.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
	}

	// back
	{
		std::string file = path + "\\back.png";
		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces.push_back(testTextureBytes);
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
	GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, texture.textureView.GetAddressOf());

	return texture;
}

mvSampler
mvCreateSampler(D3D11_FILTER mode, D3D11_TEXTURE_ADDRESS_MODE addressing, b8 hwPcf)
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
