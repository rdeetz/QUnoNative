// QUnoDirect - DeviceResources.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "DX.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace DirectX;

namespace DX
{
    class DeviceResources
    {
    public:
        static const unsigned int c_AllowTearing = 0x1;
        static const unsigned int c_EnableHDR = 0x2;

        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
            unsigned int flags = 0) noexcept(false);
        ~DeviceResources();

        DeviceResources(DeviceResources&&) = default;
        DeviceResources& operator= (DeviceResources&&) = default;

        DeviceResources(DeviceResources const&) = delete;
        DeviceResources& operator= (DeviceResources const&) = delete;

        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation) noexcept;
        bool WindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
        void ValidateDevice();
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept { m_deviceNotify = deviceNotify; }
        void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void WaitForGpu() noexcept;

        // Device Accessors.
        RECT GetOutputSize() const noexcept { return m_outputSize; }
        DXGI_MODE_ROTATION GetRotation() const noexcept { return m_rotation; }

        // Direct3D Accessors.
        auto                        GetD3DDevice() const noexcept { return m_d3dDevice.Get(); }
        auto                        GetSwapChain() const noexcept { return m_swapChain.Get(); }
        auto                        GetDXGIFactory() const noexcept { return m_dxgiFactory.Get(); }
        D3D_FEATURE_LEVEL           GetDeviceFeatureLevel() const noexcept { return m_d3dFeatureLevel; }
        ID3D12Resource* GetRenderTarget() const noexcept { return m_renderTargets[m_backBufferIndex].Get(); }
        ID3D12Resource* GetDepthStencil() const noexcept { return m_depthStencil.Get(); }
        ID3D12CommandQueue* GetCommandQueue() const noexcept { return m_commandQueue.Get(); }
        ID3D12CommandAllocator* GetCommandAllocator() const noexcept { return m_commandAllocators[m_backBufferIndex].Get(); }
        auto                        GetCommandList() const noexcept { return m_commandList.Get(); }
        DXGI_FORMAT                 GetBackBufferFormat() const noexcept { return m_backBufferFormat; }
        DXGI_FORMAT                 GetDepthBufferFormat() const noexcept { return m_depthBufferFormat; }
        D3D12_VIEWPORT              GetScreenViewport() const noexcept { return m_screenViewport; }
        D3D12_RECT                  GetScissorRect() const noexcept { return m_scissorRect; }
        UINT                        GetCurrentFrameIndex() const noexcept { return m_backBufferIndex; }
        UINT                        GetBackBufferCount() const noexcept { return m_backBufferCount; }
        XMFLOAT4X4         GetOrientationTransform3D() const noexcept { return m_orientationTransform3D; }
        DXGI_COLOR_SPACE_TYPE       GetColorSpace() const noexcept { return m_colorSpace; }
        unsigned int                GetDeviceOptions() const noexcept { return m_options; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<INT>(m_backBufferIndex), m_rtvDescriptorSize);
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

    private:
        void MoveToNextFrame();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void UpdateColorSpace();

        static const size_t MAX_BACK_BUFFER_COUNT = 3;

        UINT                                        m_backBufferIndex;

        // Direct3D objects.
        ComPtr<ID3D12Device>                m_d3dDevice;
        ComPtr<ID3D12GraphicsCommandList>   m_commandList;
        ComPtr<ID3D12CommandQueue>          m_commandQueue;
        ComPtr<ID3D12CommandAllocator>      m_commandAllocators[MAX_BACK_BUFFER_COUNT];

        // Swap chain objects.
        ComPtr<IDXGIFactory4>               m_dxgiFactory;
        ComPtr<IDXGISwapChain3>             m_swapChain;
        ComPtr<ID3D12Resource>              m_renderTargets[MAX_BACK_BUFFER_COUNT];
        ComPtr<ID3D12Resource>              m_depthStencil;

        // Presentation fence objects.
        ComPtr<ID3D12Fence>                 m_fence;
        UINT64                                              m_fenceValues[MAX_BACK_BUFFER_COUNT];
        Event                     m_fenceEvent;

        // Direct3D rendering objects.
        ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
        ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;
        UINT                                                m_rtvDescriptorSize;
        D3D12_VIEWPORT                                      m_screenViewport;
        D3D12_RECT                                          m_scissorRect;

        // Direct3D properties.
        DXGI_FORMAT                                         m_backBufferFormat;
        DXGI_FORMAT                                         m_depthBufferFormat;
        UINT                                                m_backBufferCount;
        D3D_FEATURE_LEVEL                                   m_d3dMinFeatureLevel;

        // Cached device properties.
        ::IUnknown* m_window;
        D3D_FEATURE_LEVEL                                   m_d3dFeatureLevel;
        DXGI_MODE_ROTATION                                  m_rotation;
        DWORD                                               m_dxgiFactoryFlags;
        RECT                                                m_outputSize;

        // HDR Support
        DXGI_COLOR_SPACE_TYPE                               m_colorSpace;

        // DeviceResources options (see flags above)
        unsigned int                                        m_options;

        // Transforms used for display orientation.
        XMFLOAT4X4                                 m_orientationTransform3D;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify* m_deviceNotify;
    };
}

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

/*
namespace DX
{
    // Controls all the DirectX device resources.
    class DeviceResources
    {
    public:
        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT);
        void SetWindow(Windows::UI::Core::CoreWindow^ window);
        void SetLogicalSize(Windows::Foundation::Size logicalSize);
        void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
        void SetDpi(float dpi);
        void ValidateDevice();
        void Present();
        void WaitForGpu();

        // The size of the render target, in pixels.
        Windows::Foundation::Size	GetOutputSize() const				{ return m_outputSize; }

        // The size of the render target, in dips.
        Windows::Foundation::Size	GetLogicalSize() const				{ return m_logicalSize; }

        float						GetDpi() const						{ return m_effectiveDpi; }
        bool						IsDeviceRemoved() const				{ return m_deviceRemoved; }

        // D3D Accessors.
        ID3D12Device*				GetD3DDevice() const				{ return m_d3dDevice.Get(); }
        IDXGISwapChain3*			GetSwapChain() const				{ return m_swapChain.Get(); }
        ID3D12Resource*				GetRenderTarget() const				{ return m_renderTargets[m_currentFrame].Get(); }
        ID3D12Resource*				GetDepthStencil() const				{ return m_depthStencil.Get(); }
        ID3D12CommandQueue*			GetCommandQueue() const				{ return m_commandQueue.Get(); }
        ID3D12CommandAllocator*		GetCommandAllocator() const			{ return m_commandAllocators[m_currentFrame].Get(); }
        DXGI_FORMAT					GetBackBufferFormat() const			{ return m_backBufferFormat; }
        DXGI_FORMAT					GetDepthBufferFormat() const		{ return m_depthBufferFormat; }
        D3D12_VIEWPORT				GetScreenViewport() const			{ return m_screenViewport; }
        DirectX::XMFLOAT4X4			GetOrientationTransform3D() const	{ return m_orientationTransform3D; }
        UINT						GetCurrentFrameIndex() const		{ return m_currentFrame; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrame, m_rtvDescriptorSize);
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        }

    private:
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void UpdateRenderTargetSize();
        void MoveToNextFrame();
        DXGI_MODE_ROTATION ComputeDisplayRotation();
        void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);

        UINT											m_currentFrame;

        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D12Device>			m_d3dDevice;
        Microsoft::WRL::ComPtr<IDXGIFactory4>			m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderTargets[c_frameCount];
        Microsoft::WRL::ComPtr<ID3D12Resource>			m_depthStencil;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	m_commandAllocators[c_frameCount];
        DXGI_FORMAT										m_backBufferFormat;
        DXGI_FORMAT										m_depthBufferFormat;
        D3D12_VIEWPORT									m_screenViewport;
        UINT											m_rtvDescriptorSize;
        bool											m_deviceRemoved;

        // CPU/GPU Synchronization.
        Microsoft::WRL::ComPtr<ID3D12Fence>				m_fence;
        UINT64											m_fenceValues[c_frameCount];
        HANDLE											m_fenceEvent;

        // Cached reference to the Window.
        Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;

        // Cached device properties.
        Windows::Foundation::Size						m_d3dRenderTargetSize;
        Windows::Foundation::Size						m_outputSize;
        Windows::Foundation::Size						m_logicalSize;
        Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
        Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
        float											m_dpi;

        // This is the DPI that will be reported back to the app. It takes into account whether the app supports high resolution screens or not.
        float											m_effectiveDpi;

        // Transforms used for display orientation.
        DirectX::XMFLOAT4X4								m_orientationTransform3D;
    };
}
*/
