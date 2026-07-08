#include <iostream>
#include <vector>
#include <queue>
using namespace std;

int main() {
    const int N = 10;

    vector<vector<int>> adj(N);

    // create a small graph
    for (int i = 0; i < N; i++) {
        if (i + 1 < N) adj[i].push_back(i + 1);
        if (i + 2 < N) adj[i].push_back(i + 2);
    }

    vector<bool> visited(N, false);
    queue<int> q;

    q.push(0);
    visited[0] = true;

    while (!q.empty()) {
        int node = q.front();
        q.pop();

        for (int neigh : adj[node]) {
            if (!visited[neigh]) {
                visited[neigh] = true;
                q.push(neigh);
            }
        }
    }

    return 0;
}