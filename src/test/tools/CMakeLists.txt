ExternalProject_Add(ceeaus
  SOURCE_DIR ${meta_BINARY_DIR}/../../data/ceeaus
  DOWNLOAD_DIR ${meta_BINARY_DIR}/downloads
  URL http://web.engr.illinois.edu/~massung1/files/ceeaus.tar.gz
  URL_HASH "SHA256=8ea40b32f34e9ae8aedffe562ad468fc465d1cc0ff6a5c3bdf0ee42bb85c231e"
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
