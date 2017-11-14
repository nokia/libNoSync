// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__INTERFACE_TYPE_MOCK_H
#define NOSYNC__INTERFACE_TYPE_MOCK_H

#include <gmock/gmock.h>
#include <nosync/interface-type.h>


namespace nosync
{

class interface_type_mock : public interface_type
{
public:
    interface_type_mock() = default;
};

}

#endif /* NOSYNC__INTERFACE_TYPE_MOCK_H */
