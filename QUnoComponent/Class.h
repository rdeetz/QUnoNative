// QUnoComponent - Class.h
// 2021 Roger Deetz

#pragma once

#include "Class.g.h"

namespace winrt::Mooville::QUno::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::Mooville::QUno::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
