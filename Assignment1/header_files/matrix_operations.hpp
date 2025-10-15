#pragma once
#include <utility>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <random>
#include <bits/stdc++.h>
#include <vector>

#include "common.hpp"

using namespace std;

// Generate standared basis vector of given size with 1 at given index
vector<int64_t> standared_basis_vector(int size, int index){
    vector<int64_t> vec(size,0);
    vec[index] = 1;
    return vec;
}

// Generate random number between 1 and PRIME
vector<int64_t> random_vector(int size){
    vector<int64_t> vec(size);
    for(int i=0;i<size;i++){
        vec[i] = random_uint();
        // DEBUGGING
        // vec[i] = 1;
    }
    return vec;
}

// checks if two vectors are additive shares or not
vector<int64_t> check_Subtraction_vectors(vector<int64_t> vec1, vector<int64_t> vec2){
    int size = vec1.size();
    vector<int64_t> result(size);
    for(int i=0;i<size;i++){
        result[i] = vec1[i] + vec2[i];
        assert(result[i]==0 || result[i]==1);
    }
    return result;
}


// performs element-wise subtraction of two vectors vec1 and vec2
vector<int64_t> SUB_vectors(vector<int64_t> vec1, vector<int64_t> vec2){
    int size = vec1.size();
    vector<int64_t> result(size);
    for(int i=0;i<size;i++){
        result[i] = vec1[i] - vec2[i];
    }
    return result;
}


// create shares of standard basis vector for a given vector
vector<vector<int64_t>> create_standard_basis_vec_shares(int len, int index) {
    vector<int64_t> vec = standared_basis_vector(len, index);
    vector<int64_t> share1 = random_vector(len);
    vector<int64_t> share2 = SUB_vectors(vec, share1);
    check_Subtraction_vectors(share1, share2);
    return {share1, share2};
}

// performs dot product of two vectors A and B
int64_t vector_dot_product(vector<int64_t> A, vector<int64_t> B) {
    int size = A.size();
    assert(B.size() == size);
    int64_t vec = 0;
    for (int i = 0; i < size; ++i) {
        vec += (A[i] * B[i]);
    }
    return vec;
}

// Performs element-wise addition of two matrices A and B
vector<vector<int64_t>> matrix_addition(vector<vector<int64_t>> A, vector<vector<int64_t>> B) {
    int rows = A.size();
    int cols = A[0].size();
    assert(B.size() == rows && B[0].size() == cols);
    vector<vector<int64_t>> C(rows, vector<int64_t>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
}

// performs element-wise XOR of two vectors vec1 and vec2
vector<int64_t> XOR_vectors(vector<int64_t> vec1, vector<int64_t> vec2){
    int size = vec1.size();
    vector<int64_t> result(size);
    for(int i=0;i<size;i++){
        result[i] = vec1[i] ^ vec2[i];
    }
    return result;
}

// check if two vectors are XOR shares or not
vector<int64_t> check_XOR_vectors(vector<int64_t> vec1, vector<int64_t> vec2){
    int size = vec1.size();
    vector<int64_t> result(size);
    for(int i=0;i<size;i++){
        result[i] = vec1[i] ^ vec2[i];
        assert(result[i]==0 || result[i]==1);
    }
    return result;
}

// creates random value matrix of shape rows x cols
// if randomize=1, creates random matrix
// DEBUGGING
// if randomize=2, sends TEST_U
// if randomize=3, sends TEST_V
vector<vector<int64_t>> create_random_matrix(int rows,int cols,int randomize){
    if(randomize!=1){
        return randomize == 2 ? TEST_U : TEST_V;
    }
    vector<vector<int64_t>> matrix(rows,vector<int64_t>(cols));
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            matrix[i][j] = random_uint();
        }
    }
    return matrix;
}

// checks if two matrices are XOR shares or not
void check_XOR_matrices(vector<vector<int64_t>> A, vector<vector<int64_t>> B){
    int rows = A.size();
    int cols = A[0].size();
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            int64_t val = A[i][j] ^ B[i][j];
            assert(val==0);
        }
    }
}

// performs element-wise XOR of two matrices A and B
vector<vector<int64_t>> matrix_XOR(vector<vector<int64_t>> A, vector<vector<int64_t>> B) {
    int rows = A.size();
    int cols = A[0].size();
    vector<vector<int64_t>> C(rows, vector<int64_t>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] ^ B[i][j];
        }
    }
    return C;
}

// performs element-wise subtraction of two matrices A and B
vector<vector<int64_t>> matrix_subtraction(vector<vector<int64_t>> A, vector<vector<int64_t>> B) {
    int rows = A.size();
    int cols = A[0].size();
    vector<vector<int64_t>> C(rows, vector<int64_t>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
}

// checks if two matrices are additive shares or not
// SPECIAL CASE: checks if A+B = matrix of all 0s initialized for debugging purposes
void check_Additive_matrices(vector<vector<int64_t>> A, vector<vector<int64_t>> B){
    int rows = A.size();
    int cols = A[0].size();
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            int64_t val = A[i][j] + B[i][j];
            // assert(val==0);
        }
    }
}


// performs matrix-vector multiplication of matrix A and vector B
vector<int64_t> matrix_vector_multiplication(vector<vector<int64_t>> A, vector<int64_t> B) {
    int rows = A.size();
    int cols = A[0].size();
    assert(B.size() == cols);
    vector<int64_t> C(rows, 0);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i] += (A[i][j] * B[j]);
        }
    }
    return C;
}

vector<int64_t> vector_addition(vector<int64_t> A, vector<int64_t> B) {
    int size = A.size();
    assert(B.size() == size);
    vector<int64_t> C(size);
    for (int i = 0; i < size; ++i) {
        C[i] = A[i] + B[i];
    }
    return C;
}

vector<vector<int64_t>> matrix_transpose(vector<vector<int64_t>> A) {
    int rows = A.size();
    int cols = A[0].size();
    vector<vector<int64_t>> At(cols, vector<int64_t>(rows));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            At[j][i] = A[i][j];
        }
    }
    return At;
}

// Performs MPC dot product of two vectors vec1 and vec2
awaitable<int64_t> mpc_dot_product(vector<int64_t> vec1, vector<int64_t> vec2, vector<int64_t> X, vector<int64_t> Y, int64_t Z, tcp::socket& peer_socket) {

    vector<int64_t> Xtilde = vector_addition(vec1, X);
    vector<int64_t> Ytilde = vector_addition(vec2, Y);

    // Send Xtilde and Ytilde to peer and receive peer's Xtilde and Ytilde
    co_await send_vector(peer_socket, Xtilde);
    co_await send_vector(peer_socket, Ytilde);

    vector<int64_t> Xtilde_peer = co_await recv_vector(peer_socket);
    vector<int64_t> Ytilde_peer = co_await recv_vector(peer_socket);


    assert(Xtilde_peer.size() == Xtilde.size());
    assert(Ytilde_peer.size() == Ytilde.size());

    int64_t x_dot_y_plus_Ytilde_peer = vector_dot_product(vec1,vector_addition(vec2, Ytilde_peer));
    int64_t Y_dot_Xtilde_peer = vector_dot_product(Y,Xtilde_peer);
    int64_t U_row_dot_V_row_share = x_dot_y_plus_Ytilde_peer - Y_dot_Xtilde_peer + Z;
    co_return U_row_dot_V_row_share;
}

// Fetch a specific column from a matrix
vector<int64_t> fetch_column_from_matrix(vector<vector<int64_t>> matrix, int col_index) {
    int rows = matrix.size();
    vector<int64_t> column(rows);
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i][col_index];
    }
    return column;
}

// Performs MPC multiplication of two values x and y
awaitable<int64_t> mpc_multiplication(int64_t x, int64_t y, int64_t X, int64_t Y, int64_t Z, tcp::socket& peer_socket) {
    int64_t X_tilde = X + x;
    int64_t Y_tilde = Y + y;

    // Send a_plus_x and b_plus_y to peer and receive peer's a_plus_x and b_plus_y
    co_await send_coroutine(peer_socket, X_tilde);
    co_await send_coroutine(peer_socket, Y_tilde);

    int64_t X_tilde_peer;
    int64_t Y_tilde_peer;
    co_await recv_coroutine(peer_socket, X_tilde_peer);
    co_await recv_coroutine(peer_socket, Y_tilde_peer);

    int64_t product_share = x * (y + Y_tilde_peer) - Y * X_tilde_peer + Z;
    co_return product_share;
}
