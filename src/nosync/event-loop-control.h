// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EVENT_LOOP_CONTROL_H
#define NOSYNC__EVENT_LOOP_CONTROL_H

#include <nosync/interface-type.h>
#include <system_error>


namespace nosync
{

/*!
Interface for "controlling" event loop.

Interface for "controlling" event loop with the following operations:
 - run_iterations - runs event loop iterations until the loop is "idle" (there
 are no scheduled tasks, no active "watches" for resources, etc.) or quit() is
called,
 - quit - stops loop iterations if run_iterations call is active (it will
return to the caller with error_code containing errc::interrupted).

This interface is intended to be used only in very small portions of
application code which control lifecycle of the even loop (the "main" code).
Other parts should use (and see in terms of interface types) only "user" side
of the event loop API.
*/
class event_loop_control : public interface_type
{
public:
    virtual std::error_code run_iterations() = 0;
    virtual void quit() = 0;
};

}

#endif /* NOSYNC__EVENT_LOOP_CONTROL_H */
