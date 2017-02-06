
cmake_minimum_required(VERSION 3.5)

project( sync_ptr )

file(GLOB C_FILES *.cpp )
file(GLOB H_FILES *.h )

add_executable( sync_ptr WIN32 ${C_FILES} ${H_FILES} )
