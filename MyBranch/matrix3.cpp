#include <iostream>
using namespace std;

int main() {
    const int N = 20;
    int A[N][N], B[N][N], C[N][N];

    // initialize matrices
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + j) % 10;
            B[i][j] = (i * j) % 10;
            C[i][j] = 0;
        }
    }

    // multiplication
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return 0;
}