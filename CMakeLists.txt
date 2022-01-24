cmake_minimum_required(VERSION 3.1)

set(GAME_TITLE wordle_clone)

project(${GAME_TITLE} C)

#debug/errorlog on/off
add_definitions(-DHFE_DEBUG)

#output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(${GAME_TITLE} ./src/main.c)
else()
	if(WIN32)
		add_executable(${GAME_TITLE} WIN32 ./src/main.c)
	else()
		add_executable(${GAME_TITLE} ./src/main.c)
	endif()
endif()

#emscripten output
if(EMSCRIPTEN)
    message("Using emscripten!")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(${GAME_TITLE} PROPERTIES LINK_FLAGS "-s DEMANGLE_SUPPORT=1 --preload-file ${CMAKE_SOURCE_DIR}/build/res --bind")
endif()

#copy assets
if(EMSCRIPTEN)
    set(RESOURCES_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../res)
else()
    set(RESOURCES_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)
endif()
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/sgb-words.txt DESTINATION ${RESOURCES_DESTINATION})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/letters.bmp DESTINATION ${RESOURCES_DESTINATION})

# Make compiler scream out every possible warning
if(MSVC)
	target_compile_options(${GAME_TITLE} PRIVATE /W4 /WX)
else()
	target_compile_options(${GAME_TITLE} PRIVATE -Wconversion -Wall -Wextra -Wpedantic -Werror)
endif()

#includes
include_directories(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/include)

#libs
#sdl
if(WIN32)
	if(MSVC)
		target_link_libraries(${GAME_TITLE} PUBLIC
			${CMAKE_SOURCE_DIR}/lib/sdl/win32_vc/SDL2main.lib
			${CMAKE_SOURCE_DIR}/lib/sdl/win32_vc/SDL2.lib
		)

		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl/win32_vc/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	else()
		target_link_libraries(${GAME_TITLE} PUBLIC
			mingw32
			${CMAKE_SOURCE_DIR}/lib/sdl/win32_mingw/libSDL2.dll.a
		)

		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl/win32_mingw/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	endif()
else()
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_DL_LIBS} m pthread)
	find_package(SDL2 REQUIRED)
	target_link_libraries(${GAME_TITLE} PUBLIC ${SDL2_LIBRARIES})
endif()