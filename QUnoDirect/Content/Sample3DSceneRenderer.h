// QUnoDirect - Sample3DSceneRenderer.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "..\Common\DX.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DeviceResources.h"

//using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage::Streams;
//using namespace Microsoft::WRL;
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
        winrt::Windows::Foundation::IAsyncOperation<IBuffer> ReadDataAsync(winrt::param::hstring const& filename);
        winrt::array_view<byte> GetBufferView(IBuffer const& buffer);
        void CreateRootSignature();
        winrt::Windows::Foundation::IAsyncAction LoadShaders();
        void CreatePipelineState();
        void UploadCommands();

        std::shared_ptr<DeviceResources> _deviceResources;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
        Microsoft::WRL::ComPtr<ID3D12RootSignature>	_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState>	_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _constantBuffer;
        ModelViewProjectionConstantBuffer _constantBufferData;
        UINT8* _mappedConstantBuffer;
        UINT _cbvDescriptorSize;
        D3D12_RECT _scissorRect;
        winrt::array_view<byte> _vertexShader;
        winrt::array_view<byte> _pixelShader;
        D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW _indexBufferView;
        bool _loadingComplete;
        float _radiansPerSecond;
        float _angle;
        bool _tracking;
    };
}
