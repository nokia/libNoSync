// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SIGNAL_UTILS_H
#define NOSYNC__SIGNAL_UTILS_H


namespace nosync
{

void block_signal(int signal_num);
void ignore_signal(int signal_num);

}

#endif /* NOSYNC__SIGNAL_UTILS_H */
