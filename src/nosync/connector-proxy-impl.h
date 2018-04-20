#ifndef NOSYNC__CONNECTOR_PROXY_IMPL_H
#define NOSYNC__CONNECTOR_PROXY_IMPL_H

#include <utility>


namespace nosync
{

template<typename T>
T make_connector_proxy(std::experimental::string_view, T &&base_obj)
{
    return std::forward<T>(base_obj);
}

}

#endif /* NOSYNC__CONNECTOR_PROXY_IMPL_H */
