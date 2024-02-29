cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
#project(test-app)
#project(starter_deck_selfplay)
project(test_map_elites)
project(deck_exam)

# list(APPEND CMAKE_PREFIX_PATH /usr/local/lib/python3.8/dist-packages/torch/share/cmake) #libtorchのパスを入力
# list(APPEND CMAKE_PREFIX_PATH /Users/souta/Documents/libtorch)
# find_package(Torch REQUIRED)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# # TensorRT
# set(TENSORRT_INCLUDE /TensorRT-8.5.1/include)
# file(GLOB TENSORRT_LIBRARIES /TensorRT-8.5.1/lib/*.so)

# # TorchTensorRT
# set(TORCHTRT_INCLUDE /usr/local/lib/python3.8/dist-packages/torch_tensorrt/include)
# file(GLOB TORCHTRT_LIBRARIES /usr/local/lib/python3.8/dist-packages/torch_tensorrt/lib/*.so)

# Thread
find_package(Threads REQUIRED)
# For OpenMP
find_package(OpenMP)

# MPI
find_package(MPI REQUIRED)

# add_executable(test-app test5.cpp)
# target_link_libraries(test-app PUBLIC Threads::Threads OpenMP::OpenMP_CXX ${TORCH_LIBRARIES} ${TENSORRT_LIBRARIES} ${TORCHTRT_LIBRARIES})
# target_include_directories(test-app PUBLIC ${TENSORRT_INCLUDE} ${TORCHTRT_INCLUDE})
# target_compile_features(test-app PUBLIC cxx_std_20)
# target_compile_options(test-app PUBLIC -O3 -Wall)

# add_executable(starter_deck_selfplay starter_deck_learning.cpp)
# target_link_libraries(starter_deck_selfplay PUBLIC Threads::Threads OpenMP::OpenMP_CXX ${TORCH_LIBRARIES} ${TENSORRT_LIBRARIES} ${TORCHTRT_LIBRARIES})
# target_include_directories(starter_deck_selfplay PUBLIC ${TENSORRT_INCLUDE} ${TORCHTRT_INCLUDE})
# target_compile_features(starter_deck_selfplay PUBLIC cxx_std_20)
# target_compile_options(starter_deck_selfplay PUBLIC -O3 -Wall)

add_executable(test_map_elites map_elites_oneagent.cpp)
target_link_libraries(test_map_elites PUBLIC Threads::Threads OpenMP::OpenMP_CXX MPI::MPI_CXX ${TORCH_LIBRARIES} ${TENSORRT_LIBRARIES} ${TORCHTRT_LIBRARIES})
target_include_directories(test_map_elites PUBLIC ${TENSORRT_INCLUDE} ${TORCHTRT_INCLUDE})
target_compile_features(test_map_elites PUBLIC cxx_std_20)
target_compile_options(test_map_elites PUBLIC -O3 -Wall)

add_executable(deck_exam deck_exam.cpp)
target_link_libraries(deck_exam PUBLIC Threads::Threads OpenMP::OpenMP_CXX MPI::MPI_CXX ${TORCH_LIBRARIES} ${TENSORRT_LIBRARIES} ${TORCHTRT_LIBRARIES})
target_include_directories(deck_exam PUBLIC ${TENSORRT_INCLUDE} ${TORCHTRT_INCLUDE})
target_compile_features(deck_exam PUBLIC cxx_std_20)
target_compile_options(deck_exam PUBLIC -O3 -Wall)