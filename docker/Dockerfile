###############################################################################################
# 
# Dockerfile for besdaemon image
#
#
# Some shell state reference:
# set -f # "set -o noglob"  Disable file name generation using metacharacters (globbing).
# set -v # "set -o verbose" Prints shell input lines as they are read.
# set -x # "set -o xtrace"  Print command traces before executing command.
# set -e #  Exit on error.
#
# In general use "set -e" when running commands that matter and don't use
# it for debugging stuff.
#

FROM centos:7

USER root

# Set one or more individual labels
LABEL vendor="OPeNDAP Incorporated"
LABEL org.opendap.stare-master.version=${STARE_MASTER_VERSION}
LABEL org.opendap.stare-master.release-date="2017-06-05"
LABEL org.opendap.stare-master.production="true"

MAINTAINER support@opendap.org

# VERSION INFO
ENV STARE_MASTER_VERSION=0.2.1

# Build env vars
ENV prefix="/home/build"
ENV MAKE="make"
ENV MAKEFLAGS="-j8"
ENV HYRAX_DEPENDENCIES="https://github.com/OPENDAP/hyrax-dependencies.git"

ENV STARE_MASTER="https://github.com/OPENDAP/STAREmaster"

# Update and install the needful.
RUN yum -y update

RUN yum -y install \
    gcc-c++ flex bison cmake autoconf automake libtool emacs openssl-devel libuuid-devel readline-devel zlib-devel \
    bzip2 bzip2-devel libjpeg-devel libxml2-devel curl-devel libicu-devel cppunit cppunit-devel vim bc make

RUN yum -y install epel-release

RUN yum -y install hdf-devel netcdf-devel

RUN yum clean all

# NB: See above env vars. jhrg 8/20/21

# Build and install HDFEOS2 and STARE using the hyrax deps. This will install
# these libs in $prefix/deps.
RUN git clone $HYRAX_DEPENDENCIES
RUN set -e && (cd hyrax-dependencies; $MAKE hdfeos stare $MAKEFLAGS LDFLAGS=/usr/lib64/hdf)

# Now build STAREmaster
RUN git clone $STARE_MASTER
RUN set -e && (cd STAREmaster; autoreconf --force --install --verbose; \
    ./configure --prefix=$prefix CPPFLAGS="-I$prefix/deps/include -I/usr/include/hdf" LDFLAGS="-L$prefix/deps/lib -L/usr/lib64/hdf"; \
    $MAKE $MAKEFLAGS)

ENV PATH="$prefix/bin:$prefix/deps/bin:$PATH"

COPY entrypoint.sh /entrypoint.sh
RUN  chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
