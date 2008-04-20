#ifndef NOSYNC__CONNECTOR_PROXY_H
#define NOSYNC__CONNECTOR_PROXY_H

#include <experimental/string_view>


namespace nosync
{

/*!
Tag base_obj as part of the border between application core and connectors.

The function is currently used for documenting connectors only. It returns
base_obj without doing anything else. Depending on the connector type, either
connector object itself or function object passed to a connector should be
wrapped with make_connector_proxy() call.

In future the returned object will be a wrapper around base_obj providing
additional functionality. The template will support only specific types then.
*/
template<typename T>
T make_connector_proxy(std::experimental::string_view name, T &&base_obj);

}

#include <nosync/connector-proxy-impl.h>

#endif /* NOSYNC__CONNECTOR_PROXY_H */
