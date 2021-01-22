// QUnoComponent - Card.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Card.h"
#include "Model.Card.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Card::Card(Mooville::QUno::Model::Color color, Mooville::QUno::Model::Value value)
    {
        _color = color;
        _value = value;
    }

    Mooville::QUno::Model::Color Card::Color()
    {
        return _color;
    }

    void Card::Color(Mooville::QUno::Model::Color color)
    {
        _color = color;
        return;
    }

    Mooville::QUno::Model::Value Card::Value()
    {
        return _value;
    }

    void Card::Value(Mooville::QUno::Model::Value value)
    {
        _value = value;
        return;
    }
}
