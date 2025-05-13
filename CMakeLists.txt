cmake_minimum_required(VERSION 3.5)
project(FurRendering)

# Установка стандарта C++ (C++17 рекомендуется для современного OpenGL)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка путей для исходных файлов
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Поиск необходимых пакетов
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Настройка исходных файлов
# Поиск всех исходных файлов (C++, CXX и C)
file(GLOB SOURCES 
    "${SRC_DIR}/*.cxx"
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

# Поиск всех заголовочных файлов (HXX, HPP и H)
file(GLOB HEADERS 
    "${INCLUDE_DIR}/*.hxx"
    "${INCLUDE_DIR}/*.hpp"
    "${INCLUDE_DIR}/*.h"
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Подключение заголовочных директорий
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})

# Подключение библиотек
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Копирование шейдеров и ресурсов в билд-директорию (опционально)
file(COPY fur_shader.verx DESTINATION ${CMAKE_BINARY_DIR})
file(COPY fur_shader.frag DESTINATION ${CMAKE_BINARY_DIR})

# Настройка свойств компиляции для отладки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g)
endif()
