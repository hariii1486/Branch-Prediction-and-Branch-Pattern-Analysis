#include <iostream>
using namespace std;

// jal
void foo(int n) {
    if (n % 2 == 0) {
        cout << "Even\n";
    } else {
        cout << "Odd\n";
    }
}

// jalr
void bar(int n) {
    cout << "Bar: " << n << endl;
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};

    // 🔹 Loop + conditional branches
    for (int i = 0; i < 5; i++) {
        if (arr[i] % 2 == 0) {
            cout << arr[i] << " is even\n";
        } else {
            cout << arr[i] << " is odd\n";
        }
    }

    // 🔹 Direct calls (jal)
    foo(10);
    foo(7);

    // 🔹 Indirect calls (jalr)
    void (*func_ptr)(int);
    func_ptr = bar;
    func_ptr(100);

    // 🔹 While loop (conditional branch)
    int x = 3;
    while (x > 0) {
        cout << "x = " << x << endl;
        x--;
    }

    // 🔹 Switch (multiple conditional branches)
    int val = 2;
    switch (val) {
        case 1:
            cout << "One\n";
            break;
        case 2:
            cout << "Two\n";
            break;
        default:
            cout << "Other\n";
    }

    // 🔹 Unconditional branch via goto
    goto end;

    cout << "Skipped\n";

end:
    cout << "Done\n";

    return 0;
}