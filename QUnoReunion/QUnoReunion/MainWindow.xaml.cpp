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

    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));

        Card card(Color::Red, Value::Five);
        textCard().Text(L"Red Five");

        return;
    }
}
