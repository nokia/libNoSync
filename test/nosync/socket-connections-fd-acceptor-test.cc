// This file is part of libnosync library. See LICENSE file for license details.
#include <array>
#include <cerrno>
#include <experimental/array>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/fd-watcher-mock.h>
#include <nosync/owned-fd.h>
#include <nosync/result.h>
#include <nosync/socket-connections-fd-acceptor.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std::string_literals;
using nosync::activity_handle_mock;
using nosync::fd_watch_mode;
using nosync::make_error_result;
using nosync::fd_watcher_mock;
using nosync::make_full_socket_connections_fd_acceptor;
using nosync::make_socket_connections_fd_acceptor;
using nosync::owned_fd;
using nosync::shared_fd;
using std::array;
using std::error_code;
using std::experimental::make_array;
using std::function;
using std::generic_category;
using std::make_shared;
using std::make_unique;
using std::move;
using std::size_t;
using std::string;
using std::vector;
using std::weak_ptr;
using testing::_;
using testing::Eq;
using testing::Invoke;


namespace
{

owned_fd make_listening_unix_socket()
{
    int sock_fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sock_fd < 0) {
        return {};
    }

    sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;

    if (::bind(sock_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(sa_family_t)) < 0
        || ::listen(sock_fd, 1) < 0) {
        (void) close(sock_fd);
        return {};
    }

    return owned_fd(sock_fd);
}


owned_fd make_connected_unix_socket(const string &sock_name)
{
    int sock_fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sock_fd < 0) {
        return {};
    }

    sockaddr_un addr = {};

    if (sock_name.size() > sizeof(addr.sun_path)) {
        return {};
    }

    addr.sun_family = AF_UNIX;
    sock_name.copy(addr.sun_path, sock_name.size());

    const auto addr_size = offsetof(decltype(addr), sun_path) + sock_name.size();

    if (::connect(sock_fd, reinterpret_cast<sockaddr *>(&addr), addr_size) < 0) {
        (void) close(sock_fd);
        return {};
    }

    return owned_fd(sock_fd);
}


string get_unix_socket_name(int sock_fd)
{
    sockaddr_un addr = {};

    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock_fd, reinterpret_cast<sockaddr *>(&addr), &addr_len) < 0 || addr_len > sizeof(addr)) {
        return {};
    }

    const auto sun_path_size = addr_len - offsetof(decltype(addr), sun_path);

    return string(addr.sun_path, sun_path_size);
}


int read_nointr(int fd, void *buf, size_t count)
{
    ssize_t read_retval;
    do {
        read_retval = ::read(fd, buf, count);
    } while (read_retval == -1 && errno == EINTR);

    return read_retval;
}


int write_nointr(int fd, const void *buf, size_t count)
{
    ssize_t write_retval;
    do {
        write_retval = ::write(fd, buf, count);
    } while (write_retval == -1 && errno == EINTR);

    return write_retval;
}

}


TEST(NosyncSocketConnectionsFdAcceptor, CheckSingleAccept)
{
    auto listen_sock_fd = make_listening_unix_socket();
    ASSERT_TRUE(listen_sock_fd);
    auto listen_sock_fd_no = *listen_sock_fd;
    auto listen_sock_name = get_unix_socket_name(*listen_sock_fd);
    ASSERT_FALSE(listen_sock_name.empty());

    function<void()> saved_sock_watch_notify_func;
    auto mock_sock_watch_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_sock_watch_handle, disable()).WillOnce(Invoke(
        [&saved_sock_watch_notify_func]() {
            saved_sock_watch_notify_func = nullptr;
        }));

    auto mock_watcher = make_shared<fd_watcher_mock>();
    EXPECT_CALL(*mock_watcher, add_watch_impl(Eq(listen_sock_fd_no), Eq(fd_watch_mode::input), _)).WillOnce(Invoke(
        [&](auto, auto, auto notify_func) {
            saved_sock_watch_notify_func = move(notify_func);
            return move(mock_sock_watch_handle);
        }));

    vector<shared_fd> saved_conn_fds;

    auto acceptor = make_socket_connections_fd_acceptor(
        *mock_watcher, move(listen_sock_fd),
        [&saved_conn_fds](auto conn_fd) {
            saved_conn_fds.push_back(move(conn_fd));
        });

    ASSERT_TRUE(saved_sock_watch_notify_func);
    ASSERT_EQ(saved_conn_fds.size(), 0U);

    auto client_sock_fd = make_connected_unix_socket(listen_sock_name);
    ASSERT_TRUE(client_sock_fd);

    saved_sock_watch_notify_func();
    ASSERT_EQ(saved_conn_fds.size(), 1U);
    ASSERT_TRUE(saved_conn_fds.back());

    array<char, 2> check_read_buf;
    int check_read_retval = read_nointr(*saved_conn_fds.back(), check_read_buf.data(), check_read_buf.size());
    ASSERT_EQ(check_read_retval, -1);
    ASSERT_EQ(errno, EAGAIN);

    const auto sock_data = make_array('\xA9', '\xC1');
    int write_retval = write_nointr(*client_sock_fd, sock_data.data(), sock_data.size());
    ASSERT_EQ(write_retval, sock_data.size());

    array<char, 2> data_read_buf;
    int data_read_retval = read_nointr(*saved_conn_fds.back(), data_read_buf.data(), data_read_buf.size());
    ASSERT_EQ(data_read_retval, data_read_buf.size());
    ASSERT_EQ(sock_data, data_read_buf);

    client_sock_fd = owned_fd();

    array<char, 1> eof_read_buf;
    int eof_read_retval = read_nointr(*saved_conn_fds.back(), eof_read_buf.data(), eof_read_buf.size());
    ASSERT_EQ(eof_read_retval, 0);

    saved_conn_fds.clear();

    acceptor.reset();
}


TEST(NosyncSocketConnectionsFdAcceptor, CheckDestroyHandler)
{
    auto listen_sock_fd = make_listening_unix_socket();
    ASSERT_TRUE(listen_sock_fd);
    auto listen_sock_fd_no = *listen_sock_fd;

    function<void()> saved_sock_watch_notify_func;
    auto mock_sock_watch_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_sock_watch_handle, disable()).WillOnce(Invoke(
        [&saved_sock_watch_notify_func]() {
            saved_sock_watch_notify_func = nullptr;
        }));

    auto mock_watcher = make_shared<fd_watcher_mock>();
    EXPECT_CALL(*mock_watcher, add_watch_impl(Eq(listen_sock_fd_no), Eq(fd_watch_mode::input), _)).WillOnce(Invoke(
        [&](auto, auto, auto notify_func) {
            saved_sock_watch_notify_func = move(notify_func);
            return move(mock_sock_watch_handle);
        }));

    auto conn_handler_flag_ptr = make_shared<bool>(true);
    weak_ptr<bool> conn_handler_flag_wptr = conn_handler_flag_ptr;

    auto acceptor = make_socket_connections_fd_acceptor(
        *mock_watcher, move(listen_sock_fd),
        [conn_handler_flag_ptr](auto) {
            *conn_handler_flag_ptr = false;
        });

    conn_handler_flag_ptr.reset();
    ASSERT_TRUE(conn_handler_flag_wptr.lock());

    acceptor.reset();
    ASSERT_FALSE(conn_handler_flag_wptr.lock());
}


TEST(NosyncSocketConnectionsFdAcceptor, CheckAcceptFailure)
{
    auto listen_sock_fd = make_listening_unix_socket();
    ASSERT_TRUE(listen_sock_fd);
    auto listen_sock_fd_no = *listen_sock_fd;

    function<void()> saved_sock_watch_notify_func;
    auto mock_sock_watch_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_sock_watch_handle, disable()).WillOnce(Invoke(
        [&saved_sock_watch_notify_func]() {
            saved_sock_watch_notify_func = nullptr;
        }));

    auto mock_watcher = make_shared<fd_watcher_mock>();
    EXPECT_CALL(*mock_watcher, add_watch_impl(Eq(listen_sock_fd_no), Eq(fd_watch_mode::input), _)).WillOnce(Invoke(
        [&](auto, auto, auto notify_func) {
            saved_sock_watch_notify_func = move(notify_func);
            return move(mock_sock_watch_handle);
        }));

    auto saved_conn_res = make_error_result<shared_fd>(error_code());

    auto acceptor = make_full_socket_connections_fd_acceptor(
        *mock_watcher, move(listen_sock_fd),
        [&](auto conn_res) {
            saved_conn_res = move(conn_res);
        });

    ASSERT_TRUE(saved_sock_watch_notify_func);
    ASSERT_FALSE(saved_conn_res.is_ok());
    ASSERT_EQ(saved_conn_res.get_error(), error_code());

    saved_sock_watch_notify_func();
    ASSERT_FALSE(saved_conn_res.is_ok());
    ASSERT_EQ(saved_conn_res.get_error(), error_code(EWOULDBLOCK, generic_category()));

    acceptor.reset();
}
