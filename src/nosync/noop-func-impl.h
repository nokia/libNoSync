// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NOOP_FUNC_IMPL_H
#define NOSYNC__NOOP_FUNC_IMPL_H


namespace nosync
{

template <typename ...Types>
void noop_func::operator()(Types &&...) const noexcept
{
}

}

#endif /* NOSYNC__NOOP_FUNC_IMPL_H */
