#include <iostream>
#include <fstream>

class WasmWasi {
    public:
        WasmWasi() {}

        void print()
        {
            std::cout << "Hello C++ World!" << std::endl;
        }

        bool write(const std::string& filename)
        {
            std::ofstream file;
            file.open(filename);
            file << "this is a test" << std::endl;
            file.close();

            return file.good();
        }

        bool copy(const std::string& from, const std::string& to)
        {
            std::ifstream in;
            std::ofstream out;
            size_t len;
            char* buff;
            
            out.open(to, std::ofstream::binary);
            in.open(from, std::ofstream::binary);

            in.seekg(0, std::ifstream::end);
            len = in.tellg();
            in.seekg(0, std::ifstream::beg);

            buff = new char[len];
            in.read(buff, len);

            out.write(buff, len);

            in.close();
            out.close();
            delete[] buff;

            return in.good() && out.good();
        }
};

int main(int argc, char** argv)
{
    WasmWasi w;
    w.print();
    if (!w.write("test-cc.txt")) {
        std::cerr << "Failed to write file" << std::endl;
        return -1;
    }
    std::cout << "Wrote file" << std::endl;
    if (!w.copy("test-cc.txt", "test-cc-2.txt"))
    {
        std::cerr << "Failed to copy file" << std::endl;
        return -2;
    }
    std::cout << "Copied file." << std::endl;
}