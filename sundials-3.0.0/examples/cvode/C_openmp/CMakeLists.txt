# ---------------------------------------------------------------
# Programmer: Ting Yan @ SMU
# ---------------------------------------------------------------
# LLNS/SMU Copyright Start
# Copyright (c) 2015, Southern Methodist University and 
# Lawrence Livermore National Security
#
# This work was performed under the auspices of the U.S. Department 
# of Energy by Southern Methodist University and Lawrence Livermore 
# National Laboratory under Contract DE-AC52-07NA27344.
# Produced at Southern Methodist University and the Lawrence 
# Livermore National Laboratory.
#
# All rights reserved.
# For details, see the LICENSE file.
# LLNS/SMU Copyright End
# Copyright (c) 2013, Southern Methodist University.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# CMakeLists.txt file for CVODE OpenMP examples
# -----------------------------------------------------------------

# Only include tests if OpenMP is enabled
IF(OPENMP_FOUND)

  # Examples using SUNDIALS linear solvers
  SET(CVODE_examples
    cvAdvDiff_bnd_omp
    )

ENDIF(OPENMP_FOUND)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE
IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(CVODE_LIB sundials_cvode_static)
  SET(NVECOMP_LIB sundials_nvecopenmp_static)
ELSE()
  SET(CVODE_LIB sundials_cvode_shared)
  SET(NVECOMP_LIB sundials_nvecopenmp_shared)
ENDIF()

# Set-up linker flags and link libraries
SET(SUNDIALS_LIBS ${CVODE_LIB} ${NVECOMP_LIB} ${EXTRA_LINK_LIBS})

# update the compilation flags to include OpenMP support
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_FLAGS}")

# Add the build and install targets for each example
FOREACH(example ${CVODE_examples})

  # example source files
  ADD_EXECUTABLE(${example} ${example}.c)

  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")

  # add to regression tests
  SUNDIALS_ADD_TEST(${example} ${example} TEST_ARGS 4)

  # libraries to link against
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})

  # install example source and out files
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.c ${example}.out
      DESTINATION ${EXAMPLES_INSTALL_PATH}/cvode/C_openmp)
  ENDIF()

ENDFOREACH(example ${CVODE_examples})


# create Makfile and CMakeLists.txt for examples
IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/cvode/C_openmp)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "CVODE")
  SET(SOLVER_LIB "sundials_cvode")

  LIST2STRING(CVODE_examples EXAMPLES)

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_openmp_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/cvode/C_openmp/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/cvode/C_openmp/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/cvode/C_openmp 
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_openmp_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/cvode/C_openmp/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/cvode/C_openmp/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/cvode/C_openmp
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
