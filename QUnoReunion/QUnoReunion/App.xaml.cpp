// QUnoReunion - App.xaml.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace Mooville::QUno::Reunion;
using namespace Mooville::QUno::Reunion::implementation;

App::App()
{
    InitializeComponent();
    Suspending({ this, &App::OnSuspending });

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
        if (IsDebuggerPresent())
        {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif
}

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    window = make<MainWindow>();
    window.Activate();

    return;
}

void App::OnSuspending([[maybe_unused]] IInspectable const& sender, [[maybe_unused]] Windows::ApplicationModel::SuspendingEventArgs const& e)
{
    // TODO Save application state and stop any background activity.
    return;
}