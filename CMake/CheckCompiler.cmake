
##=============================================================================##
#
# 32 Or 64 Bit Compiler ?
IF( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
        SET( HAVE_64_BIT 1 ) 
	ADD_DEFINITIONS(-DHAVE_64_BIT)
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
        SET( HAVE_64_BIT 0 ) 
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 

# Sets some variables depending on which compiler you are using
#
# USING_GCC  : gcc is being used for C compiler
# USING_GPP  : g++ is being used for C++ compiler
# USING_ICC  : icc is being used for C compiler
# USING_ICPC : icpc is being used for C++ compiler
# USING_WINDOWS_MSVC : MSVC Visual Studio's compiler
# USING_WINDOWS_ICL : Intel's Windows compiler
# USING_LLVM_CLANG : Clang = LLVM Clang (clang.llvm.org)
#  Absoft = Absoft Fortran (absoft.com)
#  ADSP = Analog VisualDSP++ (analog.com)
#  Cray = Cray Compiler (cray.com)
#  Embarcadero, Borland = Embarcadero (embarcadero.com)
#  G95 = G95 Fortran (g95.org)
# GNU = GNU Compiler Collection (gcc.gnu.org)
# HP = Hewlett-Packard Compiler (hp.com)
#  Intel = Intel Compiler (intel.com)
#  MIPSpro = SGI MIPSpro (sgi.com)
#  PGI = The Portland Group (pgroup.com)
#  PathScale = PathScale (pathscale.com)
#  SDCC = Small Device C Compiler (sdcc.sourceforge.net)
# SunPro = Oracle Solaris Studio (oracle.com)
#  TI_DSP = Texas Instruments (ti.com)
#  TinyCC = Tiny C Compiler (tinycc.org)
# Watcom = Open Watcom (openwatcom.org)
# XL, VisualAge, zOS = IBM XL (ibm.com)


# Have to set this variable outside of the top level IF statement,
# since CMake will break if you use it in an IF statement.

SET(NATIVE_COMPILER_NAME_REGEXPR "icc.*$")

#GCC/ICC
IF(NOT CMAKE_COMPILER_IS_GNUCC)
  # This regular expression also matches things like icc-9.1
  IF(CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
    SET(USING_ICC TRUE)
    SET(USING_KNOWN_C_COMPILER TRUE)
	
	MESSAGE("Using USING_ICC  : icc is being used for C compiler")
	
  ENDIF(CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
ELSE(NOT CMAKE_COMPILER_IS_GNUCC)
  SET(USING_GCC TRUE)
  SET(USING_KNOWN_C_COMPILER TRUE)
  
  MESSAGE("Using USING_GCC  : gcc is being used for C compiler")
  
ENDIF(NOT CMAKE_COMPILER_IS_GNUCC)

SET(NATIVE_COMPILER_NAME_REGEXPR "icpc.*$")
IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  IF   (CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
    SET(USING_ICPC TRUE)
    SET(USING_KNOWN_CXX_COMPILER TRUE)

    EXEC_PROGRAM(${CMAKE_CXX_COMPILER} 
      ARGS --version 
      OUTPUT_VARIABLE TEMP)

    STRING(REGEX MATCH "([0-9\\.]+)"
      INTEL_COMPILER_VERSION
      ${TEMP})
	  
	MESSAGE("Using USING_ICPC : icpc is being used for C++ compiler, version ${INTEL_COMPILER_VERSION}")
	
    MARK_AS_ADVANCED(INTEL_COMPILER_VERSION)
  ENDIF(CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
ELSE(NOT CMAKE_COMPILER_IS_GNUCXX)
  SET(USING_GPP TRUE)
  SET(USING_KNOWN_C_COMPILER TRUE)
  SET(USING_KNOWN_CXX_COMPILER TRUE)
  MESSAGE("Using USING_GPP  : g++ is being used for C++ compiler")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)

# The idea is to match a string that ends with cl but doesn't have icl in it.
SET(NATIVE_COMPILER_NAME_REGEXPR "([^i]|^)cl.*$")
IF( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
  SET(USING_WINDOWS_MSVC TRUE)
  SET(USING_KNOWN_C_COMPILER TRUE)
  SET(USING_KNOWN_CXX_COMPILER TRUE)
  # We should set this macro as well to get our nice trig functions
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  # Microsoft does some stupid things like #define min and max.
  ADD_DEFINITIONS(-DNOMINMAX)
  
  MESSAGE("Using USING_WINDOWS_MSVC : Visual Studio's compiler")
  
ENDIF( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
	
IF( ${CMAKE_C_COMPILER_ID} STREQUAL Clang  AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang  AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
  SET(USING_LLVM_CLANG TRUE)
  SET(USING_KNOWN_C_COMPILER TRUE)
  SET(USING_KNOWN_CXX_COMPILER TRUE)
  
  MESSAGE("Using USING_LLVM_CLANG : LLVM Clang compiler")
  
ENDIF( ${CMAKE_C_COMPILER_ID} STREQUAL Clang AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})

# Intel compiler on windows.  Make sure this goes after the cl one.
SET(NATIVE_COMPILER_NAME_REGEXPR "icl.exe$")
IF( ${CMAKE_C_COMPILER_ID} STREQUAL Intel AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL Intel  AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
  SET(USING_WINDOWS_ICL TRUE)
  SET(USING_WINDOWS_MSVC FALSE) # Turn off the other compiler just in case
  SET(USING_KNOWN_C_COMPILER TRUE)
  SET(USING_KNOWN_CXX_COMPILER TRUE)
  # We should set this macro as well to get our nice trig functions
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  # Microsoft does some stupid things like #define min and max.
  ADD_DEFINITIONS(-DNOMINMAX)
  
  MESSAGE("Using USING_WINDOWS_ICL : Intel's Windows compiler")
  
ENDIF( ${CMAKE_C_COMPILER_ID} STREQUAL Intel AND CMAKE_C_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR} AND
    ${CMAKE_CXX_COMPILER_ID} STREQUAL Intel  AND CMAKE_CXX_COMPILER MATCHES ${NATIVE_COMPILER_NAME_REGEXPR})
# Do some error checking

# Mixing compilers
IF   (USING_ICC AND USING_GPP)
  FIRST_TIME_MESSAGE("Using icc combined with g++.  Good luck with that.")
ENDIF(USING_ICC AND USING_GPP)

IF   (USING_GCC AND USING_ICPC)
  FIRST_TIME_MESSAGE("Using gcc combined with icpc.  Good luck with that.")
ENDIF(USING_GCC AND USING_ICPC)

IF   (USING_ICC AND USING_GPP)
  FIRST_TIME_MESSAGE("Using icc combined with g++.  Good luck with that")
ENDIF(USING_ICC AND USING_GPP)

# Using unknown compilers
IF   (NOT USING_KNOWN_C_COMPILER)
  FIRST_TIME_MESSAGE("Specified C compiler ${CMAKE_C_COMPILER} is not recognized (gcc, icc).  Using CMake defaults.")
ENDIF(NOT USING_KNOWN_C_COMPILER)

IF   (NOT USING_KNOWN_CXX_COMPILER)
  FIRST_TIME_MESSAGE("Specified CXX compiler ${CMAKE_CXX_COMPILER} is not recognized (g++, icpc).  Using CMake defaults.")
ENDIF(NOT USING_KNOWN_CXX_COMPILER)

# Warn if the compiler is not icc on SGI_LINUX systems
IF   (CMAKE_SYSTEM_PROCESSOR MATCHES "ia64")
  IF(NOT USING_ICC)
	  FIRST_TIME_MESSAGE("Intel Compilers recommended on ia64. setenv CC icc before running cmake.")
  ENDIF(NOT USING_ICC)

  IF(NOT USING_ICPC)
	  FIRST_TIME_MESSAGE("Intel Compilers recommended on ia64. setenv CXX icpc before running cmake.")
  ENDIF(NOT USING_ICPC)
ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "ia64")