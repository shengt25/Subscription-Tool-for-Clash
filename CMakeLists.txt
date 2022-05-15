project(sub4clash C)
set(CMAKE_C_STANDARD 99)
add_executable(sub4clash src/downloader.c src/main.c src/StrQueue.c)
target_link_libraries(sub4clash curl)