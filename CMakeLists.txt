cmake_minimum_required(VERSION 3.14)

project(Sylens
	VERSION "0.0.1"
    DESCRIPTION "Computer Graphics Learning Project"
	LANGUAGES CXX)
if (POLICY CMP0144)
  cmake_policy(SET CMP0144 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/3rd/ape/libs/amun/lib/amun)

include(${CMAKE_CURRENT_LIST_DIR}/3rd/ape/libs/amun/lib/amun/amun.cmake)

if (NOT AMUN_INCLUDED)
    message(FATAL_ERROR "Failed to include Amun")
endif()
amun_disable_build_in_source()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

amun_check_build(Sylens NORMAL INSTALL TEST)

#if(${Sylens_NORMAL})
if(FALSE)
	option(APE_BUILD_SHARED "Enable shared library build" ON )
	if (APE_BUILD_SHARED)
		add_library(ape SHARED ${sources})
		target_compile_definitions(ape PRIVATE APE_BUILD_SHARED)
	else()
		add_library(ape STATIC ${sources})
		target_compile_definitions(ape PRIVATE APE_BUILD_STATIC)
	endif()
	target_compile_features(ape PUBLIC cxx_std_20)
	target_compile_definitions(ape PUBLIC
		$<$<CXX_COMPILER_ID:MSVC>:
		_SCL_SECURE_NO_WARNINGS
		_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
		>)
	add_library(Ape::ape ALIAS ape )
endif()

amun_enable_testing()

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED)

add_subdirectory(3rd/ape)
add_subdirectory(src/sylens)
