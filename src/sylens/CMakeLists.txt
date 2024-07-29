cmake_minimum_required(VERSION 3.14)

project(sylens LANGUAGES CXX)

add_executable(sylens main.cpp sylens.cpp utils.cpp)
target_include_directories(sylens PUBLIC "${gsl_include_dir}")
target_compile_features(sylens PUBLIC cxx_std_23)
set_property(TARGET sylens PROPERTY POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(sylens
  Ape::estl
  glfw
  Vulkan::Vulkan
  Boost::boost
)
 
