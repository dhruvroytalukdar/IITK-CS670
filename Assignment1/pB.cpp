#include "header_files/common.hpp"
#include "header_files/matrix_operations.hpp"

#if !defined(ROLE_p0) && !defined(ROLE_p1)
#error "ROLE must be defined as ROLE_p0 or ROLE_p1"
#endif


// Function to perform a single query (not used in current code)
awaitable<vector<int64_t>> perform_query(
                        std::vector<std::vector<int64_t>>& U_share,
                        std::vector<std::vector<int64_t>>& V_share,
                        int64_t user_index,
                        const std::vector<int64_t>& item_share,
                        const std::vector<vector<int64_t>>& X,
                        const std::vector<vector<int64_t>>& Y,
                        const std::vector<int64_t>& Z,
                        const std::vector<int64_t>& X_uv,
                        const std::vector<int64_t>& Y_uv,
                        int64_t Z_uv,
                        const std::vector<int64_t>& deltaX,
                        const std::vector<int64_t>& deltaY,
                        const std::vector<int64_t>& deltaZ,
                        tcp::socket& peer_socket,
                        tcp::socket& server_socket
                    ) {
    int k = no_of_features;
    std::vector<int64_t> U_row = U_share[user_index];
    assert(U_row.size() == k);

    assert(X.size() == k);
    assert(Y.size() == k);
    assert(Z.size() == k);


    std::vector<int64_t> V_row;
    for(int i=0;i<k;i++){
        vector<int64_t> col = fetch_column_from_matrix(V_share, i);
        assert(col.size() == item_share.size());

        int64_t dot_product = co_await mpc_dot_product(col, item_share, X[i], Y[i], Z[i], peer_socket);
        V_row.push_back(dot_product);
    }

    assert(V_row.size() == k);

    assert(U_row.size() == k);
    assert(V_row.size() == k);
    assert(X_uv.size() == k);
    assert(Y_uv.size() == k);

    int64_t U_row_dot_V_row_share = co_await mpc_dot_product(U_row, V_row, X_uv, Y_uv, Z_uv, peer_socket);

    // recieve share of 1 from p2
    int64_t share_of_1;
    co_await recv_coroutine(server_socket, share_of_1);

    int64_t delta = share_of_1 - U_row_dot_V_row_share;

    vector<int64_t> V_row_mult_delta;
    for (int i = 0; i < k; i++) {
        V_row_mult_delta.push_back(co_await mpc_multiplication(V_row[i], delta, deltaX[i], deltaY[i], deltaZ[i], peer_socket));
    }

    // cout << "V_row multiplied by delta: ";
    // for (const auto& val : V_row_mult_delta) {
    //     cout << val << " ";
    // }
    // cout << endl;
    // assert(V_row_mult_delta.size() == k);
    // cout << U_row.size() << " " << V_row_mult_delta.size() << endl;
    vector<int64_t> result = vector_addition(V_row_mult_delta, U_row);
    for (int i = 0; i < result.size(); i++) {
        U_share[user_index][i] = result[i];
    }
    assert(result.size() == k);
    co_return result;
}

// ----------------------- Main protocol -----------------------
awaitable<void> run(boost::asio::io_context& io_context) {
    tcp::resolver resolver(io_context);

    // Step 1: connect to P2 and receive random value
    tcp::socket server_sock = co_await setup_server_connection(io_context, resolver);

    std::vector<std::vector<int64_t>> U = co_await recv_matrix(server_sock);
    std::vector<std::vector<int64_t>> V = co_await recv_matrix(server_sock);

    int64_t num_queries;
    co_await recv_coroutine(server_sock, num_queries);
    
    tcp::socket peer_sock = co_await setup_peer_connection(io_context, resolver);

    for (int64_t q = 0; q < num_queries; ++q) {
        int64_t user_index;
        co_await recv_coroutine(server_sock, user_index);
        std::vector<int64_t> item_share = co_await recv_vector(server_sock);

        std::vector<vector<int64_t>> X = co_await recv_matrix(server_sock);
        std::vector<vector<int64_t>> Y = co_await recv_matrix(server_sock);
        std::vector<int64_t> Z = co_await recv_vector(server_sock);

        std::vector<int64_t> X_uv = co_await recv_vector(server_sock);
        std::vector<int64_t> Y_uv = co_await recv_vector(server_sock);
        int64_t Z_uv;
        co_await recv_coroutine(server_sock, Z_uv);

        std::vector<int64_t> deltaX, deltaY, deltaZ;
        deltaX = co_await recv_vector(server_sock);
        deltaY = co_await recv_vector(server_sock);
        deltaZ = co_await recv_vector(server_sock);


        co_await perform_query(U, V, user_index, item_share, X, Y, Z, X_uv, Y_uv, Z_uv, deltaX, deltaY, deltaZ, peer_sock, server_sock);
    }

    co_await send_matrix(server_sock, U);
    co_return;
}

int main() {
    std::cout.setf(std::ios::unitbuf); // auto-flush cout for Docker logs
    boost::asio::io_context io_context(1);
    co_spawn(io_context, run(io_context), boost::asio::detached);
    io_context.run();
    return 0;
}
