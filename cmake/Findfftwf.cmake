# FFTWF_INCLUDE_DIR = fftw3.h
# FFTWF_LIBRARIES = libfftw3.a
# FFTWF_FOUND = true if FFTW3 is found

IF(FFTWF_INCLUDE_DIRS)
  FIND_PATH(FFTWF_INCLUDE_DIR fftw3.h  ${FFTWF_INCLUDE_DIRS})
  FIND_LIBRARY(FFTWF_LIBRARY fftw3f ${FFTWF_LIBRARY_DIRS})
ELSE(FFTWF_INCLUDE_DIRS)
  #  SET(TRIAL_PATHS
  #    $ENV{FFTWF_HOME}/include
  #    /usr/include
  #    /usr/local/include
  #    /opt/include
  #    /usr/apps/include
  #  )
  #
  #  SET(TRIAL_LIBRARY_PATHS
  #    $ENV{FFTWF_HOME}/lib
  #    /usr/lib 
  #    /usr/local/lib
  #    /opt/lib
  #    /sw/lib
  #    )
  #
  #  FIND_PATH(FFTWF_INCLUDE_DIR fftw3.h ${TRIAL_PATHS})
  #  FIND_LIBRARY(FFTWF_LIBRARY fftw3 ${TRIAL_LIBRARY_PATHS})
  FIND_PATH(FFTWF_INCLUDE_DIR fftw3.h ${QMC_INCLUDE_PATHS})
  FIND_LIBRARY(FFTWF_LIBRARIES fftw3f ${QMC_LIBRARY_PATHS}) 

ENDIF(FFTWF_INCLUDE_DIRS)


IF(FFTWF_INCLUDE_DIR AND FFTWF_LIBRARIES)
  SET(FFTWF_FOUND TRUE)
ELSE()
  SET(FFTWF_FOUND FALSE)
ENDIF()

IF(FFTWF_FOUND)
  MESSAGE(STATUS "Found FFTW3F (FFTWF_INCLUDE_DIR = ${FFTWF_INCLUDE_DIR})")
  MESSAGE(STATUS "Found FFTW3F (FFTWF_LIBRARIES = ${FFTWF_LIBRARIES})")
ELSE()
  MESSAGE(FATAL_ERROR "Could not find FFTW3F")
ENDIF()

MARK_AS_ADVANCED(
   FFTWF_INCLUDE_DIR
   FFTWF_LIBRARIES
   FFTWF_FOUND
)
