// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__OBSERVABLE_MOCK_H
#define NOSYNC__OBSERVABLE_MOCK_H

#include <gmock/gmock.h>
#include <nosync/observable.h>


namespace nosync
{

class observable_no_rvref_wrapper : public observable
{
public:
    observable_no_rvref_wrapper() = default;

    std::unique_ptr<activity_handle> add_listener(std::function<void()> &&listener) override final;

    virtual std::unique_ptr<activity_handle> add_listener_impl(std::function<void()> listener) = 0;
};


class observable_mock : public observable_no_rvref_wrapper
{
public:
    observable_mock() = default;

    MOCK_METHOD1(add_listener_impl, std::unique_ptr<activity_handle>(std::function<void()>));
};

}

#include <nosync/observable-mock-impl.h>

#endif /* NOSYNC__OBSERVABLE_MOCK_H */
