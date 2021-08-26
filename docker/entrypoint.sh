#!/bin/bash
# This is the entrypoint.sh file for the mk_stare container.

echo "df -h"
df -h

echo "ls -l /"
ls -l /

echo "ls -l /project"
ls -l /project

/home/build/bin/mk_stare "$@"

# while true; do sleep 60; done
