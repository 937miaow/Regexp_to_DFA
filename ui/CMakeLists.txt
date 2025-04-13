cmake_minimum_required(VERSION 3.15)
project(RegexpToDFAUI)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Python
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# 创建可执行文件
add_executable(regexp_to_dfa
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

# 添加头文件搜索路径
target_include_directories(regexp_to_dfa PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 链接regexp_core库
target_link_libraries(regexp_to_dfa PRIVATE regexp_core)

# 添加Python文件
set(PYTHON_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.py
)

# 复制Python文件到构建目录
file(COPY ${PYTHON_FILES} DESTINATION ${CMAKE_BINARY_DIR}/bin)

# 创建自定义目标
add_custom_target(setup_python_env
    COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_BINARY_DIR}/bin/venv
    COMMAND ${CMAKE_BINARY_DIR}/bin/venv/Scripts/python.exe -m pip install --upgrade pip
    COMMAND ${CMAKE_BINARY_DIR}/bin/venv/Scripts/python.exe -m pip install PyQt5==5.15.9
    COMMAND ${CMAKE_BINARY_DIR}/bin/venv/Scripts/python.exe -m pip install pyinstaller
)

# 创建启动脚本
file(WRITE ${CMAKE_BINARY_DIR}/bin/run.bat
    "@echo off\n"
    "cd /d %~dp0\n"
    "call venv\\Scripts\\activate.bat\n"
    "python main.py\n"
    "pause\n"
)

# 创建打包脚本
file(WRITE ${CMAKE_BINARY_DIR}/bin/build_exe.bat
    "@echo off\n"
    "cd /d %~dp0\n"
    "call venv\\Scripts\\activate.bat\n"
    "pyinstaller --onefile --windowed --add-data \"regexp_to_dfa.exe;.\" main.py\n"
    "pause\n"
) 