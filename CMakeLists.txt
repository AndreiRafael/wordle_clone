cmake_minimum_required(VERSION 3.1)

set(GAME_TITLE wordle_clone)

project(${GAME_TITLE} C)

#debug/errorlog on/off
add_definitions(-DHFE_DEBUG)

#output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
	add_executable(${GAME_TITLE} WIN32 ./src/main.c)
else()
	add_executable(${GAME_TITLE} ./src/main.c)
endif()

# Make compiler scream out every possible warning
if(MSVC)
	target_compile_options(${GAME_TITLE} PRIVATE /W4 /WX)
else()
	target_compile_options(${GAME_TITLE} PRIVATE -Wconversion -Wall -Wextra -Wpedantic -Werror)
endif()

# Add sources from Sourcefile (snip from Sonic Robo Blast 2)
function(target_sourcefile file prjct)
	file(STRINGS ${file} list)
	list(TRANSFORM list PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
	target_sources(${prjct} PRIVATE ${list})
endfunction()

#includes
include_directories(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/include)

#libs
#sdl
if(WIN32)
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/lib/sdl/SDL2main.lib)
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/lib/sdl/SDL2.lib)

	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_DL_LIBS} m pthread)
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/lib/sdl/libSDL2maind.a)
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/lib/sdl/libSDL2d.a)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/sgb-words.txt DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/letters.bmp DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)