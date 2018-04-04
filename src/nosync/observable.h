// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__OBSERVABLE_H
#define NOSYNC__OBSERVABLE_H

#include <functional>
#include <nosync/activity-handle.h>
#include <nosync/interface-type.h>
#include <memory>


namespace nosync
{

/*!
Interface for observable objects.

Simple interface which corresponds to a variant of "observer" pattern.
It uses standard activity_handle interface for unregistering observers.
*/
class observable : interface_type
{
public:
    virtual std::unique_ptr<activity_handle> add_listener(std::function<void()> &&listener) = 0;
};

}

#endif /* NOSYNC__OBSERVABLE_H */
