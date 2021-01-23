// QUnoComponent - Hand.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Hand.h"
#include "Model.Hand.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Hand::Hand()
    {
        _cards = winrt::single_threaded_observable_vector<Mooville::QUno::Model::Card>();
    }

    Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Card> Hand::Cards()
    {
        return _cards;
    }
}
