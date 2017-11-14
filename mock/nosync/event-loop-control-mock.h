// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EVENT_LOOP_CONTROL_MOCK_H
#define NOSYNC__EVENT_LOOP_CONTROL_MOCK_H

#include <gmock/gmock.h>
#include <nosync/event-loop-control.h>


namespace nosync
{

class event_loop_control_mock : public event_loop_control
{
public:
    event_loop_control_mock() = default;

    MOCK_METHOD0(run_iterations, std::error_code());
    MOCK_METHOD0(quit, void());
};

}

#endif /* NOSYNC__EVENT_LOOP_CONTROL_MOCK_H */
