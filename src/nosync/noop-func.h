// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NOOP_FUNC_H
#define NOSYNC__NOOP_FUNC_H


namespace nosync
{

class noop_func
{
public:
    template <typename ...Types>
    void operator()(Types &&...args) const noexcept;
};

}

#include <nosync/noop-func-impl.h>

#endif /* NOSYNC__NOOP_FUNC_H */
