#include <errno.h>
#include <stdio.h>

int copy(const char* n1, const char* n2)
{
    FILE *f1, *f2;
    f1 = fopen(n1, "r");
    if (!f1) {
        return errno;
    }
    f2 = fopen(n2, "w+");
    if (!f2) {
        return errno;
    }

    char buffer[64];
    fread(buffer, sizeof(char), 64, f1);
    fprintf(f2, "%s", buffer);

    fclose(f1);
    fclose(f2);

    return 0;
}

int main()
{
    fprintf(stdout, "Hello C World!\n");

    FILE *f = fopen("test.txt", "w+");
    if (!f) {
        fprintf(stderr, "Failed to open file (%d)\n", errno);
        return -1;
    }
    fprintf(stdout, "Wrote to file...\n");
    fprintf(f, "This is a simple test\n");
    if (fclose(f) != 0) {
        fprintf(stderr, "Failed to close file (%d)\n", errno);
        return -2;
    }
    if (copy("test.txt", "test2.txt") != 0) {
        fprintf(stderr, "Failed to close file (%d)\n", errno);
        return -3;
    }
    return 0;
}