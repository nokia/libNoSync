// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__DESTROY_NOTIFIER_H
#define NOSYNC__DESTROY_NOTIFIER_H

#include <functional>
#include <nosync/interface-type.h>
#include <memory>


namespace nosync
{

/*!
Create object that calls provided function when being destroyed (unique_ptr variant).

The created object simplifies tracking lifetime of other objects (by
putting destroy notifiers into them). When the notifier is being
destroyed, it calls function provided by the user. This is useful e.g.
when tracking lifetime of data put into closures (which don't have
user-provided destructors).
*/
std::unique_ptr<interface_type> make_destroy_notifier(std::function<void()> &&notify_func);


/*!
Create object that calls provided function when being destroyed (shared_ptr variant).

A variant of make_destroy_notifier() that generates shared_ptr and thus
is slightly more efficient if the user ends up using shared_ptr anyway.
*/
std::shared_ptr<interface_type> make_shared_destroy_notifier(std::function<void()> &&notify_func);

}

#endif /* NOSYNC__DESTROY_NOTIFIER_H */
