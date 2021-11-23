// QUnoReunion - App.xaml..h
// 2021 Roger Deetz

#pragma once

#include "App.xaml.g.h"

namespace winrt::Mooville::QUno::Reunion::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
    };
}
