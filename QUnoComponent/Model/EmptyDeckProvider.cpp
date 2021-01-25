// QUnoComponent - EmptyDeckProvider.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\EmptyDeckProvider.h"
#include "Model.EmptyDeckProvider.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    EmptyDeckProvider::EmptyDeckProvider()
    {
    }

    Windows::Foundation::Collections::IIterable<Mooville::QUno::Model::Card> EmptyDeckProvider::ProvideCards()
    {
        throw hresult_not_implemented();
    }
}
