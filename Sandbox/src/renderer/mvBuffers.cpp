#include "mvBuffers.h"
#include "mvSandbox.h"
#include <assert.h>

mvBuffer
create_buffer(void* data, u32 size, D3D11_BIND_FLAG flags, u32 stride, u32 miscFlags)
{
	mvBuffer buffer{};
	buffer.size = size;

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;

    bufferDesc.ByteWidth = size;
    bufferDesc.BindFlags = flags;
    bufferDesc.StructureByteStride = stride;
    bufferDesc.MiscFlags = miscFlags;

    // Define the resource data.
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = data;

    // Create the buffer with the device.
    HRESULT hresult = GContext->graphics.device->CreateBuffer(&bufferDesc, &InitData, nullptr);

    if (hresult != S_FALSE)
    {
        assert(false && "something is wrong");
    }

    else
    {
        hresult = GContext->graphics.device->CreateBuffer(&bufferDesc, &InitData, buffer.buffer.GetAddressOf());
        assert(SUCCEEDED(hresult));
    }

    if (stride != 0u)
    {
        D3D11_BUFFER_DESC descBuf;
        buffer.buffer->GetDesc(&descBuf);

        D3D11_UNORDERED_ACCESS_VIEW_DESC descView = {};
        descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        descView.Buffer.FirstElement = 0;

        descView.Format = DXGI_FORMAT_UNKNOWN;
        descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

        hresult = GContext->graphics.device->CreateUnorderedAccessView(buffer.buffer.Get(),
            &descView, buffer.unorderedAccessView.GetAddressOf());
        assert(SUCCEEDED(hresult));
    }

	return buffer;
}

mvConstBuffer
create_const_buffer(void* data, u32 size)
{
    mvConstBuffer buffer{};
    buffer.size = size;

    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0u;
    cbd.ByteWidth = size;
    cbd.StructureByteStride = 0u;

    if (data != nullptr)
    {
        D3D11_SUBRESOURCE_DATA csd = {};
        csd.pSysMem = &data;
        GContext->graphics.device->CreateBuffer(&cbd, &csd, buffer.buffer.GetAddressOf());
    }
    else
        GContext->graphics.device->CreateBuffer(&cbd, nullptr, buffer.buffer.GetAddressOf());

    return buffer;
}

void
update_const_buffer(mvConstBuffer& buffer, void* data)
{
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    GContext->graphics.imDeviceContext->Map(buffer.buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, data, buffer.size);
    GContext->graphics.imDeviceContext->Unmap(buffer.buffer.Get(), 0u);
}