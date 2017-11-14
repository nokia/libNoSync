// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/destroy-notifier.h>
#include <utility>

using std::function;
using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::unique_ptr;


namespace nosync
{

namespace
{

class destroy_notifier : public interface_type
{
public:
    explicit destroy_notifier(function<void()> &&notify_func);
    ~destroy_notifier() override;

private:
    function<void()> notify_func;
};


destroy_notifier::destroy_notifier(function<void()> &&notify_func)
    : notify_func(move(notify_func))
{
}


destroy_notifier::~destroy_notifier()
{
    notify_func();
}

}


unique_ptr<interface_type> make_destroy_notifier(function<void()> &&notify_func)
{
    return make_unique<destroy_notifier>(move(notify_func));
}


shared_ptr<interface_type> make_shared_destroy_notifier(function<void()> &&notify_func)
{
    return make_shared<destroy_notifier>(move(notify_func));
}

}
