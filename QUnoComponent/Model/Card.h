// QUnoComponent - Card.h
// 2021 Roger Deetz

#pragma once

#include "Model\Color.h"
#include "Model\Value.h"
#include "Model.Card.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct Card : CardT<Card>
    {
        Card() = default;

        Mooville::QUno::Model::Color Color();
        void Color(Mooville::QUno::Model::Color color);
        Mooville::QUno::Model::Value Value();
        void Value(Mooville::QUno::Model::Value value);
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct Card : CardT<Card, implementation::Card>
    {
    };
}
