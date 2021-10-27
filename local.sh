#! /bin/sh
# --------------------------------------------------------------------- #
# local.sh -- local config/build for AAMV biogears
#
# History:
#  - 10/22/2021 created, jpb
# --------------------------------------------------------------------- #

#
# aamv -- path to shared dir onc CRUNCHR, includes
#  - local install of cmake, ninja
#  - external dependencies.  E.g eigen, etc
#
aamv=/home/idies/workspace/Storage/jbergma8/aamv

#
# build-type
#  - Release
#  - Debug
#
build_type=Release

do_make=0
do_config=0
dry=''
cmake=cmake

OPTS=""
LONGOPTS=built-type:,config,make,dry
! PARSED=$(getopt --options=$OPTS --longoptions=$LONGOPTS --name "$0" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
  echo 'getopt error'
  exit -1
fi
eval set -- "$PARSED"

while true; do
  case "$1" in
    --build-type)
      build_type="$2"
      shift 2
      ;;
    --config)
      do_config=1
      shift
      ;;
    --make)
      do_make=1
      shift
      ;;
    --dry)
      dry=echo
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "arg processing error"
      exit
      ;;
  esac
done

if [[ $do_config -eq 1 ]]; then
  $dry cmake -G "Ninja"					\
	-DCMAKE_INSTALL_PREFIX=$aamv/ref/install	\
	-DCMAKE_PREFIX_PATH=$aamv/ref/external		\
	-DCMAKE_BUILD_TYPE=$build_type			\
	-DBiogears_BUILD_JAVATOOLS=OFF			\
	-DBiogears_BUILD_HOWTOS=OFF			\
	..
fi

if [[ $do_make -eq 1 ]]; then
  $dry cmake --build . --config Release --target bg-cli bg-scenario
fi
