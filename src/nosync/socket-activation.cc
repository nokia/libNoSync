// This file is part of libnosync library. See LICENSE file for license details.
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <experimental/optional>
#include <fcntl.h>
#include <limits>
#include <nosync/result-utils.h>
#include <nosync/socket-activation.h>
#include <sys/types.h>
#include <unistd.h>

using std::experimental::make_optional;
using std::experimental::nullopt;
using std::experimental::optional;
using std::getenv;
using std::numeric_limits;
using std::strlen;
using std::strtol;
using std::vector;


namespace nosync
{

namespace
{

constexpr auto socket_activation_first_fd = 3;

constexpr auto socket_activation_listen_pid_var_name = "LISTEN_PID";
constexpr auto socket_activation_listen_fds_var_name = "LISTEN_FDS";


result<void> set_cloexec_flag_on_fd(int fd)
{
    auto flags_get_retval = ::fcntl(fd, F_GETFD);
    int retval = flags_get_retval >= 0
        ? ::fcntl(fd, F_SETFD, flags_get_retval | FD_CLOEXEC)
        : flags_get_retval;

    return retval >= 0
        ? make_ok_result()
        : make_raw_error_result_from_errno();
}


optional<int> try_parse_int(const char *str_value)
{
    errno = 0;
    char *end_ptr;
    auto value = strtol(str_value, &end_ptr, 10);

    auto opt_value =
        errno == 0 && end_ptr == str_value + strlen(str_value)
        && (numeric_limits<long>::max() == numeric_limits<int>::max() || value <= numeric_limits<int>::max())
            ? make_optional(static_cast<int>(value))
            : nullopt;

    return opt_value;
}


optional<int> try_get_int_env_var(const char *var_name)
{
    const auto *str_value = getenv(var_name);
    return str_value != nullptr ? try_parse_int(str_value) : nullopt;
}

}


vector<owned_fd> takeover_socket_activated_fds()
{
    const auto listen_pid_var = try_get_int_env_var(socket_activation_listen_pid_var_name);
    const auto listen_fds_var = try_get_int_env_var(socket_activation_listen_fds_var_name);
    if (!listen_pid_var || !listen_fds_var
        || *listen_fds_var > numeric_limits<int>::max() - socket_activation_first_fd
        || *listen_pid_var != static_cast<int>(::getpid())) {
        return {};
    }

    (void) ::unsetenv(socket_activation_listen_pid_var_name);
    (void) ::unsetenv(socket_activation_listen_fds_var_name);

    vector<owned_fd> activated_fds;
    for (int i = 0; i < *listen_fds_var; ++i) {
        int fd = socket_activation_first_fd + i;
        if (set_cloexec_flag_on_fd(fd).is_ok()) {
            activated_fds.emplace_back(fd);
        }
    }

    return activated_fds;
}

}
