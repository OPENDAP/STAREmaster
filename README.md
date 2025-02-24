# STAREmaster

This is a utility to use the STARE library on netCDF and HDF5
files. For full documentation, see
https://spatiotemporal.github.io/STAREmaster/

For more info about STARE, see [the STARE GitHub
site](https://github.com/SpatioTemporal/STARE) and [STARE - TOWARD
UNPRECEDENTED GEO-DATA
INTEROPERABILITY](https://www.researchgate.net/publication/320908197_STARE_-_TOWARD_UNPRECEDENTED_GEO-DATA_INTEROPERABILITY).

For more information about STARE, see the [STARE demonstration
presentation](https://drive.google.com/file/d/16z9pPxAnWKl2b1yKkhHyKM9pAQuGP0Z7).

## Supported Datasets

Dataset        | STAREmaster ID | Full Name | Notes
-------        | -------------- | --------- | -----
MODIS MYD05    | MOD05          | Total Precipitable Water Vapor 5-Min L2 Swath 1km and 5km | https://ladsweb.modaps.eosdis.nasa.gov/missions-and-measurements/products/MYD05_L2
MODIS MYD09    | MOD09          | Atmospherically Corrected Surface Reflectance 5-Min L2 Swath 250m, 500m, 1km | LAADS DAAC https://ladsweb.modaps.eosdis.nasa.gov/missions-and-measurements/products/MOD09

## Building STAREmaster

STAREmaster has both autotools and CMake build systems. The
STAREmaster program requires several libraries.

Library | Required Version      | Source
------- | ----------------      | ------
STARE   | 0.16.3-beta           | https://github.com/SpatioTemporal/STARE/releases/tag/0.16.3-beta
HDF4    | 4.2.13                | https://support.hdfgroup.org/ftp/HDF/HDF_Current/src/hdf-4.2.13.tar.gz
HDFEOS2 | 1.00                  | https://observer.gsfc.nasa.gov/ftp/edhs/hdfeos/latest_release/HDF-EOS2.20v1.00.tar.Z
HDF5    | 1.8.x, 1.10.x, 1.12.x | https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/hdf5-1.10.6.tar.gz
netcdf-c| 4.x                   | https://github.com/Unidata/netcdf-c/releases

### Build Notes

*Note*: To build with Hyrax, see below

When building HDF4 and HDFEOS2, there are some build issues. As
demonstrated in the GitHub workflow
builds(ex. https://github.com/SpatioTemporal/STAREmaster/blob/master/.github/workflows/autotools.yml),
set the CFLAGS to -fPIC. For the most recent versions of gcc,
-Wno-implicit-function-declaration is required.

When building HDF4, the --disable-netcdf option must be used. The
--disable-fortran option may be used, as STAREmaster does not use the
Fortran API for HDF4.

Building HDF4 can be accomplished with these commands:

<pre>
export CFLAGS="-fPIC -Wno-implicit-function-declaration"
./configure --prefix=/usr/local/hdf-4.2.15_fPIC --disable-netcdf --disable-fortran
make all
make check
sudo make install
</pre>

The hdfeos documentation can be found here:
https://edhs1.gsfc.nasa.gov/waisdata/eeb/pdf/170eeb001.pdf

When building hdfeos, the --enable-install-include option must be used
with configure. Building hdfeos can be accomplished with these commands:

<pre>
export CPPFLAGS=-I/usr/local/hdf-4.2.15_fPIC/include LDFLAGS=-L/usr/local/hdf-4.2.15_fPIC/lib
export CFLAGS=-fPIC
./configure --prefix=/usr/local/hdfeos_fPIC --enable-install-include
make all
make check
sudo make install
</pre>

#### Different HDFOES2 Versions

There are several versions of HDFEOS2, and the two of interest to STAREmaster code:

Version | Link
--------|-----
2.19    | https://git.earthdata.nasa.gov/rest/git-lfs/storage/DAS/hdfeos/ce2beacbb1ab8471a9a207def005d559f0ab725b9a4f1b1525cbee3d20aab5b0?response-content-disposition=attachment%3B%20filename%3D%22hdfeos2_19.zip%22%3B%20filename*%3Dutf-8%27%27hdfeos2_19.zip
2.20    | https://observer.gsfc.nasa.gov/ftp/edhs/hdfeos/latest_release/HDF-EOS2.20v1.00.tar.Z

When building 2.19 from this link, it was necessary to cd into a subdirectory, and change the permissions of the configure file:

<pre>
wget -O hdfeos2_19.zip https://git.earthdata.nasa.gov/rest/git-lfs/storage/DAS/hdfeos/ce2beacbb1ab8471a9a207def005d559f0ab725b9a4f1b1525cbee3d20aab5b0?response-content-disposition=attachment%3B%20filename%3D%22hdfeos2_19.zip%22%3B%20filename*%3Dutf-8%27%27hdfeos2_19.zip &> /dev/null
unzip hdfeos2_19.zip
cd hdfeos2_19/hdfeos2.19/hdfeos
chmod ug+x configure
</pre>

### Building with Hyrax
The hyrax-dependencies repo contains all the libraries STAREmaster requires. Assuming you
are working from within the 'hyrax' meta project/repo, build using autotools with the following environment
variables (but see the notes that follow for more information):

<pre>
CPPFLAGS=-I$prefix/deps/include
CXXFLAGS="--std=c++11 -g3 -O0"
LDFLAGS=-L$prefix/deps/lib
</pre>

*Note*:
1. This assumes you've correctly configured the 'hyrax' repo for a build. 
   You should 'source spath.h' in it's top-level directory to get $prefix defined correctly
2. CXXFLAGS needs --std=c++11 to find a 'usable' STARE.h header.
3. Adding "-g3 -O0" enables debugging in CLion
   
Once the environment variables are set, run autoreconf, configure and make
as per the usual, with the following exception:

<pre>
./configure --prefix=$prefix/deps
</pre>

This will install the library and mk_stare command line tool in $prefix/deps/{lib,bin}

### Building with Autotools

When building with autotools locations of all required header files
and libraries must be provided via the CPPFLAGS and LDFLAGS
environmental variable. For example:

<pre>
export CPPFLAGS="-I/usr/local/hdfeos_fPIC/include -I/usr/local/hdf-4.2.15_fPIC/include -I/usr/local/STARE-0.16.3/include -I/usr/local/netcdf-c-4.7.4-development_hdf5-1.10.6/include"
export LDFLAGS="-L/usr/local/hdfeos_fPIC/lib -L/usr/local/hdf-4.2.15_fPIC/lib -L/usr/local/STARE-0.16.3/lib -L/usr/local/netcdf-c-4.7.4-development_hdf5-1.10.6/lib"
</pre>

Configure and make in the standard way:

<pre>
autoreconf -i
./configure
make check
make install
make clean
</pre>

If OpenMP is present on the system, it will be automatically enabled.

### Building with CMake

CMake finds the necessary library using CMake variables:

<pre>
mkdir build
cd build
cmake  -DTEST_LARGE=/home/ed -DENABLE_LARGE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug --trace-source=test/CMakeLists.txt -DNETCDF_INCLUDES=/usr/local/netcdf-c-4.7.4_hdf5-1.10.6/include -DNETCDF_LIBRARIES=/usr/local/netcdf-c-4.7.4_hdf5-1.10.6/lib -DSTARE_INCLUDE_DIR=/usr/local/STARE-0.15.6/include -DSTARE_LIBRARY=/usr/local/STARE-0.15.6/lib -DCMAKE_PREFIX_PATH="/usr/local/hdf-4.2.15;/usr/local/hdfeos" .. 
make VERBOSE=1
make VERBOSE=1 test
</pre>

Sometimes the FindNetCDF.cmake module has difficulty finding all the netCDF libraries needed for linking. This has been identified as an issue, but the fix is non-trivial (see https://github.com/SpatioTemporal/STAREmaster/issues/11). A workaround is to set the CMAKE_CXX_STANDARD_LIBRARIES variable when invoking cmake. For example:

<pre>
cmake -DTEST_LARGE=/home/ed -DENABLE_LARGE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug --trace-source=CMakeLists.txt -DNETCDF_INCLUDES=/usr/local/netcdf-c-4.7.4_hdf5-1.10.6/include -DNETCDF_LIBRARIES=/usr/local/netcdf-c-4.7.4_hdf5-1.10.6/lib -DSTARE_INCLUDE_DIR=/usr/local/STARE-0.16.3/include -DSTARE_LIBRARY=/usr/local/STARE-0.16.3/lib -DCMAKE_PREFIX_PATH="/usr/local/hdf-4.2.15;/usr/local/hdfeos" -DCMAKE_CXX_STANDARD_LIBRARIES="-lcurl" ..
</pre>

To make use of OpenMP multi-threading you may need to add -DOpenMP_EXE_LINKER_FLAGS=...
to the cmake command line.

## Using STAREmaster

The STAREmaster package installs a binary mk_stare, which may
be used to create a sidecar file for any of the supported datasets. To
use:

<pre>
mk_stare -d MOD09 data/MYD09.A2020058.1515.006.2020060020205.hdf
</pre>

To see a number usage options execute _mk_stare_ without any
command line arguments.

To enable OpenMP multithreading, you may need to set the environment
variable OMP_NUM_THREADS to the number of threads you wish used.

## Adding New Data Types

Each input data set requires custom programming in order to generate the sidecar files.

To handle a new MODIS data type, take a subclass of ModisGeoFile.

To handle a new non-MODIS data type, take a subclass of GeoFile.h.

The GeoFile class has a function that must be implemented for every
derived class: the readFile() function.

In the readFile() function, open a data file and use the STARE library
to create the sidecar file. To do this:

* Determine what data variables exist in the file, and their grid
  information. Store the variables in the variables vector of
  GeoFile.h.

* One set of STARE indexes are needed for each gridding system in use
  in the file. For example, if the file contains data at two different
  resolutions, two sets of STARE indexes will be required, one for
  each resolution.

* Use the STARE library to construct the STARE indexes, and store them
  in the geo_index1 variable of GeoFile.h.

* Use the STARE library to calculate a "cover" for the data
  file. Store in the cover variable (see GeoFile.h).

Once the data file has been read, and the STARE indexes constructed,
an instance of the SidecarFile class can be instantiated. To construct
a sidecar file:

* Call the createFile() method to start the creation of a sidecar file.

* Call the writeSTARECover() function to write the cover information
  to the file.

* Call the writeSTAREIndex() function once for each STARE index in the
  file.

* Call the closeFile() function to close the netCDF file and release
  all resources.

## References

Kuo, K-S and ML Rilee, “STARE – Toward unprecedented geo-data
interoperability,” 2017 Conference on Big Data from Space. Toulouse,
France. 28-30 November 2017, retrieved on Sep 19, 2020 from
https://www.researchgate.net/publication/320908197_STARE_-_TOWARD_UNPRECEDENTED_GEO-DATA_INTEROPERABILITY.

Kuo, K-S, H. YuYu, and ML Rilee, "Leveraging STARE for Co-aligned Data
Locality with netCDF and Python MPI", 2019 IEEE International
Geoscience and Remote Sensing Symposium, June 2019, retrieved on Sep
19, 2020 from
https://www.researchgate.net/publication/337504144_Leveraging_STARE_for_Co-aligned_Data_Locality_with_netCDF_and_Python_MPI.

HDF-EOS Library User's Guide Volume 1: Overview and Examples,
retrieved Sep 7, 2020 from
http://newsroom.gsfc.nasa.gov/sdptoolkit/docs/2.20/HDF-EOS_UG.pdf

HDF-EOS Library User's Guide Volume 2: Function Reference Guide,
retrieved Sep 7, 2020 from
http://newsroom.gsfc.nasa.gov/sdptoolkit/docs/2.20/HDF-EOS_REF.pdf

HDF-EOS Interface Based on HDF5, Volume 1: Overview and Examples,
retrieved Sep 7, 2020 from
http://newsroom.gsfc.nasa.gov/sdptoolkit/docs/2.20/HDF-EOS5_UG.pdf

HDF-EOS Interface Based on HDF5, Volume 2: Function Reference Guide,
retrieved Sep 7, 2020 from
http://newsroom.gsfc.nasa.gov/sdptoolkit/docs/2.20/HDF-EOS5_REF.pdf
