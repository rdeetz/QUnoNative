﻿// QUnoDirect - Sample3DSceneRenderer.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Sample3DSceneRenderer.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DX;
using namespace Mooville::QUno::Direct;

// Indices into the application state map.
winrt::hstring AngleKey = L"Angle";
winrt::hstring TrackingKey = L"Tracking";

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
/*
inline IAsyncOperation<std::vector<byte>> ReadDataAsync(const winrt::param::hstring& filename)
{
    auto folder = Package::Current().InstalledLocation();
    auto file = co_await folder.GetFileAsync(filename);
    auto fileBuffer = co_await FileIO::ReadBufferAsync(file);
    std::vector<byte> returnBuffer;
    returnBuffer.resize(fileBuffer.Length());
    DataReader::FromBuffer(fileBuffer).ReadBytes(returnBuffer.data());
    //auto dataReader = DataReader::FromBuffer(fileBuffer);
    //dataReader.ReadBytes(returnBuffer.data());

    co_return returnBuffer;

    return create_task(
            // get a file via IAsync given a file name from the installed location folder
            folder->GetFileAsync(Platform::StringReference(filename.c_str()))
        ).then([](StorageFile^ file)
        {
            // This returns an IAsync that returns a buffer
            return FileIO::ReadBufferAsync(file);
        }).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
            {
                // This takes a file buffer, reads the bytes and copies them to an array which is then returned
                std::vector<byte> returnBuffer;
                returnBuffer.resize(fileBuffer->Length);
                Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
                return returnBuffer;
            });
}
*/

Sample3DSceneRenderer::Sample3DSceneRenderer(std::shared_ptr<DeviceResources> const& deviceResources) :
    _loadingComplete(false),
    _radiansPerSecond(XM_PIDIV4),	// rotate 45 degrees per second
    _angle(0),
    _tracking(false),
    _mappedConstantBuffer(nullptr),
    _deviceResources(deviceResources)
{
    LoadState();
    ZeroMemory(&_constantBufferData, sizeof(_constantBufferData));

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

Sample3DSceneRenderer::~Sample3DSceneRenderer()
{
    _constantBuffer->Unmap(0, nullptr);
    _mappedConstantBuffer = nullptr;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
    /*
    auto d3dDevice = m_deviceResources->GetD3DDevice();

    // Create a root signature with a single constant buffer slot.
    {
        CD3DX12_DESCRIPTOR_RANGE range;
        CD3DX12_ROOT_PARAMETER parameter;

        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

        ComPtr<ID3DBlob> pSignature;
        ComPtr<ID3DBlob> pError;
        DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
        DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
        NAME_D3D12_OBJECT(m_rootSignature);
    }

    // Load shaders asynchronously.
    auto createVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso").then([this](std::vector<byte>& fileData) {
        m_vertexShader = fileData;
    });

    auto createPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso").then([this](std::vector<byte>& fileData) {
        m_pixelShader = fileData;
    });

    // Create the pipeline state once the shaders are loaded.
    auto createPipelineStateTask = (createPSTask && createVSTask).then([this]() {

        std::vector<byte>& fileData = DX::ReadDataAsync(L"SampleVertexShader.cso");
        m_vertexShader = fileData;

        std::vector<byte>& fileData = DX::ReadDataAsync(L"SamplePixelShader.cso");
        m_pixelShader = fileData;

        static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
        state.InputLayout = { inputLayout, _countof(inputLayout) };
        state.pRootSignature = m_rootSignature.Get();
        state.VS = CD3DX12_SHADER_BYTECODE(&m_vertexShader[0], m_vertexShader.size());
        state.PS = CD3DX12_SHADER_BYTECODE(&m_pixelShader[0], m_pixelShader.size());
        state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        state.SampleMask = UINT_MAX;
        state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        state.NumRenderTargets = 1;
        state.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
        state.DSVFormat = m_deviceResources->GetDepthBufferFormat();
        state.SampleDesc.Count = 1;

        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&m_pipelineState)));

        // Shader data can be deleted once the pipeline state is created.
        m_vertexShader.clear();
        m_pixelShader.clear();
    });

    // Create and upload cube geometry resources to the GPU.
    auto createAssetsTask = createPipelineStateTask.then([this]() {
        auto d3dDevice = m_deviceResources->GetD3DDevice();

        // Create a command list.
        DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
        NAME_D3D12_OBJECT(m_commandList);

        // Cube vertices. Each vertex has a position and a color.
        VertexPositionColor cubeVertices[] =
        {
            { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
            { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
            { XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
            { XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
        };

        const UINT vertexBufferSize = sizeof(cubeVertices);

        // Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
        // The upload resource must not be released until after the GPU has finished using it.
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

        CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &vertexBufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &vertexBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBufferUpload)));

        NAME_D3D12_OBJECT(m_vertexBuffer);

        // Upload the vertex buffer to the GPU.
        {
            D3D12_SUBRESOURCE_DATA vertexData = {};
            vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
            vertexData.RowPitch = vertexBufferSize;
            vertexData.SlicePitch = vertexData.RowPitch;

            UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

            CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
                CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
        }

        // Load mesh indices. Each trio of indices represents a triangle to be rendered on the screen.
        // For example: 0,2,1 means that the vertices with indexes 0, 2 and 1 from the vertex buffer compose the
        // first triangle of this mesh.
        unsigned short cubeIndices[] =
        {
            0, 2, 1, // -x
            1, 2, 3,

            4, 5, 6, // +x
            5, 7, 6,

            0, 1, 5, // -y
            0, 5, 4,

            2, 6, 7, // +y
            2, 7, 3,

            0, 4, 6, // -z
            0, 6, 2,

            1, 3, 7, // +z
            1, 7, 5,
        };

        const UINT indexBufferSize = sizeof(cubeIndices);

        // Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
        // The upload resource must not be released until after the GPU has finished using it.
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

        CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &indexBufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)));

        DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &indexBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&indexBufferUpload)));

        NAME_D3D12_OBJECT(m_indexBuffer);

        // Upload the index buffer to the GPU.
        {
            D3D12_SUBRESOURCE_DATA indexData = {};
            indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
            indexData.RowPitch = indexBufferSize;
            indexData.SlicePitch = indexData.RowPitch;

            UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

            CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
                CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
        }

        // Create a descriptor heap for the constant buffers.
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = DX::c_frameCount;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            // This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

            NAME_D3D12_OBJECT(m_cbvHeap);
        }

        CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(DX::c_frameCount * c_alignedConstantBufferSize);
        DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        NAME_D3D12_OBJECT(m_constantBuffer);

        // Create constant buffer views to access the upload buffer.
        D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
        m_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (int n = 0; n < DX::c_frameCount; n++)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
            desc.BufferLocation = cbvGpuAddress;
            desc.SizeInBytes = c_alignedConstantBufferSize;
            d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

            cbvGpuAddress += desc.SizeInBytes;
            cbvCpuHandle.Offset(m_cbvDescriptorSize);
        }

        // Map the constant buffers.
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
        ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount * c_alignedConstantBufferSize);
        // We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

        // Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
        DX::ThrowIfFailed(m_commandList->Close());
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Create vertex/index buffer views.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
        m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

        // Wait for the command list to finish executing; the vertex/index buffers need to be uploaded to the GPU before the upload resources go out of scope.
        m_deviceResources->WaitForGpu();
    });

    createAssetsTask.then([this]() {
        m_loadingComplete = true;
    });
    */
}

void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
    RECT outputSize = _deviceResources->GetOutputSize();
    float aspectRatio = (outputSize.right - outputSize.left) / (outputSize.bottom - outputSize.top);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
    _scissorRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height)};

    // This is a simple example of change that can be made when the app is in portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // Note that the OrientationTransform3D matrix is post-multiplied here
    // in order to correctly orient the scene to match the display orientation.
    // This post-multiplication step is required for any draw calls that are
    // made to the swap chain render target. For draw calls to other targets,
    // this transform should not be applied.

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
        );

    XMFLOAT4X4 orientation = _deviceResources->GetOrientationTransform3D();
    XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

    XMStoreFloat4x4(
        &_constantBufferData.projection,
        XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
        );

    // Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
    static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMStoreFloat4x4(&_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

    return;
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(StepTimer const& timer)
{
    if (_loadingComplete)
    {
        if (!_tracking)
        {
            // Rotate the cube a small amount.
            _angle += static_cast<float>(timer.GetElapsedSeconds()) * _radiansPerSecond;
            Rotate(_angle);
        }

        // Update the constant buffer resource.
        UINT8* destination = _mappedConstantBuffer + (_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
        memcpy(destination, &_constantBufferData, sizeof(_constantBufferData));
    }

    return;
}

// Renders one frame using the vertex and pixel shaders.
bool Sample3DSceneRenderer::Render()
{
    // Loading is asynchronous. Only draw geometry after it's loaded.
    if (!_loadingComplete)
    {
        return false;
    }

    ThrowIfFailed(_deviceResources->GetCommandAllocator()->Reset());

    // The command list can be reset anytime after ExecuteCommandList() is called.
    ThrowIfFailed(_commandList->Reset(_deviceResources->GetCommandAllocator(), _pipelineState.Get()));

    PIXBeginEvent(_commandList.Get(), 0, L"Draw the cube");
    {
        // Set the graphics root signature and descriptor heaps to be used by this frame.
        _commandList->SetGraphicsRootSignature(_rootSignature.Get());
        ID3D12DescriptorHeap* ppHeaps[] = { _cbvHeap.Get() };
        _commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        // Bind the current frame's constant buffer to the pipeline.
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_cbvHeap->GetGPUDescriptorHandleForHeapStart(), _deviceResources->GetCurrentFrameIndex(), _cbvDescriptorSize);
        _commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

        // Set the viewport and scissor rectangle.
        D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
        _commandList->RSSetViewports(1, &viewport);
        _commandList->RSSetScissorRects(1, &_scissorRect);

        // Indicate this resource will be in use as a render target.
        CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        _commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

        // Record drawing commands.
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = _deviceResources->GetRenderTargetView();
        D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = _deviceResources->GetDepthStencilView();
        _commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
        _commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        _commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

        _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
        _commandList->IASetIndexBuffer(&_indexBufferView);
        _commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

        // Indicate that the render target will now be used to present when the command list is done executing.
        CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        _commandList->ResourceBarrier(1, &presentResourceBarrier);
    }

    PIXEndEvent(_commandList.Get());

    ThrowIfFailed(_commandList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { _commandList.Get() };
    _deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return true;
}

void Sample3DSceneRenderer::SaveState()
{
    auto state = ApplicationData::Current().LocalSettings().Values();

    if (state.HasKey(AngleKey))
    {
        state.Remove(AngleKey);
    }

    if (state.HasKey(TrackingKey))
    {
        state.Remove(TrackingKey);
    }

    state.Insert(AngleKey, PropertyValue::CreateSingle(_angle));
    state.Insert(TrackingKey, PropertyValue::CreateBoolean(_tracking));

    return;
}

void Sample3DSceneRenderer::LoadState()
{
    auto state = ApplicationData::Current().LocalSettings().Values();

    if (state.HasKey(AngleKey))
    {
        _angle = state.Lookup(AngleKey).as<IPropertyValue>().GetSingle();
        state.Remove(AngleKey);
    }

    if (state.HasKey(TrackingKey))
    {
        _tracking = state.Lookup(TrackingKey).as<IPropertyValue>().GetBoolean();
        state.Remove(TrackingKey);
    }

    return;
}

void Sample3DSceneRenderer::StartTracking()
{
    _tracking = true;
    
    return;
}

void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
    // When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
    if (_tracking)
    {
        float radians = XM_2PI * 2.0f * positionX / (_deviceResources->GetOutputSize().right - _deviceResources->GetOutputSize().left);
        Rotate(radians);
    }

    return;
}

void Sample3DSceneRenderer::StopTracking()
{
    _tracking = false;

    return;
}

void Sample3DSceneRenderer::Rotate(float radians)
{
    // Rotate the 3D cube model a set amount of radians.
    // Prepare to pass the updated model matrix to the shader.
    XMStoreFloat4x4(&_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));

    return;
}
