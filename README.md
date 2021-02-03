# QUno

An Uno-like card game.

## Requirements

* Windows 10
* [Windows 10 SDK](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk/)
* [Visual Studio 2019](https://visualstudio.microsoft.com/) (I use the Community edition, v16.8)
* [C++/WinRT Extension](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264)
* [WinUI 3 Project Templates Extension](https://marketplace.visualstudio.com/items?itemName=Microsoft-WinUI.WinUIProjectTemplates)
* Your favorite editor (my favorite editor is [Visual Studio Code](https://code.visualstudio.com/))

## How To Play

*Coming soon.*

## Developer Notes

This repository includes experimental implementations of QUno in C++. They are inspired 
by the original QUno implementations in MFC and ATL, but brought into the world of 
modern Windows and C++.

* `QUnoComponent` contains the game engine. This is a Windows Runtime Component modeled after 
`QUnoLibrary` in the [QUnoEngine](https://github.com/rdeetz/QUnoEngine) repository. 
* `QUnoBare` contains a Universal Windows Platform application implemented with C++/WinRT that uses 
`Windows.ApplicationModel.Core.CoreApplication` directly and draws a user interface 
using the Composition layer.
* `QUnoDirect` contains a Universal Windows Platform application using DirectX 12 and C++/CX.
* `QUnoReunion` contains a Windows desktop application implemented using C++/WinRT and the 
[Windows UI Library](https://github.com/microsoft/microsoft-ui-xaml).
