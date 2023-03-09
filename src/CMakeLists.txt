cmake_minimum_Required ( VERSION 2.8 )
project (Split_Merge)
find_package ( OpenCV REQUIRED)
add_executable (app.out src.cpp )
target_link_libraries(app.out ${OpenCV_LIBS})

