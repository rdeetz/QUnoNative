// QUnoDirect - App.cpp
// 2021 Roger Deetz

#include "pch.h"

#include "App.h"
//#include "Game.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace DirectX;

namespace Mooville::QUno::Direct
{
    class App : public winrt::implements<App, IFrameworkView>
    {
    public:
        App() noexcept :
            m_exit(false),
            m_visible(true),
            m_in_sizemove(false),
            m_DPI(96.f),
            m_logicalWidth(800.f),
            m_logicalHeight(600.f),
            m_nativeOrientation(DisplayOrientations::None),
            m_currentOrientation(DisplayOrientations::None)
        {
        }

        void Initialize(CoreApplicationView const& applicationView)
        {
            applicationView.Activated({ this, &App::OnActivated });

            CoreApplication::Suspending({ this, &App::OnSuspending });

            CoreApplication::Resuming({ this, &App::OnResuming });

            //m_game = std::make_unique<Game>();
        }

        void Uninitialize() noexcept
        {
            //m_game.reset();
            return;
        }

        void SetWindow(CoreWindow const& window)
        {
            window.SizeChanged({ this, &App::OnWindowSizeChanged });

            try
            {
                window.ResizeStarted([this](auto&&, auto&&) { m_in_sizemove = true; });
                window.ResizeCompleted([this](auto&&, auto&&) { m_in_sizemove = false; HandleWindowSizeChanged(); });
            }
            catch (...)
            {
                // Requires Windows 10 Creators Update (10.0.15063) or later.
            }

            window.VisibilityChanged({ this, &App::OnVisibilityChanged });
            window.Closed([this](auto&&, auto&&) { m_exit = true; });

            auto dispatcher = CoreWindow::GetForCurrentThread().Dispatcher();

            dispatcher.AcceleratorKeyActivated({ this, &App::OnAcceleratorKeyActivated });

            auto navigation = SystemNavigationManager::GetForCurrentView();

            // UWP on Xbox One triggers a back request whenever the B button is pressed
            // which can result in the app being suspended if unhandled.
            navigation.BackRequested([](const winrt::Windows::Foundation::IInspectable&, const BackRequestedEventArgs& args)
                {
                    args.Handled(true);
                });

            auto currentDisplayInformation = DisplayInformation::GetForCurrentView();

            currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });

            currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });

            DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });

            m_DPI = currentDisplayInformation.LogicalDpi();

            m_logicalWidth = window.Bounds().Width;
            m_logicalHeight = window.Bounds().Height;

            m_nativeOrientation = currentDisplayInformation.NativeOrientation();
            m_currentOrientation = currentDisplayInformation.CurrentOrientation();

            int outputWidth = ConvertDipsToPixels(m_logicalWidth);
            int outputHeight = ConvertDipsToPixels(m_logicalHeight);

            DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

            if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
            {
                std::swap(outputWidth, outputHeight);
            }

            auto windowPtr = static_cast<::IUnknown*>(winrt::get_abi(window));
            //m_game->Initialize(windowPtr, outputWidth, outputHeight, rotation);
        }

        void Load(winrt::hstring const&) noexcept
        {
        }

        void Run()
        {
            while (!m_exit)
            {
                if (m_visible)
                {
                    //m_game->Tick();

                    CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
                }
                else
                {
                    CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
                }
            }
        }

    protected:
        void OnActivated(CoreApplicationView const&, IActivatedEventArgs const& args)
        {
            if (args.Kind() == ActivationKind::Launch)
            {
                auto launchArgs = (const LaunchActivatedEventArgs*)(&args);

                if (launchArgs->PrelaunchActivated())
                {
                    // Opt-out of Prelaunch
                    CoreApplication::Exit();
                    return;
                }
            }

            int w, h;
            //m_game->GetDefaultSize(w, h);
            w = 1024;
            h = 768;

            m_DPI = DisplayInformation::GetForCurrentView().LogicalDpi();

            ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
            // Change to ApplicationViewWindowingMode::FullScreen to default to full screen

            auto desiredSize = Size(ConvertPixelsToDips(w), ConvertPixelsToDips(h));

            ApplicationView::PreferredLaunchViewSize(desiredSize);

            auto view = ApplicationView::GetForCurrentView();

            auto minSize = Size(ConvertPixelsToDips(320), ConvertPixelsToDips(200));

            view.SetPreferredMinSize(minSize);

            CoreWindow::GetForCurrentThread().Activate();

            view.FullScreenSystemOverlayMode(FullScreenSystemOverlayMode::Minimal);

            view.TryResizeView(desiredSize);

            return;
        }

        void OnSuspending(IInspectable const&, SuspendingEventArgs const& args)
        {
            auto deferral = args.SuspendingOperation().GetDeferral();

            auto f = std::async(std::launch::async, [this, deferral]()
                {
                    //m_game->OnSuspending();

                    deferral.Complete();
                });

            return;
        }

        void OnResuming(IInspectable const&, IInspectable const&)
        {
            //m_game->OnResuming();
            return;
        }

        void OnWindowSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const&)
        {
            m_logicalWidth = sender.Bounds().Width;
            m_logicalHeight = sender.Bounds().Height;

            if (m_in_sizemove)
                return;

            HandleWindowSizeChanged();
        }

        void OnVisibilityChanged(CoreWindow const&, VisibilityChangedEventArgs const& args)
        {
            m_visible = args.Visible();
            /*
            if (m_visible)
                m_game->OnActivated();
            else
                m_game->OnDeactivated();
            */
        }

        void OnAcceleratorKeyActivated(CoreDispatcher const&, AcceleratorKeyEventArgs const& args)
        {
            if (args.EventType() == CoreAcceleratorKeyEventType::SystemKeyDown
                && args.VirtualKey() == VirtualKey::Enter
                && args.KeyStatus().IsMenuKeyDown
                && !args.KeyStatus().WasKeyDown)
            {
                // Implements the classic ALT+ENTER fullscreen toggle
                auto view = ApplicationView::GetForCurrentView();

                if (view.IsFullScreenMode())
                    view.ExitFullScreenMode();
                else
                    view.TryEnterFullScreenMode();

                args.Handled(true);
            }
        }

        void OnDpiChanged(DisplayInformation const& sender, IInspectable const&)
        {
            m_DPI = sender.LogicalDpi();

            HandleWindowSizeChanged();
        }

        void OnOrientationChanged(DisplayInformation const& sender, IInspectable const&)
        {
            auto resizeManager = CoreWindowResizeManager::GetForCurrentView();
            resizeManager.ShouldWaitForLayoutCompletion(true);

            m_currentOrientation = sender.CurrentOrientation();

            HandleWindowSizeChanged();

            resizeManager.NotifyLayoutCompleted();
        }

        void OnDisplayContentsInvalidated(DisplayInformation const&, IInspectable const&)
        {
            //m_game->ValidateDevice();
        }

    private:
        bool                    m_exit;
        bool                    m_visible;
        bool                    m_in_sizemove;
        float                   m_DPI;
        float                   m_logicalWidth;
        float                   m_logicalHeight;
        //std::unique_ptr<Game>   m_game;

        winrt::Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
        winrt::Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;

        inline int ConvertDipsToPixels(float dips) const noexcept
        {
            return int(dips * m_DPI / 96.f + 0.5f);
        }

        inline float ConvertPixelsToDips(int pixels) const noexcept
        {
            return (float(pixels) * 96.f / m_DPI);
        }

        DXGI_MODE_ROTATION ComputeDisplayRotation() const noexcept
        {
            DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

            switch (m_nativeOrientation)
            {
            case DisplayOrientations::Landscape:
                switch (m_currentOrientation)
                {
                case DisplayOrientations::Landscape:
                    rotation = DXGI_MODE_ROTATION_IDENTITY;
                    break;

                case DisplayOrientations::Portrait:
                    rotation = DXGI_MODE_ROTATION_ROTATE270;
                    break;

                case DisplayOrientations::LandscapeFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE180;
                    break;

                case DisplayOrientations::PortraitFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE90;
                    break;
                }
                break;

            case DisplayOrientations::Portrait:
                switch (m_currentOrientation)
                {
                case DisplayOrientations::Landscape:
                    rotation = DXGI_MODE_ROTATION_ROTATE90;
                    break;

                case DisplayOrientations::Portrait:
                    rotation = DXGI_MODE_ROTATION_IDENTITY;
                    break;

                case DisplayOrientations::LandscapeFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE270;
                    break;

                case DisplayOrientations::PortraitFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE180;
                    break;
                }
                break;
            }

            return rotation;
        }

        void HandleWindowSizeChanged()
        {
            int outputWidth = ConvertDipsToPixels(m_logicalWidth);
            int outputHeight = ConvertDipsToPixels(m_logicalHeight);

            DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

            if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
            {
                std::swap(outputWidth, outputHeight);
            }

            //m_game->OnWindowSizeChanged(outputWidth, outputHeight, rotation);
        }
    };

    class FrameworkViewSource : public winrt::implements<FrameworkViewSource, IFrameworkViewSource>
    {
    public:
        IFrameworkView CreateView()
        {
            return winrt::make<App>();
        }
    };
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    if (!XMVerifyCPUSupport())
    {
        throw std::runtime_error("XMVerifyCPUSupport");
    }

    auto frameworkViewSource = winrt::make<Mooville::QUno::Direct::FrameworkViewSource>();
    CoreApplication::Run(frameworkViewSource);

    return 0;
}

void ExitGame() noexcept
{
    winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
    return;
}

/*
#include "pch.h"
#include "App.h"
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Mooville::QUno::Direct;

using Microsoft::WRL::ComPtr;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto direct3DApplicationSource = ref new Direct3DApplicationSource();
    CoreApplication::Run(direct3DApplicationSource);
    return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new App();
}

App::App() :
    m_windowClosed(false),
    m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

    CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
    window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

    currentDisplayInformation->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

    currentDisplayInformation->OrientationChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

    DisplayInformation::DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
    if (m_main == nullptr)
    {
        m_main = std::unique_ptr<QUnoDirectMain>(new QUnoDirectMain());
    }
}

// This method is called after the window becomes active.
void App::Run()
{
    while (!m_windowClosed)
    {
        if (m_windowVisible)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            auto commandQueue = GetDeviceResources()->GetCommandQueue();
            PIXBeginEvent(commandQueue, 0, L"Update");
            {
                m_main->Update();
            }
            PIXEndEvent(commandQueue);

            PIXBeginEvent(commandQueue, 0, L"Render");
            {
                if (m_main->Render())
                {
                    GetDeviceResources()->Present();
                }
            }
            PIXEndEvent(commandQueue);
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    // Save app state asynchronously after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely. After about five seconds,
    // the app will be forced to exit.
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

    create_task([this, deferral]()
    {
        m_main->OnSuspending();
        deferral->Complete();
    });
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
    // Restore any data or state that was unloaded on suspend. By default, data
    // and state are persisted when resuming from suspend. Note that this event
    // does not occur if the app was previously terminated.

    m_main->OnResuming();
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
    m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
    // Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
    // if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
    // you should always retrieve it using the GetDpi method.
    // See DeviceResources.cpp for more details.
    GetDeviceResources()->SetDpi(sender->LogicalDpi);
    m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
    GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
    m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
    GetDeviceResources()->ValidateDevice();
}

std::shared_ptr<DX::DeviceResources> App::GetDeviceResources()
{
    if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
    {
        // All references to the existing D3D device must be released before a new device
        // can be created.

        m_deviceResources = nullptr;
        m_main->OnDeviceRemoved();

#if defined(_DEBUG)
        ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
#endif
    }

    if (m_deviceResources == nullptr)
    {
        m_deviceResources = std::make_shared<DX::DeviceResources>();
        m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
        m_main->CreateRenderers(m_deviceResources);
    }

    return m_deviceResources;
}
*/
