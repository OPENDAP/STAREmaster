// This is the main program to create the sidecar files.
//
// Ed Hartnett 3/14/2020
//
// Renamed and hacked. The new name is easier for me to type.
// The hack enables the command to be run from withing a docker
// container and access files in the parent computer's file system,
// assuming that file system has been mounted on /parent in the
// container.

#include "config.h"

#include <getopt.h>

#include <STARE.h>
#include "VarStr.h"

#include "ssc.h"
#include "Modis05L2GeoFile.h"
#include "Modis09L2GeoFile.h"
#include "Modis09GAGeoFile.h"
#include "SidecarFile.h"

#define DOCKER_PREFIX "/parent"

using namespace std;

void usage(char *name) {
    cout
        << "STARE spatial create sidecar file. " << endl
        << "Usage: " << name << " [options] filename " << endl
        << "Examples:" << endl
        << "  " << name << " data.nc" << endl
        << "  " << name << " data.h5" << endl
        << endl
        << "Options:" << endl
        << "  " << " -h, --help        : print this help" << endl
        << "  " << " -v, --verbose     : verbose: print all" << endl
        << "  " << " -b, --build_level : Higher levels -> longer initialization time. (default is 5)" << endl
        << "  " << " -c, --cover_level    : Cover resolution, level 10 ~ 10 km." << endl
        << "  " << " -g, --use_gring      : Use GRING data to construct cover (default)" << endl
        << "  " << " -w, --walk_perimeter : Provide stride and walk perimeter to construct cover (more accurate)"
        << endl
        << "  " << " -d, --data_type   : Allows specification of data type." << endl
        << "  " << " -i, --institution : Institution where sidecar file is produced." << endl
        << "  " << " -o, --output_file : Provide file name for output file." << endl
        << "  " << " -r, --output_dir  : Provide output directory name." << endl
        << "  " << " -D, --docker      : This is running in a docker container, access files via a 'prefix'" << endl
        << "  " << "                     that maps the parent computer's file system into the container. The" << endl
        << "  " << "                     must be mounted read/write." << endl
        << endl;
    exit(0);
};

struct Arguments {
    bool verbose = false;
    int build_level = SSC_DEFAULT_BUILD_LEVEL;
    int cover_level = -1;
    bool cover_gring = false;
    int stride = -1; // if stride > 0, then we're walking the perimeter and cover_gring = false.
    char data_type[SSC_MAX_NAME] = "";
    char institution[SSC_MAX_NAME] = "";
    char output_file[SSC_MAX_NAME] = "";
    char output_dir[SSC_MAX_NAME] = "";
    char docker[SSC_MAX_NAME] = "";
    int err_code = 0;
};

Arguments parseArguments(int argc, char *argv[]) {
    if (argc == 1) usage(argv[0]);
    Arguments arguments;
    static struct option long_options[] = {
            {"help",             no_argument,       0, 'h'},
            {"verbose",          no_argument,       0, 'v'},
            {"build_level",      required_argument, 0, 'b'},
            {"cover_level",      required_argument, 0, 'c'},
            {"use_gring",        no_argument,       0, 'g'},
            {"walk_perimeter",   required_argument, 0, 'w'},
            {"data_type",        required_argument, 0, 'd'},
            {"institution",      required_argument, 0, 'i'},
            {"output_file",      required_argument, 0, 'o'},
            {"output_directory", required_argument, 0, 'r'},
            {"docker",           required_argument, 0, 'D'},
            {0,                  0,        0, 0}
    };

    int long_index = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "hvqb:c:gw:d:o:r:i:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
            case 'v':
                arguments.verbose = true;
                break;
            case 'b':
                arguments.build_level = atoi(optarg);
                break;
            case 'c':
                arguments.cover_level = atoi(optarg);
                break;
            case 'g':
                arguments.cover_gring = true;
                break;
            case 'w':
                arguments.stride = atoi(optarg);
                break;
            case 'd':
                strcpy(arguments.data_type, optarg);
                break;
            case 'i':
                strcpy(arguments.institution, optarg);
                break;
            case 'o':
                strcpy(arguments.output_file, optarg);
                break;
            case 'r':
                strcpy(arguments.output_dir, optarg);
                break;
            case 'D':
                strcpy(arguments.docker, optarg);
                break;
        }
    }

    // Check for argument consistency.
    if (!arguments.cover_gring) {
        if (arguments.stride <= 0) {
            arguments.cover_gring = true;
        }
    }
    else {
        if (arguments.stride > 0) { // error case, both gring and walk are set
            cerr << "Incompatible arguments. Both perimeter walk (-w) and gring (-g) STARE covers requested.\n";
            arguments.err_code = 99;
        }
    }

    return arguments;
};

/**
 * Pick an output filename for the STARE index file, including output
 * directory.
 *
 * @param in_file Name of the data file.
 * @param output_dir If present, the name of the output directory
 * for the STARE index sidecar file.
 * @return The path and name of the sidecar file.
 */
string
pickOutputName(const string &in_file, const string &output_dir = "") {
    // Do we want this in a different directory?
    if (!output_dir.empty()) {
        size_t f = in_file.rfind("/");
        if (f != string::npos)
            return output_dir + in_file.substr(f, string::npos);
        else
            return output_dir + in_file;
    }

    return in_file;
}

int
main(int argc, char *argv[]) {
    Arguments arg = parseArguments(argc, argv);

    const string MOD09 = "MOD09";
    const string MOD09GA = "MOD09GA";
    const string SIN_TABLE = "sn_bound_10deg.txt";

    // Input file must be provided.
    if (!argv[optind]) {
        cerr << "Must provide input file.\n";
        return SSC_EINPUT;
    }

    if (arg.err_code) {
        return arg.err_code;
    }

    // Docker command hack. jhrg 8/20/21
    string in_file = argv[optind];
    string output_dir = arg.output_dir;
    string output_file = arg.output_file;
    string docker = arg.docker;
    if (!docker.empty()) {
        in_file = docker.append("/").append(in_file);
        // output_dir = docker.append("/").append(output_dir);
        // output_file = docker.append("/").append(output_file);
        if (arg.verbose) {
            cerr << "Docker option: in_file: " << in_file << endl;
            cerr << "Docker option: output_dir: " << output_dir << endl;
            cerr << "Docker option: output_file: " << output_file << endl;
        }
    }

    GeoFile *gf = nullptr;
    if (arg.data_type == MOD09) {
        gf = new Modis09L2GeoFile();
        if (((Modis09L2GeoFile *) gf)->readFile(in_file, arg.verbose, arg.build_level,
						arg.cover_level, arg.cover_gring, arg.stride)) {
            cerr << "Error reading MOD09 L2 file.\n";
            return 99;
        }
    }
    else if (arg.data_type == MOD09GA) {
        gf = new Modis09GAGeoFile();
        if (((Modis09GAGeoFile *) gf)->readFile(in_file, arg.verbose, arg.build_level)) {
            cerr << "Error reading MOD09GA file.\n";
            return 99;
        }
    }
    else {
        gf = new Modis05L2GeoFile();
        if (((Modis05L2GeoFile *) gf)->readFile(in_file, arg.verbose, arg.build_level,
                                                arg.cover_level, arg.cover_gring, arg.stride)) {
            cerr << "Error reading MOD05 file.\n";
            return 99;
        }
    }

    // Determine the output filename; output_file and output_dir are mutually exclusive. jhrg 8/20/21
    string file_out;
    if (!output_file.empty())
        file_out = output_file;
    else
        file_out = pickOutputName(gf->sidecar_filename(in_file), output_dir);

    // Create the sidecar file.
    SidecarFile sf;
    sf.createFile(file_out, arg.verbose, arg.institution);

    // Write the sidecar file.
    for (int i = 0; i < gf->d_num_index; i++)
    {
	double *lats = &gf->geo_lat[i][0];
	double *lons = &gf->geo_lon[i][0];
	unsigned long long int *geo_idx = &gf->geo_index[i][0];
	if (sf.writeSTAREIndex(arg.verbose, arg.build_level, gf->geo_num_i[i],
                               gf->geo_num_j[i], lats, lons, geo_idx,
                               gf->var_name[i], gf->d_stare_index_name[i])) {
            cerr << "Error writing STARE index.\n";
            return 99;
        }
    }

    std::cout << "writing covers" << std::endl;
    for (int i = 0; i < gf->num_cover; i++) {
        std::cout << "writing cover i = " << i << ", name = " <<
                  gf->stare_cover_name.at(i) << std::endl;
        if (sf.writeSTARECover(arg.verbose, gf->geo_num_cover_values[i], &gf->geo_cover[i][0],
                               gf->stare_cover_name.at(i))) {
            cerr << "Error writing STARE cover.\n";
            return 99;
        }
    }

    // Close the sidecar file.
    sf.close_file();

    delete gf;
    return 0;
};

