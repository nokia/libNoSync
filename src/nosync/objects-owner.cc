// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/objects-owner.h>
#include <utility>

using std::make_unique;
using std::move;
using std::shared_ptr;
using std::unique_ptr;


namespace nosync
{

void objects_owner::add_object(shared_ptr<interface_type> &&obj)
{
    owned_objects.push_back(move(obj));
}


unique_ptr<objects_owner> make_objects_owner()
{
    return make_unique<objects_owner>();
}

}
