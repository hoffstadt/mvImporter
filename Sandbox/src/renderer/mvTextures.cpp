#include "mvTextures.h"
#include "mvSandbox.h"
#include "mvMath.h"
#include <filesystem>
#include <cmath>
#include "mvPipeline.h"

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

static mvCubeTexture
create_cube_texture_separate(const std::string& path)
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

static mvVec3 
uvToXYZ(int face, mvVec2 uv)
{
	if (face == 0)
		return mvVec3{ 1.f, uv.y, -uv.x };

	else if (face == 1)
		return mvVec3{-1.f, uv.y, uv.x};

	else if (face == 2)
		return mvVec3{ +uv.x, -1.f, +uv.y };

	else if (face == 3)
		return mvVec3{ +uv.x, 1.f, -uv.y };

	else if (face == 4)
		return mvVec3{ +uv.x, uv.y, 1.f };

	else //if(face == 5)
		return mvVec3{-uv.x, +uv.y, -1.f};
}

static mvVec2
dirToUV(mvVec3 dir)
{
	return mvVec2{0.5f + 0.5f * atan2(dir.z, dir.x) / (f32)M_PI, 1.f - acos(dir.y) / (f32)M_PI };
}

static i32
pixelFromUV(mvVec2 uv, i32 width, i32 height)
{
	i32 column = uv.x * (f32)width;
	i32 row = uv.y * (f32)height;

	return column + row * width;
}

mvCubeTexture
create_cube_texture(const std::string& path, b8 separate)
{
	if (separate)
		return create_cube_texture_separate(path);

	b8 hardwareAccelerated = false;
	mvCubeTexture texture{};
	//mvTexture temptexture = create_texture(path);

	mvVec4* surfaces[6];

	if (hardwareAccelerated)
	{
		//mvComputeShader shader = create_compute_shader(GContext->IO.shaderDirectory + "panorama_to_cube.hlsl");
	}
	else
	{
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

		mvVec2 inUV = { 0.5f, 2.0f };
		i32 currentPixel = 0;

		i32 res = 2048;
		f32 xinc = 1.5f / (f32)res;
		f32 yinc = 1.5f / (f32)res;
		for (int i = 0; i < 6; i++)
			surfaces[i] = new mvVec4[res*res];

		for (int row = 0; row < res; row++)
		{
			for (int pixel = 0; pixel < res; pixel++)
			{
				for (int face = 0; face < 6; face++)
				{
					mvVec3 scan = uvToXYZ(face, inUV * 2.0f - mvVec2{ 1.0f, 1.0f });

					mvVec3 direction = normalize(scan);

					mvVec2 src = dirToUV(direction);

					i32 srcpixelIndex = pixelFromUV(src, texWidth, texHeight);

					mvVec4 color{};
					color.x = (f32)face/6;
					color.y = (f32)row / res;
					color.z = (f32)pixel / res;
					//color.x = (f32)*(i8*)&testTextureBytes[srcpixelIndex*4]/255.0f;
					//color.y = (f32)*(i8*)&testTextureBytes[srcpixelIndex*4+1]/255.0f;
					//color.z = (f32)*(i8*)&testTextureBytes[srcpixelIndex*4+2]/255.0f;
					color.w = 1.0f;

					surfaces[face][currentPixel] = color;
				}
				currentPixel++;
				inUV.x += xinc;
			}
			inUV.y += yinc;
		}


		// texture descriptor
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = res;
		textureDesc.Height = res;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		//textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
			data[i].SysMemPitch = 4*res*4;
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
