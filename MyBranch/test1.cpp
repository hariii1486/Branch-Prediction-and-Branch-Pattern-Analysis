#include <iostream>
using namespace std;

// Function for direct call (jal)
int add(int a, int b) {
    return a + b;
}

// Function for indirect call (jalr)
int mul(int a, int b) {
    return a * b;
}

int main() {
    int x = 5, y = 10;

    // 🔹 Conditional branch (if-else)
    if (x < y) {
        cout << "x < y\n";
    } else {
        cout << "x >= y\n";
    }

    // 🔹 Loop (conditional branch repeated)
    for (int i = 0; i < 3; i++) {
        cout << "Loop: " << i << endl;
    }

    // 🔹 Direct function call → jal
    int s = add(x, y);
    cout << "Sum: " << s << endl;

    // 🔹 Indirect function call → jalr
    int (*fp)(int, int) = mul;
    int m = fp(x, y);
    cout << "Mul: " << m << endl;

    // 🔹 Unconditional branch (goto)
    goto label;

    cout << "This won't run\n";

label:
    cout << "Reached label\n";

    return 0;
}