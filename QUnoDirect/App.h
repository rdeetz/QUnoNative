// QUnoDirect - App.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "Game.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace DirectX;
using namespace Mooville::QUno::Direct;

namespace Mooville::QUno::Direct
{
    class FrameworkViewSource : public winrt::implements<FrameworkViewSource, IFrameworkViewSource>
    {
    public:
        // IFrameworkViewSource
        IFrameworkView CreateView();
    };

    class App : public winrt::implements<App, IFrameworkView>
    {
    public:
        App() noexcept :
            _exit(false),
            _visible(true),
            _sizemove(false),
            _dpi(96.f),
            _logicalWidth(1024.f),
            _logicalHeight(768.f),
            _nativeOrientation(DisplayOrientations::None),
            _currentOrientation(DisplayOrientations::None)
        {
        }

        // IFrameworkView
        void Initialize(CoreApplicationView const& applicationView);
        void Uninitialize() noexcept;
        void SetWindow(CoreWindow const& window);
        void Load(winrt::hstring const& entryPoint) noexcept;
        void Run();

    protected:
        void OnActivated(CoreApplicationView const& applicationView, IActivatedEventArgs const& args);
        void OnSuspending(IInspectable const& sender, SuspendingEventArgs const& args);
        void OnResuming(IInspectable const& sender, IInspectable const& args);
        void OnWindowSizeChanged(CoreWindow const& window, WindowSizeChangedEventArgs const& args);
        void OnVisibilityChanged(CoreWindow const& window, VisibilityChangedEventArgs const& args);
        void OnAcceleratorKeyActivated(CoreDispatcher const& dispatcher, AcceleratorKeyEventArgs const& args);
        void OnDpiChanged(DisplayInformation const& sender, IInspectable const& args);
        void OnOrientationChanged(DisplayInformation const& sender, IInspectable const& args);
        void OnDisplayContentsInvalidated(DisplayInformation const& sender, IInspectable const& args);

    private:
        bool _exit;
        bool _visible;
        bool _sizemove;
        float _dpi;
        float _logicalWidth;
        float _logicalHeight;
        DisplayOrientations	_nativeOrientation;
        DisplayOrientations	_currentOrientation;
        std::unique_ptr<Game> _game;

        DXGI_MODE_ROTATION ComputeDisplayRotation() const noexcept;
        void HandleWindowSizeChanged();

        inline int ConvertDipsToPixels(float dips) const noexcept
        {
            return int(dips * _dpi / 96.f + 0.5f);
        }

        inline float ConvertPixelsToDips(int pixels) const noexcept
        {
            return (float(pixels) * 96.f / _dpi);
        }
    };
}
