// QUnoComponent - Card.h
// 2021 Roger Deetz

#pragma once

#include "Color.h"
#include "Value.h"
#include "Card.g.h"

namespace winrt::Mooville::QUno::implementation
{
    struct Card : CardT<Card>
    {
        Card() = default;

        Mooville::QUno::Color Color();
        void Color(Mooville::QUno::Color color);
        Mooville::QUno::Value Value();
        void Value(Mooville::QUno::Value value);
    };
}

namespace winrt::Mooville::QUno::factory_implementation
{
    struct Card : CardT<Card, implementation::Card>
    {
    };
}
