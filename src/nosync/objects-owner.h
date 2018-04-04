// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__OBJECTS_OWNER_H
#define NOSYNC__OBJECTS_OWNER_H

#include <memory>
#include <nosync/interface-type.h>
#include <vector>


namespace nosync
{

class objects_owner : public interface_type
{
public:
    objects_owner() = default;

    void add_object(std::shared_ptr<interface_type> &&obj);

private:
    std::vector<std::shared_ptr<interface_type>> owned_objects;
};


std::unique_ptr<objects_owner> make_objects_owner();

}

#endif /* NOSYNC__OBJECTS_OWNER_H */
