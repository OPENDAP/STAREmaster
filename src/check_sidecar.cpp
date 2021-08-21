//
// Created by Ed Hartnett on 8/21/21.
//
// Utility to check correctness of a STARE sidecar file.

#include "config.h"

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <bitset>

using namespace std;

void usage(char *name) {
    cout
    << "STARE sidecare check utility. " << endl
    << "Usage: " << name << " [options] sidecar_file " << endl
    << "Examples:" << endl
    << "  " << name << " MOD05_L2.A2021232.1600.061.2021233022815_stare.nc" << endl
    << endl
    << "Options:" << endl
    << " -h, --help        : print this help" << endl
    << " -v, --verbose     : verbose: print all" << endl;

    exit(0);
};

struct Arguments {
    bool verbose = false;
    int err_code = 0;
};

Arguments parseArguments(int argc, char *argv[]) {
    if (argc == 1) usage(argv[0]);
    Arguments arguments;
    static struct option long_options[] = {
            {"help",             no_argument,       nullptr, 'h'},
            {"verbose",          no_argument,       nullptr, 'v'},
            {nullptr,           0,         nullptr, 0}
    };

    int long_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hvf:sb", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
            case 'v':
                arguments.verbose = true;
                break;
            default:
                usage(argv[0]);
        }
    }

    return arguments;
};

int main(int argc, char *argv[])
{
    Arguments arg = parseArguments(argc, argv);

    // Input STARE sidecar file must be provided.
    if (!argv[optind]) {
        cerr << "Must provide STARE sidecar filen name." << endl;
        return 1;
    }

    if (arg.err_code) {
        return arg.err_code;
    }

    return 0;
}
