// QUnoReunion - MainWindow.xaml.h
// 2021 Roger Deetz

#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "MainWindow.g.h"

#pragma pop_macro("GetCurrentTime")

namespace winrt::QUnoReunion::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::QUnoReunion::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
