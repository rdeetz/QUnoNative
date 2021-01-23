// QUnoComponent - Hand.h
// 2021 Roger Deetz

#pragma once

#include "Model\Card.h"
#include "Model.Hand.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct Hand : HandT<Hand>
    {
        Hand();

        Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Card> Cards();

    private:
        Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Card> _cards;
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct Hand : HandT<Hand, implementation::Hand>
    {
    };
}
