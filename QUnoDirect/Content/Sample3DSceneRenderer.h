// QUnoDirect - Sample3DSceneRenderer.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "..\Common\DX.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DeviceResources.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DX;

namespace Mooville::QUno::Direct
{
    class Sample3DSceneRenderer
    {
    public:
        Sample3DSceneRenderer(std::shared_ptr<DeviceResources> const& deviceResources);
        ~Sample3DSceneRenderer();
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void Update(StepTimer const& timer);
        bool Render();
        void SaveState();
        void LoadState();
        void StartTracking();
        void TrackingUpdate(float positionX);
        void StopTracking();
        bool IsTracking() { return _tracking; }

    private:
        // Constant buffers must be 256-byte aligned.
        static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

        void Rotate(float radians);

        std::shared_ptr<DeviceResources> _deviceResources;
        ComPtr<ID3D12GraphicsCommandList> _commandList;
        ComPtr<ID3D12RootSignature>	_rootSignature;
        ComPtr<ID3D12PipelineState>	_pipelineState;
        ComPtr<ID3D12DescriptorHeap> _cbvHeap;
        ComPtr<ID3D12Resource> _vertexBuffer;
        ComPtr<ID3D12Resource> _indexBuffer;
        ComPtr<ID3D12Resource> _constantBuffer;
        ModelViewProjectionConstantBuffer _constantBufferData;
        UINT8* _mappedConstantBuffer;
        UINT _cbvDescriptorSize;
        D3D12_RECT _scissorRect;
        std::vector<byte> _vertexShader;
        std::vector<byte> _pixelShader;
        D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW _indexBufferView;
        bool _loadingComplete;
        float _radiansPerSecond;
        float _angle;
        bool _tracking;
    };
}
