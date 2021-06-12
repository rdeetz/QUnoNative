// QUnoReunion - MainWindow.xaml.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "winrt/Mooville.QUno.Model.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Mooville::QUno::Model;

namespace winrt::Mooville::QUno::Reunion::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        Title(L"QUno");
    }

    void MainWindow::ButtonNew_Click(IInspectable const&, RoutedEventArgs const&)
    {
        textMessage().Text(L"You are the winner!");

        return;
    }
}
