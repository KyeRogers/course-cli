# CMake generated Testfile for 
# Source directory: /home/kye/Escritorio/dev/projects/course-cli
# Build directory: /home/kye/Escritorio/dev/projects/course-cli/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[assignment-tests]=] "/home/kye/Escritorio/dev/projects/course-cli/build/assignment-tests")
set_tests_properties([=[assignment-tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kye/Escritorio/dev/projects/course-cli/CMakeLists.txt;76;add_test;/home/kye/Escritorio/dev/projects/course-cli/CMakeLists.txt;0;")
add_test([=[manager-tests]=] "/home/kye/Escritorio/dev/projects/course-cli/build/manager-tests")
set_tests_properties([=[manager-tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kye/Escritorio/dev/projects/course-cli/CMakeLists.txt;77;add_test;/home/kye/Escritorio/dev/projects/course-cli/CMakeLists.txt;0;")
subdirs("_deps/nlohmann_json-build")
subdirs("_deps/ftxui-build")
subdirs("_deps/doctest-build")
