// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__ACTIVITY_HANDLE_MOCK_H
#define NOSYNC__ACTIVITY_HANDLE_MOCK_H

#include <gmock/gmock.h>
#include <nosync/activity-handle.h>


namespace nosync
{

class activity_handle_mock : public activity_handle
{
public:
    activity_handle_mock() = default;

    MOCK_CONST_METHOD0(is_enabled, bool());
    MOCK_METHOD0(disable, void());
};

}

#endif /* NOSYNC__ACTIVITY_HANDLE_MOCK_H */
