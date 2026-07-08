#include <iostream>
using namespace std;

int main() {
    const int n = 50;
    int arr[n];

    // Reverse sorted (good branch activity)
    for (int i = 0; i < n; i++) {
        arr[i] = n - i;
    }

    // Bubble sort
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    return 0;
}