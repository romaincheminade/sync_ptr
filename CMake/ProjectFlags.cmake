
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( CMAKE_INCLUDE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/include )

# Grab target compiler and set corresponding variables.
###################################################################################################
# USING_GCC  			: gcc is being used for C compiler
# USING_GPP  			: g++ is being used for C++ compiler
# USING_ICC  			: icc is being used for C compiler
# USING_ICPC 			: icpc is being used for C++ compiler
# USING_WINDOWS_MSVC 		: Visual Studio's compiler
# USING_WINDOWS_ICL 		: Intel's Windows compiler 
# USING_LLVM_CLANG 		: Mac OSX 10.8 and hieveer (XCode) compiler
include( ${CMAKE_CURRENT_SOURCE_DIR}/CMake/CheckCompiler.cmake )


if( USING_WINDOWS_MSVC )
	set(CMAKE_C_FLAGS   	"${CMAKE_C_FLAGS}   	/W4 /wd4251")
	set(CMAKE_CXX_FLAGS 	"${CMAKE_CXX_FLAGS} 	/W4 /wd4251")
	
	set (CMAKE_C_FLAGS 	"${CMAKE_C_FLAGS} 	/MP /bigobj /Gy /GT ")
	set (CMAKE_CXX_FLAGS 	"${CMAKE_CXX_FLAGS} 	/MP /bigobj /Gy /GT ")
	
	# # Edit and continue.
	# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	
	# Linker flags
	set( CMAKE_EXE_LINKER_FLAGS 	"${CMAKE_EXE_LINKER_FLAGS} 	/INCREMENTAL:NO") 
	set( CMAKE_SHARED_LINKER_FLAGS 	"${CMAKE_SHARED_LINKER_FLAGS} 	/INCREMENTAL:NO") 
	set( CMAKE_MODULE_LINKER_FLAGS 	"${CMAKE_MODULE_LINKER_FLAGS} 	/INCREMENTAL:NO") 

	# With Visual Studio 2005, Microsoft deprecates the standard C library, for
	# example fopen() and sprintf(), to non-portable functions fopen_s() and
	# sprintf_s(). These functions are considered by Microsoft more secure. This is
	# a worthwhile exercise ! The use of these deprecated functions causes a lot of
	# warnings. To suppress it, we add the _CRT_SECURE_NO_DEPRECATE and _CRT_NONSTDC_NO_DEPRECATE preprocessor
	# definition -fprofile-arcs -ftest-coverage
	add_definitions( -D_CRT_SECURE_NO_DEPRECATE  -D_CRT_NONSTDC_NO_DEPRECATE )	
	# /D_CRT_SECURE_NO_WARNINGS : This switch disables the warnings that Visual Studio spits out if you use any of the C RunTime functions that Microsoft has provided secure replacements for (such as strncpy). 
	# This isn't necessarily a bad idea, but their safe functions are not part of the standard and are thus non-portable:
	# reference : http://msdn.microsoft.com/en-us/library/8ef0s5kh.aspx
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )	
	# /D_SCL_SECURE_NO_WARNINGS : Just like the switch above, only for the Standard C++ Library (anything in the std namespace):
	# reference : http://msdn.microsoft.com/en-us/library/aa985974.aspx	
	add_definitions( -D_SCL_SECURE_NO_WARNINGS )
	# __STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS are a workaround to allow C++ programs to use macros specified in the C99 standard that aren't in the C++ standard. 
	# The macros, such as UINT8_MAX, INT64_MIN, and INT32_C() may be defined already in C++ applications in other ways. 
	# To allow the user to decide if they want the macros defined as C99 does, many implementations require that __STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS be defined before stdint.h is included
	add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )		
	# Unicode
	# add_definitions( -D_UNICODE -DUNICODE )	
	# Remove Windows "min" and "max" defines conflicting with "std::min" and "std::max"
	add_definitions( -DNOMINMAX )
	# Variadic template max number of elements.
	add_definitions( -D_VARIADIC_MAX=10 )

elseif( USING_GCC OR USING_LLVM_CLANG )

	message( "\nEclipse Debug: Please set CMAKE_BUILD_TYPE to Debug manually.\n" )

	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z" )

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND USING_LLVM_CLANG)	

		# Address Sanitizer		
		option( BUILD_ENABLE_ASAN "Enable Address Sanitizer" ON)
		if( BUILD_ENABLE_ASAN )
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" ) 
		endif()
	
		# Memory Sanitizer.
		option( BUILD_ENABLE_MSAN "Enable Memory Sanitizer" OFF)
		if( BUILD_ENABLE_MSAN )
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=memory" )
		endif()
	
		# Thread Sanitizer.
		option( BUILD_ENABLE_TSAN "Enable Thread Sanitizer" OFF)
		if( BUILD_ENABLE_TSAN )
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread" )
		endif()
	
		# Undefined Behavior Sanitizer.
		option( BUILD_ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" ON)
		if( BUILD_ENABLE_UBSAN )
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined" )
		endif()

	endif()

endif()

# Postfix
###################################################################################################
set(CMAKE_DEBUG_POSTFIX			"d")
set(CMAKE_RELEASE_POSTFIX		"")
set(CMAKE_RELWITHDEBINFO_POSTFIX 	"rd")
set(CMAKE_MINSIZEREL_POSTFIX 		"s")

if( CMAKE_BUILD_TYPE MATCHES "Release" )
    set( CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}" )

elseif( CMAKE_BUILD_TYPE MATCHES "MinSizeRel" )
    set( CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}" )

elseif( CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" )
    set( CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}" )

elseif( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set( CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}" )

else()
    set( CMAKE_BUILD_POSTFIX "" )

endif()

