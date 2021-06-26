// QUnoReunion - MainWindow.xaml.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::Mooville::QUno::Reunion::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        Title(L"QUno");
    }

    void MainWindow::ButtonSettings_Click(IInspectable const&, RoutedEventArgs const&)
    {
        return;
    }
}
