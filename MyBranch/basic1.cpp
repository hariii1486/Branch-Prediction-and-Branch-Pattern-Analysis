#include <iostream>
using namespace std;

int main() {
    int state = 0;

    for (int i = 0; i < 100000; i++) {

        if (state == 0 || state == 3)
            asm volatile("");

        state = (state * 5 + 1) % 7;
    }
}