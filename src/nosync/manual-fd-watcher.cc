// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <iterator>
#include <nosync/manual-fd-watcher.h>
#include <type_traits>
#include <utility>

using std::back_inserter;
using std::function;
using std::make_shared;
using std::make_tuple;
using std::make_unique;
using std::move;
using std::multimap;
using std::shared_ptr;
using std::tuple;
using std::unique_ptr;
using std::vector;
using std::weak_ptr;


namespace nosync
{

struct fd_watch
{
    fd_watch(int fd, fd_watch_mode mode, function<void()> notify_func);

    int fd;
    fd_watch_mode mode;
    function<void()> notify_func;
};


fd_watch::fd_watch(int fd, fd_watch_mode mode, function<void()> notify_func)
    : fd(fd), mode(mode), notify_func(move(notify_func))
{
}


namespace
{

class manual_activity_handle : public activity_handle
{
public:
    manual_activity_handle(
        weak_ptr<multimap<tuple<int, fd_watch_mode>, shared_ptr<fd_watch>>> registry_wptr,
        weak_ptr<fd_watch> watch_wptr);

    bool is_enabled() const override;
    void disable() override;

private:
    weak_ptr<multimap<tuple<int, fd_watch_mode>, shared_ptr<fd_watch>>> registry_wptr;
    weak_ptr<fd_watch> watch_wptr;
};


manual_activity_handle::manual_activity_handle(
    weak_ptr<multimap<tuple<int, fd_watch_mode>, shared_ptr<fd_watch>>> registry_wptr,
    weak_ptr<fd_watch> watch_wptr)
    : registry_wptr(move(registry_wptr)), watch_wptr(move(watch_wptr))
{
}


void manual_activity_handle::disable()
{
    auto registry_ptr = registry_wptr.lock();
    auto watch_ptr = watch_wptr.lock();
    if (registry_ptr && watch_ptr) {
        std::decay_t<decltype(*registry_ptr->begin())> reg_item = {{watch_ptr->fd, watch_ptr->mode}, watch_ptr};
        auto reg_items_range = registry_ptr->equal_range(reg_item.first);
        auto reg_item_iter = std::find(reg_items_range.first, reg_items_range.second, reg_item);
        if (reg_item_iter != reg_items_range.second) {
            registry_ptr->erase(reg_item_iter);
        }

        registry_wptr.reset();
        watch_wptr.reset();
    }
}


bool manual_activity_handle::is_enabled() const
{
    return registry_wptr.lock() && watch_wptr.lock();
}

}


shared_ptr<manual_fd_watcher> manual_fd_watcher::create()
{
    return make_shared<manual_fd_watcher>();
}


unique_ptr<activity_handle> manual_fd_watcher::add_watch(
    int fd, fd_watch_mode mode, function<void()> &&notify_func)
{
    if (!watches) {
        watches = make_shared<multimap<tuple<int, fd_watch_mode>, shared_ptr<fd_watch>>>();
    }

    auto watch = make_shared<fd_watch>(fd, mode, move(notify_func));
    watches->emplace(make_tuple(watch->fd, watch->mode), watch);

    return make_unique<manual_activity_handle>(watches, watch);
}


vector<tuple<int, fd_watch_mode>> manual_fd_watcher::get_watches_fd_info() const
{
    if (!watches) {
        return {};
    }

    vector<tuple<int, fd_watch_mode>> fd_infos;
    for (auto iter = watches->begin(); iter != watches->end(); iter = watches->upper_bound(iter->first)) {
        fd_infos.push_back(iter->first);
    }

    return fd_infos;
}


bool manual_fd_watcher::notify_watches(int fd, fd_watch_mode mode)
{
    if (!watches) {
        return true;
    }

    vector<weak_ptr<fd_watch>> watches_wptrs;
    auto reg_items_range = watches->equal_range(make_tuple(fd, mode));
    std::transform(
        reg_items_range.first, reg_items_range.second, back_inserter(watches_wptrs),
        [](const auto &item) {
            return item.second;
        });

    bool normal_return = true;

    for (const auto &watch_wptr : watches_wptrs) {
        auto watch_ptr = watch_wptr.lock();
        if (watch_ptr) {
            watch_ptr->notify_func();
        }

        if (quit_request_pending) {
            normal_return = false;
            quit_request_pending = false;
            break;
        }
    }

    return normal_return;
}


void manual_fd_watcher::quit()
{
    quit_request_pending = true;
}

}
