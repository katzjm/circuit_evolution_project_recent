# ---------------------------------------------------------------
# Programmer:  Slaven Peles @ LLNL
# ---------------------------------------------------------------
# LLNS Copyright Start
# Copyright (c) 2014, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End
# ---------------------------------------------------------------
# CMakeLists.txt file for nvector examples

# Add variable nvector_cuda_examples with the names of the nvector examples

SET(nvector_cuda_examples
  test_nvector_cuda
)

SET(nvector_examples_dependencies
  test_nvector
  sundials_nvector
  sundials_math
)

# Add source directory to include directories
INCLUDE_DIRECTORIES(. .. ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(NVECS_LIB sundials_nveccuda_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(NVECS_LIB sundials_nveccuda_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Set-up linker flags and link libraries
SET(SUNDIALS_LIBS ${NVECS_LIB} ${EXTRA_LINK_LIBS})

# Add the build and install targets for each nvector example
FOREACH(example ${nvector_cuda_examples})

  IF(WIN32)
    # This is used to get around DLL linkage issue since we are
    # manually including sundials_nvector.c here, which is normally in
    # a library that is included.  If this is not set build system
    # thinks nvector is externally linked.
    ADD_DEFINITIONS(-DBUILD_SUNDIALS_LIBRARY)
  ENDIF(WIN32)

  CUDA_ADD_EXECUTABLE(${example} ${example}.cu ../test_nvector.c ../../../src/sundials/sundials_nvector.c ../../../src/sundials/sundials_math.c)
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")

  SUNDIALS_ADD_TEST( ${example}_10000_0 ${example} TEST_ARGS 10000 0)

  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS} ${CUDA_LIBRARIES})

  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.cu ../test_nvector.c ../test_nvector.h ../../../src/sundials/sundials_nvector.c  ../../../src/sundials/sundials_math.c DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/cuda)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example ${nvector_cuda_examples})

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/cuda)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER_LIB "sundials_nveccuda")
  LIST2STRING(nvector_cuda_examples EXAMPLES)
  LIST2STRING(nvector_examples_dependencies EXAMPLES_DEPENDENCIES)

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_cuda_CUDA_ex.in
      ${PROJECT_BINARY_DIR}/examples/nvector/cuda/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/nvector/cuda/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/cuda
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_cuda_CUDA_ex.in
      ${PROJECT_BINARY_DIR}/examples/nvector/cuda/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/nvector/cuda/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/cuda
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
