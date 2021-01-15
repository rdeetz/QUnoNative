#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Mooville::QUno::Reunion;
using namespace Mooville::QUno::Reunion::implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

MainWindow::MainWindow()
{
    InitializeComponent();
}

int32_t MainWindow::MyProperty()
{
    throw hresult_not_implemented();
}

void MainWindow::MyProperty(int32_t /* value */)
{
    throw hresult_not_implemented();
}

/*
void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
{
    myButton().Content(box_value(L"Clicked"));
}
*/
