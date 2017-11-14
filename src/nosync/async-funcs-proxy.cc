// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <nosync/async-funcs-proxy.h>
#include <utility>
#include <vector>

using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::vector;
using std::weak_ptr;


namespace nosync
{

class async_func_wrapper
{
public:
    async_func_wrapper(weak_ptr<function<void()>> func_wptr, weak_ptr<vector<shared_ptr<function<void()>>>> owning_registry_wptr);
    ~async_func_wrapper();

    async_func_wrapper(const async_func_wrapper &) = delete;
    async_func_wrapper &operator=(const async_func_wrapper &) = delete;

    void operator()();

private:
    weak_ptr<function<void()>> func_wptr;
    weak_ptr<vector<shared_ptr<function<void()>>>> owning_registry_wptr;
};


async_func_wrapper::async_func_wrapper(
    weak_ptr<function<void()>> func_wptr, weak_ptr<vector<shared_ptr<function<void()>>>> owning_registry_wptr)
    : func_wptr(move(func_wptr)), owning_registry_wptr(move(owning_registry_wptr))
{
}


async_func_wrapper::~async_func_wrapper()
{
    auto func_ptr = func_wptr.lock();
    auto owning_registry_ptr = owning_registry_wptr.lock();
    if (func_ptr && owning_registry_ptr) {
        auto func_ptr_iter = std::find(owning_registry_ptr->begin(), owning_registry_ptr->end(), func_ptr);
        if (func_ptr_iter != owning_registry_ptr->end()) {
            owning_registry_ptr->erase(func_ptr_iter);
        }
    }
}


void async_func_wrapper::operator()()
{
    auto func_ptr = func_wptr.lock();
    if (func_ptr) {
        (*func_ptr)();
    }
}


function<void()> async_funcs_proxy::wrap(function<void()> &&func)
{
    if (!active_funcs_registry) {
        active_funcs_registry = make_shared<vector<shared_ptr<function<void()>>>>();
    }
    active_funcs_registry->push_back(make_shared<function<void()>>(move(func)));

    return [func_wrapper_ptr = make_shared<async_func_wrapper>(active_funcs_registry->back(), active_funcs_registry)]() {
        return (*func_wrapper_ptr)();
    };
}

}
