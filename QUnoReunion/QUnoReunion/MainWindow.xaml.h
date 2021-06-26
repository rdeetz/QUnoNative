// QUnoReunion - MainWindow.xaml.h
// 2021 Roger Deetz

#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "MainWindow.g.h"

#pragma pop_macro("GetCurrentTime")

namespace winrt::Mooville::QUno::Reunion::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void ButtonSettings_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
    };
}

namespace winrt::Mooville::QUno::Reunion::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
