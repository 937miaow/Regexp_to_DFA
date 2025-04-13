# 添加源文件
set(SOURCES
    graph.cpp
    nfa.cpp
    dfa.cpp
)

# 添加头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 创建库
add_library(regexp_core STATIC ${SOURCES})

# 设置输出文件名
set_target_properties(regexp_core PROPERTIES
    OUTPUT_NAME "regexp_core"
)

# 添加编译选项
if(MSVC)
    target_compile_options(regexp_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(regexp_core PRIVATE -Wall -Wextra)
endif()

# 安装目标
install(TARGETS regexp_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
) 