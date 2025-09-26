#pragma once
#include <utility>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <random>
#include <bits/stdc++.h>
#include <vector>

using namespace std;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;


/*------------- CONSTANTS ---------------*/
int no_of_features = 3;
int no_of_users = 3;
int no_of_items = 3;

int64_t PRIME = 69696969; // random numbers will be between 0 and PRIME
/*****************************************/

vector<vector<int64_t>> TEST_U = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

vector<vector<int64_t>> TEST_V = {
    {9, 8, 7},
    {6, 5, 4},
    {3, 2, 1}
};

// ----------------------- Helper coroutines -----------------------
awaitable<void> send_coroutine(tcp::socket& sock, int64_t value) {
    co_await boost::asio::async_write(sock, boost::asio::buffer(&value, sizeof(value)), use_awaitable);
}

awaitable<void> recv_coroutine(tcp::socket& sock, int64_t& out) {
    co_await boost::asio::async_read(sock, boost::asio::buffer(&out, sizeof(out)), use_awaitable);
}

// Receive a vector from a server
awaitable<std::vector<int64_t>> recv_vector(tcp::socket& sock) {
    int64_t size;
    co_await boost::asio::async_read(sock, boost::asio::buffer(&size, sizeof(size)), use_awaitable);
    std::vector<int64_t> result(size);
    if (size > 0) {
        co_await boost::asio::async_read(sock, boost::asio::buffer(result, size * sizeof(int64_t)), use_awaitable);
    }
    co_return result;
}

// Setup connection to P2 (P0/P1 act as clients, P2 acts as server)
awaitable<tcp::socket> setup_server_connection(boost::asio::io_context& io_context, tcp::resolver& resolver) {
    tcp::socket sock(io_context);

    // Connect to P2
    auto endpoints_p2 = resolver.resolve("p2", "9002");
    co_await boost::asio::async_connect(sock, endpoints_p2, use_awaitable);

    co_return sock;
}

// Receive random value from P2 used by the clients P0/P1
awaitable<int64_t> recv_from_P2(tcp::socket& sock) {
    int64_t received;
    co_await recv_coroutine(sock, received);
    co_return received;
}

// Setup peer connection between clients P0 and P1
awaitable<tcp::socket> setup_peer_connection(boost::asio::io_context& io_context, tcp::resolver& resolver) {
    tcp::socket sock(io_context);
#ifdef ROLE_p0
    auto endpoints_p1 = resolver.resolve("p1", "9001");
    co_await boost::asio::async_connect(sock, endpoints_p1, use_awaitable);
#else
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9001));
    sock = co_await acceptor.async_accept(use_awaitable);
#endif
    co_return sock;
}


// Generate random number between 1 and PRIME
inline int64_t random_uint() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<int64_t> dis(1, PRIME);
    return dis(gen);
}

// Receive a matrix from the server socket
awaitable<std::vector<std::vector<int64_t>>> recv_matrix(tcp::socket& sock) {
    // Read dimensions (rows, cols)
    int64_t rows, cols;
    co_await boost::asio::async_read(sock, boost::asio::buffer(&rows, sizeof(rows)), use_awaitable);
    co_await boost::asio::async_read(sock, boost::asio::buffer(&cols, sizeof(cols)), use_awaitable);

    // If either dimension is zero, return an empty matrix immediately
    if (rows == 0 || cols == 0) {
        co_return std::vector<std::vector<int64_t>>();
    }

    // Allocate a buffer to hold the flattened matrix data
    std::vector<int64_t> flattened_data(rows * cols);

    // Read the entire matrix data in a single operation
    co_await boost::asio::async_read(sock, boost::asio::buffer(flattened_data, flattened_data.size() * sizeof(int64_t)), use_awaitable);

    // Reshape the flattened data into a 2D matrix
    std::vector<std::vector<int64_t>> matrix(rows, std::vector<int64_t>(cols));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i][j] = flattened_data[i * cols + j];
        }
    }
    co_return matrix;
}


// Send a matrix to the receiver socket
awaitable<void> send_matrix(tcp::socket& sock, const std::vector<std::vector<int64_t>>& matrix) {
    // Send dimensions (rows, cols) first
    int64_t rows = matrix.size();
    int64_t cols = (rows > 0) ? matrix[0].size() : 0;
    co_await boost::asio::async_write(sock, boost::asio::buffer(&rows, sizeof(rows)), use_awaitable);
    co_await boost::asio::async_write(sock, boost::asio::buffer(&cols, sizeof(cols)), use_awaitable);

    // Flatten the 2D matrix into a 1D vector for a single write operation
    std::vector<int64_t> flattened_data;
    if (rows > 0 && cols > 0) {
        flattened_data.reserve(rows * cols);
        for (const auto& row : matrix) {
            flattened_data.insert(flattened_data.end(), row.begin(), row.end());
        }
    }
    // Send the entire matrix data
    co_await boost::asio::async_write(sock, boost::asio::buffer(flattened_data, flattened_data.size() * sizeof(int64_t)), use_awaitable);
    co_return;
}


// Send a vector to the receiver socket
awaitable<void> send_vector(tcp::socket& sock, const std::vector<int64_t>& vec) {
    int64_t size = vec.size();
    co_await boost::asio::async_write(sock, boost::asio::buffer(&size, sizeof(size)), use_awaitable);
    if (size > 0) {
        co_await boost::asio::async_write(sock, boost::asio::buffer(vec, size * sizeof(int64_t)), use_awaitable);
    }
    co_return;
}

