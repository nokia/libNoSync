// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__ACTIVITY_HANDLE_H
#define NOSYNC__ACTIVITY_HANDLE_H

#include <nosync/interface-type.h>


namespace nosync
{

/*!
A handle for controlling asynchronous "activity".

Interface for controlling asynchronous "activity" such as scheduled task or
some kind of event monitor that triggers asynchronous tasks.

A handle lets you to:
 - check if related activity is still enabled i.e. can execute some code in
the future,
 - disable the activity if it's enabled (this operation is fully synchronous,
after disable() call the activity won't execute any code).
*/
class activity_handle : public interface_type
{
public:
    virtual bool is_enabled() const = 0;
    virtual void disable() = 0;
};

}

#endif /* NOSYNC__ACTIVITY_HANDLE_H */
