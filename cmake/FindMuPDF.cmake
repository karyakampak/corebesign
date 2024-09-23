# FindMuPDF.cmake

# Set default paths if necessary
set(MUPDF_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/mupdf/include")
set(MUPDF_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/mupdf/lib")

# Find the MuPDF include directory
find_path(MUPDF_INCLUDE_DIR NAMES mupdf/fitz.h PATHS ${MUPDF_INCLUDE_DIR})

# Find the MuPDF library
find_library(MUPDF_LIBRARIES NAMES mupdf PATHS ${MUPDF_LIB_DIR})

# Check if both include directory and library are found
if (MUPDF_INCLUDE_DIR AND MUPDF_LIBRARIES)
    set(MUPDF_FOUND TRUE)
else()
    set(MUPDF_FOUND FALSE)
endif()

if (MUPDF_FOUND)
    message(STATUS "Found MuPDF: ${MUPDF_LIBRARIES} (include: ${MUPDF_INCLUDE_DIR})")
    set(MUPDF_INCLUDE_DIRS ${MUPDF_INCLUDE_DIR})
    set(MUPDF_LIBRARIES ${MUPDF_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find MuPDF")
endif()
