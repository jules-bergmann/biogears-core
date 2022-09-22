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
# cores -- number of jobs/cores to run in parallel
#  - default to 4 to be nice to crunchr
#
if [[ $(hostname) -eq 'mrphpch1' ]]; then
  aamv=/home/jbergma8/dev/projects/biogears-deps
  aamv_prefix=$aamv
  cores=16
else
  aamv=/home/idies/workspace/Storage/jbergma8/aamv
  aamv_prefix=$aamv/ref/external
  cores=4
fi

#
# build-type
#  - Release
#  - Debug
#
build_type=Release

#



do_make=0
do_config=0
dry=''
cmake=cmake

OPTS=""
LONGOPTS=build-type:,cores:,config,make,dry
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
    --cores)
      cores="$2"
      shift 2
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "Unexpected arg $1"
      exit
      ;;
  esac
done

echo "remaining: $@"
echo "count: $#"


if [[ $do_config -eq 1 ]]; then
  $dry cmake -G "Ninja"					\
	-DCMAKE_INSTALL_PREFIX=$aamv/ref/install	\
	-DCMAKE_PREFIX_PATH=$aamv_prefix		\
	-DCMAKE_BUILD_TYPE=$build_type			\
	-DBiogears_BUILD_JAVATOOLS=OFF			\
	-DBiogears_BUILD_HOWTOS=ON			\
	..
fi

if [[ $do_make -eq 1 ]]; then
  if [[ $# -eq 0 ]]; then
    target="bg-cli bg-scenario"
  else
    target=$@
  fi
  echo "make target: $target"
  $dry cmake --build . --config $build_type --parallel $cores --target $target 
fi
