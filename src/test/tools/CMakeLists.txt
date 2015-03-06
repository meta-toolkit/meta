ExternalProject_Add(ceeaus
  SOURCE_DIR ${meta_BINARY_DIR}/../../data/ceeaus
  DOWNLOAD_DIR ${meta_BINARY_DIR}/downloads
  URL http://web.engr.illinois.edu/~massung1/files/ceeaus.tar.gz
  URL_HASH "SHA256=dbcdecc4098bd02dd31c35930fad9ae81a85dc07ac79f734a127fe915a52ca25"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Add(breast-cancer
  SOURCE_DIR ${meta_BINARY_DIR}/../../data/breast-cancer
  DOWNLOAD_DIR ${meta_BINARY_DIR}/../downloads
  URL http://web.engr.illinois.edu/~massung1/files/breast-cancer.tar.gz
  URL_HASH "SHA256=4944e327a359d024f69c75a3da27dc5ab8f6493bdaa5c533bac5c8510f84bf21"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

add_executable(unit-test unit-test.cpp)
add_dependencies(unit-test ceeaus breast-cancer)
target_link_libraries(unit-test meta-testing)
