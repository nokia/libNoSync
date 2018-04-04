// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_H
#define NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_H

#include <nosync/event-loop-control.h>
#include <nosync/fd-watching-event-loop.h>


namespace nosync
{

/*!
Interface "aggregating" fd_watching_event_loop and event_loop_control.

This interface inherits from both fd_watcher and event_loop without adding any
extra operations. It is the broadest "general purpose" interface for event loop
which should be used only in the "main" part of the application code.
*/
class full_fd_watching_event_loop : public fd_watching_event_loop, public event_loop_control
{
};

}

#endif /* NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_H */
