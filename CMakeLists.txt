cmake_minimum_required (VERSION 2.6)
project (prism)

set(CMAKE_CXX_FLAGS -fno-operator-names)

set(SOURCES
  codegen.cpp
  codegen.h
  ast.cpp
  ast.h
  main.cpp
  node.h
  tokens.l
  parser.y
  Makefile
  test.prism
  vm.h
  vm.cpp
)
include_directories(. xbyak ${PROJECT_BINARY_DIR})


# Create target for the parser
ADD_CUSTOM_TARGET(PrismParser echo "Creating parser.c")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
   SOURCE ${PROJECT_SOURCE_DIR}/tokens.l
   COMMAND lex
   ARGS -o ${PROJECT_BINARY_DIR}/tokens.cpp
        ${PROJECT_SOURCE_DIR}/tokens.l
        ${PROJECT_BINARY_DIR}/parser.hpp
   TARGET PrismParser
 DEPENDS ${PROJECT_BINARY_DIR}/parser.hpp
   OUTPUTS ${PROJECT_BINARY_DIR}/tokens.cpp)

# Create custom command for bison/yacc (note the DEPENDS)
 ADD_CUSTOM_COMMAND(
   SOURCE ${PROJECT_SOURCE_DIR}/parser.y
   COMMAND bison
   ARGS -d -y ${PROJECT_SOURCE_DIR}/parser.y
        -o ${PROJECT_BINARY_DIR}/parser.cpp
   TARGET PrismParser
   OUTPUTS ${PROJECT_BINARY_DIR}/parser.cpp ${PROJECT_BINARY_DIR}/parser.hpp)

# Add parser.c to the list of sources
 SET(SOURCES ${SOURCES} ${PROJECT_BINARY_DIR}/parser.cpp ${PROJECT_BINARY_DIR}/tokens.cpp )

# Since parser.c does not exists yet when cmake is run, mark
# it as generated
 SET_SOURCE_FILES_PROPERTIES(${PROJECT_BINARY_DIR}/parser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${PROJECT_BINARY_DIR}/tokens.cpp GENERATED)

# Include binary directory to include lexer.c in parser.c
 INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/src)

add_subdirectory(yaml-cpp)
add_executable (parser ${SOURCES})
