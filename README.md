## How to run the code:
```unix
	$:> docker-compose build
	$:> docker-compose up
```
  

## How to give inputs?

-  `no_of_users` ~ m, `no_of_features` ~ k, `no_of_items` ~ n should be specified in the header file `common.hpp`

- The initial matrices should be specified in a text file with the name `initial_matrix.hpp`.  A sample matrix is given. The first matrix should be the U matrix with dimensions `m x k` and the next matrix would be V with dimensions `n x k`. Overall, the file should have a matrix of dimension `(m+n) x k` with no spaces in between rows. A sample example is already given in the files.

- The queries will be read from the file named `queries.txt`. Each line would be a pair `(user_index, item_index)`. This file will be read and accessed by P2 alone and will be a secret from P0 and P1.

## How to see outputs?
The following things will be printed:

- The updated U matrix share of P0.

- The updated U matrix share of P1.

- The updated U matrix after adding the above two shares.

  

## Explanation of the pipeline:

**Step (1)**: P2 reads the inputs from the respective files

**Step (2)**: P2 creates additive shares of the U and V matrices

**Step (3)**: For the $i^{th}$ query P2 generates pairs of the form `(ui, vj_share)` to send to each party. `ui` is sent as it is, because its value is public and we have to send of `vj` index as shares because it has to be a secret from P0 and P1. `vj_share` is actually the additive share of standard basis vector `e` where `e[x] = 1` if x = j otherwise it is 0.

**Step (4)**: For each query, do the following steps:

- Here, we have to perform two types of Du-Atallah protocol for dot products and normal multiplications

- Given the share of the matrix $U$ and the user index `ui` we can easily find the share of row vector $U_i$ (which is equal to `U[ui]`)

- Finding row vector $V_j$ is tricky because we have shares of the matrix $V$ and shares of standard basis vectors $e$. In order to find shares of $V_j$, I am computing dot products of columns of $V$ with $e$. We have additive shares of both so we can just perform dot products via MPC using the Du-Atallah protocol. We need to perform k (# of features) dot products to compute all the components of row vector $V_j$. For each dot product I am using fresh shares of random vectors `X0,X1,Y0,Y1` and random values `Z0,Z1` respectively which I am generating in the preprocessing phase in P2. Each vector in the above k dot products will be of length n (# of items).

- After we have shares of $U_i$ and $V_j$ we have to perform dot product of these two vectors using Du-Atallah as well. For this also I have used fresh shares of random vectors each having length k.

**Step (5)**: We have to compute $delta = 1 - <U_i V_j>$. For this, we already have shares of the dot product. We just need shares of $1$, which we obtained from P2 in the preprocessing phase.

**Step (6)**: Now we have shares of scalar value $delta$ and shares of vector $V_j$. We have to perform element-wise MPC multiplication of the above two values. I have used Du Atallah for this as well. Fresh shares of random values, namely `deltaX, deltaY and deltaZ` are sent for each multiplication generated during the preprocessing phase by P2.

**Step (7)**: Add the shares of $delta*V_j$ with shares of $U_i$ and update the matrix $U$.

**Step (8)**: At the end of processing all queries, the updated shares of $U$ are shared back to P2 from P0 and P1. P2 prints the shares and the sum of shares (which is $U$ itself) at the end.

## Security and privacy
We can see that throughout the entire program, the parties are only working with shares of original values, which is known only to party P2. So the entire process is indeed secure.

## Communication

For each query, data sent from `P2`  to `Pb` where b $\in \{0,1\}$:

1.  `ui` → the user index (public value).
2. `vj_share` → share of standard basis vector $e_j$.

3. **Random vector shares (for Du-Atallah dot product protocol)**
For calculation of $V_j$:
   - `Xb` → matrix where `Xb[i]` is the random vector share to calculate $i^{th}$ component of $V_j$.
   - `Yb` → matrix where `Yb[i]` is the random vector share to calculate $i^{th}$ component of $V_j$.
   - `Zb` → vector where `Zb[i]` is the random value share to calculate $i^{th}$ component of $V_j$.
For calculation of $<U_i V_j>$:
   - `Xb_uv_i` → random vector share.
   - `Yb_uv_i` → random vector share.
   - `Zb_uv_i` → random scalar share.

4. **Delta Shares (for Du-Atallah multiplication protocol)**
   - `deltaXb` → vector where `deltaXb[i]` is used to calculate $delta*V_j[i]$.
   - `deltaYb` → vector where `deltaYb[i]` is used to calculate $delta*V_j[i]$.
   - `deltaZb` → vector where `deltaZb[i]` is used to calculate $delta*V_j[i]$.

5. **Share of Constant**
   - `share_of_1_Pb` → additive share of constant `1`.

Data sent between `P0` & `P1` during the Du Atallah protocol:
* `x0 + X0` & `y0 + Y0` from `P0` to `P1`
* `x1 + X1` & `y1 + Y1` from `P1` to `P0`
This needs to be done for every instance of the Du-Atallah protocol. In case of vector dot product above terms are vectors and in case of multiplication the above terms are scalar.