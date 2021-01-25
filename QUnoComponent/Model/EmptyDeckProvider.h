// QUnoComponent - EmptyDeckProvider.h
// 2021 Roger Deetz

#pragma once

#include "Model\Card.h"
#include "Model\IDeckProvider.h"
#include "Model.EmptyDeckProvider.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct EmptyDeckProvider : EmptyDeckProviderT<EmptyDeckProvider>, IDeckProvider
    {
        EmptyDeckProvider();

        Windows::Foundation::Collections::IIterable<Mooville::QUno::Model::Card> ProvideCards();

    private:
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct EmptyDeckProvider : EmptyDeckProviderT<EmptyDeckProvider, implementation::EmptyDeckProvider>
    {
    };
}
