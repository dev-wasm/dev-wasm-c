#include <stdio.h>

class WasmWasi {
    public:
        WasmWasi() {}

        void print() {
            printf("Hello C++ World!\n");
        }
};

int main(int argc, char** argv)
{
    WasmWasi w;
    w.print();
}