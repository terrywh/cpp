#include <iostream>

extern int core_test(int argc, char* argv[]);
extern int encoding_test(int argc, char* argv[]);
extern int hash_test(int argc, char* argv[]);
extern int net_test(int argc, char* argv[]);
extern int strconv_test(int argc, char* argv[]);
extern int time_test(int argc, char* argv[]);
extern int os_test(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    core_test(argc, argv);
    encoding_test(argc, argv);
    hash_test(argc, argv);
    strconv_test(argc, argv);
    time_test(argc, argv);
    os_test(argc, argv);
    net_test(argc, argv);
    return 0;
}
