// This file is part of libnosync library. See LICENSE file for license details.
#include <iterator>
#include <nosync/objects-owner.h>
#include <utility>

using std::make_move_iterator;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;


namespace nosync
{

void objects_owner::add_object(shared_ptr<interface_type> &&obj)
{
    owned_objects.push_back(move(obj));
}


void objects_owner::add_objects(vector<shared_ptr<interface_type>> &&objs)
{
    owned_objects.insert(
        owned_objects.end(),
        make_move_iterator(objs.begin()),
        make_move_iterator(objs.end()));
}


unique_ptr<objects_owner> make_objects_owner(vector<shared_ptr<interface_type>> &&objs)
{
    auto owner = make_unique<objects_owner>();
    owner->add_objects(move(objs));
    return owner;
}

}
