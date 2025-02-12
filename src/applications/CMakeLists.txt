find_external_library(
  DEPENDENCY_NAME CRYPTOPP
  HEADER_NAME cryptlib.h
  LIBRARY_NAME crypto++
               cryptopp 
  SEARCH_PATHS /usr/include/cryptopp
               ${NS3_WITH_CRYPTOPP}
)

if (NOT CRYPTOPP_FOUND)
    message(FATAL_ERROR "Crypto++ library not found. Please ensure it is installed.")
endif()

# Only process module if include folder and library have been found
include_directories(${CRYPTOPP_INCLUDE_DIRS}) 
set(NS3_CRYPTOPP
    "ON"
    CACHE INTERNAL
          "ON if CRYPTOPP is found in NS3_WITH_CRYPTOPP"
)

build_lib(
  LIBNAME applications
  SOURCE_FILES
    helper/bulk-send-helper.cc
    helper/on-off-helper.cc
    helper/packet-sink-helper.cc
    helper/three-gpp-http-helper.cc
    helper/udp-client-server-helper.cc
    helper/udp-echo-helper.cc
    helper/sptm-client-helper.cc
    helper/sptm-sink-helper.cc
    model/application-packet-probe.cc
    model/bulk-send-application.cc
    model/onoff-application.cc
    model/packet-loss-counter.cc
    model/packet-sink.cc
    model/seq-ts-echo-header.cc
    model/seq-ts-header.cc
    model/seq-ts-size-header.cc
    model/three-gpp-http-client.cc
    model/three-gpp-http-header.cc
    model/three-gpp-http-server.cc
    model/three-gpp-http-variables.cc
    model/udp-client.cc
    model/udp-echo-client.cc
    model/udp-echo-server.cc
    model/udp-server.cc
    model/udp-trace-client.cc
    model/sptm-client.cc
    model/sptm-sink.cc
    model/sptm-header.cc
    model/ah-tag.cc
  HEADER_FILES
    helper/bulk-send-helper.h
    helper/on-off-helper.h
    helper/packet-sink-helper.h
    helper/three-gpp-http-helper.h
    helper/udp-client-server-helper.h
    helper/udp-echo-helper.h
    helper/sptm-client-helper.h
    helper/sptm-sink-helper.h
    model/application-packet-probe.h
    model/bulk-send-application.h
    model/onoff-application.h
    model/packet-loss-counter.h
    model/packet-sink.h
    model/seq-ts-echo-header.h
    model/seq-ts-header.h
    model/seq-ts-size-header.h
    model/three-gpp-http-client.h
    model/three-gpp-http-header.h
    model/three-gpp-http-server.h
    model/three-gpp-http-variables.h
    model/udp-client.h
    model/udp-echo-client.h
    model/udp-echo-server.h
    model/udp-server.h
    model/udp-trace-client.h
    model/sptm-client.h
    model/sptm-sink.h
    model/sptm-header.h
    model/ah-tag.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libstats}
                    ${CRYPTOPP_LIBRARIES}
  TEST_SOURCES
    test/three-gpp-http-client-server-test.cc
    test/bulk-send-application-test-suite.cc
    test/udp-client-server-test.cc
)
