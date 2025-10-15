#include "header_files/common.hpp"
#include "header_files/matrix_operations.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <random>


using boost::asio::ip::tcp;


// Run multiple coroutines in parallel
template <typename... Fs>
void run_in_parallel(boost::asio::io_context& io, Fs&&... funcs) {
    (boost::asio::co_spawn(io, funcs, boost::asio::detached), ...);
}

// Read initial U and V matrices from input file
vector<vector<vector<int64_t>>> read_data_from_file(const std::string& filename) {
    std::ifstream fin(filename);
    vector<vector<int64_t>> U_data(no_of_users, vector<int64_t>(no_of_features));
    vector<vector<int64_t>> V_data(no_of_items, vector<int64_t>(no_of_features));

    for (int i = 0; i < no_of_users; i++) {
        for (int j = 0; j < no_of_features; j++) {
            fin >> U_data[i][j];
        }
    }

    for (int i = 0; i < no_of_items; i++) {
        for (int j = 0; j < no_of_features; j++) {
            fin >> V_data[i][j];
        }
    }

    fin.close();
    return {U_data, V_data};
}

// Read queries from input file
vector<pair<int,int>> read_queries(const std::string& filename) {
    std::ifstream fin(filename);
    vector<pair<int,int>> queries;
    while(!fin.eof()){
        int user_index,item_index;
        fin>>user_index>>item_index;
        assert(user_index>=1 && user_index<=no_of_users);
        assert(item_index>=1 && item_index<=no_of_items);
        queries.emplace_back(user_index-1, item_index-1);
    }
    fin.close();
    return queries;
}

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9002));

        // Accept clients
        tcp::socket socket_p0(io_context);
        acceptor.accept(socket_p0);

        tcp::socket socket_p1(io_context);
        acceptor.accept(socket_p1);

        // create the user matrix U with dimensions m(# of users) x k(# of features)
        std::vector<std::vector<std::vector<int64_t>>> file_data = read_data_from_file("inputs/initial_matrix.txt");
        std::vector<std::vector<int64_t>> U = file_data[0];
        std::vector<std::vector<int64_t>> U_0 = create_random_matrix(no_of_users, no_of_features,1);
        std::vector<std::vector<int64_t>> U_1 = matrix_subtraction(U, U_0);

        // create the item matrix V with dimensions n(# of items) x k(# of features)
        std::vector<std::vector<int64_t>> V = file_data[1];
        std::vector<std::vector<int64_t>> V_0 = create_random_matrix(no_of_items, no_of_features,1);
        std::vector<std::vector<int64_t>> V_1 = matrix_subtraction(V, V_0);

        // load queries from the file "queries.txt"
        vector<pair<int,int>> queries = read_queries("inputs/queries.txt");

        vector<pair<int,vector<int64_t>>> u_v0share_pairs;
        vector<pair<int,vector<int64_t>>> u_v1share_pairs;

        // Each X0_set[i] = X0 for query i which is again a list of shares for doing k(# of features) dot products
        // similar for other sets
        vector<vector<vector<int64_t>>> X0_set, Y0_set, X1_set, Y1_set;
        vector<vector<int64_t>> Z0_set, Z1_set;

        vector<vector<int64_t>> X0_uv, Y0_uv, X1_uv, Y1_uv;
        vector<int64_t> Z0_uv, Z1_uv;

        vector<vector<int64_t>> delta_X0_set, delta_Y0_set, delta_X1_set, delta_Y1_set, delta_Z0_set, delta_Z1_set;
        
        // GENSHARES
        // generate random shares for Du Attalah vector dot product protocol and multiplication protocol
        for (const auto& [user_index, item_index]  : queries) {
            
            vector<vector<int64_t>> X0_store, Y0_store, X1_store, Y1_store;
            vector<int64_t> Z0_store, Z1_store;
            vector<int64_t> deltaX0_store, deltaY0_store, deltaX1_store, deltaY1_store;
            vector<int64_t> deltaZ0_store, deltaZ1_store;

            // For the k dot products between ith column of V and share of standared basis vector in order to obtain V_row
            for(int i=0;i<no_of_features;i++){
                vector<int64_t> X0 = random_vector(no_of_items);
                vector<int64_t> X1 = random_vector(no_of_items);
                vector<int64_t> Y0 = random_vector(no_of_items);
                vector<int64_t> Y1 = random_vector(no_of_items);
                int64_t T = random_uint();

                int64_t Z0 = vector_dot_product(X0, Y1) + T;
                int64_t Z1 = vector_dot_product(X1, Y0) - T;

                X0_store.push_back(X0);
                X1_store.push_back(X1);
                Y0_store.push_back(Y0);
                Y1_store.push_back(Y1);
                Z0_store.push_back(Z0);
                Z1_store.push_back(Z1);
            }

            // For the final dot product between U_row and V_row
            vector<int64_t> X0 = random_vector(no_of_features);
            vector<int64_t> X1 = random_vector(no_of_features);
            vector<int64_t> Y0 = random_vector(no_of_features);
            vector<int64_t> Y1 = random_vector(no_of_features);
            int64_t T = random_uint();

            int64_t Z0 = vector_dot_product(X0, Y1) + T;
            int64_t Z1 = vector_dot_product(X1, Y0) - T;

            X0_uv.push_back(X0);
            X1_uv.push_back(X1);
            Y0_uv.push_back(Y0);
            Y1_uv.push_back(Y1);
            Z0_uv.push_back(Z0);
            Z1_uv.push_back(Z1);

            for (int i = 0; i < no_of_features; i++) {
                int64_t deltaX0 = random_uint();
                int64_t deltaY0 = random_uint();
                int64_t deltaX1 = random_uint();
                int64_t deltaY1 = random_uint();
                int64_t alpha = random_uint();
                
                int64_t deltaZ0 = deltaX0 * deltaY1 + alpha;
                int64_t deltaZ1 = deltaX1 * deltaY0 - alpha;
                
                deltaX0_store.push_back(deltaX0);
                deltaY0_store.push_back(deltaY0);
                deltaX1_store.push_back(deltaX1);
                deltaY1_store.push_back(deltaY1);
                deltaZ0_store.push_back(deltaZ0);
                deltaZ1_store.push_back(deltaZ1);
            }

            // put in the set
            X0_set.push_back(X0_store);
            X1_set.push_back(X1_store);
            Y0_set.push_back(Y0_store);
            Y1_set.push_back(Y1_store);
            Z0_set.push_back(Z0_store);
            Z1_set.push_back(Z1_store);


            delta_X0_set.push_back(deltaX0_store);
            delta_X1_set.push_back(deltaX1_store);
            delta_Y0_set.push_back(deltaY0_store);
            delta_Y1_set.push_back(deltaY1_store);
            delta_Z0_set.push_back(deltaZ0_store);
            delta_Z1_set.push_back(deltaZ1_store);


            vector<vector<int64_t>> v_share = create_standard_basis_vec_shares(no_of_items, item_index);
            u_v0share_pairs.emplace_back(user_index, v_share[0]);
            u_v1share_pairs.emplace_back(user_index, v_share[1]);
        }

        vector<int64_t> share_of_1_P0 = random_vector(queries.size());
        vector<int64_t> share_of_1_P1;
        for(int i=0;i<queries.size();i++){
            share_of_1_P1.push_back(1 - share_of_1_P0[i]);
        }

        std::vector<std::vector<int64_t>> U_from_p0, U_from_p1;

        run_in_parallel(io_context,
            [&]() -> boost::asio::awaitable<void> {
                // send the matrix U_0 and V_0 to P0
                co_await send_matrix(socket_p0, U_0);
                co_await send_matrix(socket_p0, V_0);

                // send # of queries to P0
                int64_t num_queries = queries.size();
                co_await boost::asio::async_write(socket_p0, boost::asio::buffer(&num_queries, sizeof(num_queries)), use_awaitable);

                // send the user index and item share vector to P0
                for (int i=0;i<queries.size();i++) {

                    const auto& [user_index, item_share]  = u_v0share_pairs[i];

                    const auto& X0 = X0_set[i];
                    const auto& Y0 = Y0_set[i];
                    const auto& Z0 = Z0_set[i];

                    const auto& X0_uv_i = X0_uv[i];
                    const auto& Y0_uv_i = Y0_uv[i];
                    const auto& Z0_uv_i = Z0_uv[i];
                    

                    const auto& deltaX0 = delta_X0_set[i];
                    const auto& deltaY0 = delta_Y0_set[i];
                    const auto& deltaZ0 = delta_Z0_set[i];

                    // Send user index to p0 because it is public
                    int64_t u_idx = user_index;
                    co_await boost::asio::async_write(socket_p0, boost::asio::buffer(&u_idx, sizeof(u_idx)), use_awaitable);
                    // Send item share vector to p0
                    co_await send_vector(socket_p0, item_share);
                    
                    // Send random vector shares for Du Attalah vector dot product protocol
                    co_await send_matrix(socket_p0, X0);
                    co_await send_matrix(socket_p0, Y0);
                    co_await send_vector(socket_p0, Z0);
                    
                    co_await send_vector(socket_p0, X0_uv_i);
                    co_await send_vector(socket_p0, Y0_uv_i);
                    co_await boost::asio::async_write(socket_p0, boost::asio::buffer(&Z0_uv_i, sizeof(Z0_uv_i)), use_awaitable);


                    // Send delta shares for Du Attalah multiplication protocol
                    co_await send_vector(socket_p0, deltaX0);
                    co_await send_vector(socket_p0, deltaY0);
                    co_await send_vector(socket_p0, deltaZ0);

                    // send share of 1 to P0
                    co_await boost::asio::async_write(socket_p0, boost::asio::buffer(&share_of_1_P0[i], sizeof(share_of_1_P0[i])), use_awaitable);
                }
                // get the shares of updated U matrix from P0
                U_from_p0 = co_await recv_matrix(socket_p0);
            },
            [&]() -> boost::asio::awaitable<void> {
                // send the matrix U_1 and V_1 to P1
                co_await send_matrix(socket_p1, U_1);
                co_await send_matrix(socket_p1, V_1);

                // send # of queries to P1
                int64_t num_queries = queries.size();
                co_await boost::asio::async_write(socket_p1, boost::asio::buffer(&num_queries, sizeof(num_queries)), use_awaitable);

                // send the user index and item share vector to P1
                for (int i=0;i<queries.size();i++) {

                    const auto& [user_index, item_share]  = u_v1share_pairs[i];

                    const auto& X1 = X1_set[i];
                    const auto& Y1 = Y1_set[i];
                    const auto& Z1 = Z1_set[i];

                    const auto& X1_uv_i = X1_uv[i];
                    const auto& Y1_uv_i = Y1_uv[i];
                    const auto& Z1_uv_i = Z1_uv[i];

                    const auto& deltaX1 = delta_X1_set[i];
                    const auto& deltaY1 = delta_Y1_set[i];
                    const auto& deltaZ1 = delta_Z1_set[i];
                    

                    int64_t u_idx = user_index;
                    co_await boost::asio::async_write(socket_p1, boost::asio::buffer(&u_idx, sizeof(u_idx)), use_awaitable);
                    co_await send_vector(socket_p1, item_share);

                    // Send random vector shares for Du Attalah vector dot product protocol
                    co_await send_matrix(socket_p1, X1);
                    co_await send_matrix(socket_p1, Y1);
                    co_await send_vector(socket_p1, Z1);

                    
                    co_await send_vector(socket_p1, X1_uv_i);
                    co_await send_vector(socket_p1, Y1_uv_i);
                    co_await boost::asio::async_write(socket_p1, boost::asio::buffer(&Z1_uv_i, sizeof(Z1_uv_i)), use_awaitable);
                    
                    // send random multiplication shares for Du Attalah multiplication protocol
                    co_await send_vector(socket_p1, deltaX1);
                    co_await send_vector(socket_p1, deltaY1);
                    co_await send_vector(socket_p1, deltaZ1);

                    // send share of 1 to P1
                    co_await boost::asio::async_write(socket_p1, boost::asio::buffer(&share_of_1_P1[i], sizeof(share_of_1_P1[i])), use_awaitable);
                }
                // get the shares of updated U matrix from P1
                U_from_p1 = co_await recv_matrix(socket_p1);
            }
        );

        io_context.run();

        // Print the final U matrix from both the parties
        std::cout << "\nFinal share of U matrix from P0:\n";
        for (const auto& row : U_from_p0) {
            for (const auto& val : row) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n\nFinal share of U matrix from P1:\n";
        for (const auto& row : U_from_p1) {
            for (const auto& val : row) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        
        // Add the shares of the updated U matrix received from P0 and P1
        std::vector<std::vector<int64_t>> U_final = matrix_addition(U_from_p0, U_from_p1);

        // Print the final U matrix
        std::cout << "\nFinal U matrix after adding both the shares:\n";
        for (const auto& row : U_final) {
            for (const auto& val : row) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        std::cout << "Adios from P2. ;)\n";

    } catch (std::exception& e) {
        std::cerr << "Exception in P2: " << e.what() << "\n";
    }
}
