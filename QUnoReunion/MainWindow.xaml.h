// QUnoReunion - MainWindow.xaml.h
// 2021 Roger Deetz

#pragma once

#include "MainWindow.g.h"

namespace winrt::Mooville::QUno::Reunion::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Mooville::QUno::Reunion::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
