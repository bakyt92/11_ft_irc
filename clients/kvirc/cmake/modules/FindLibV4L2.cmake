# cmake macro to find LibV4L2
#
# Copyright (c) 2009, Jaroslav Reznik <jreznik@redhat.com>
#
# Once done this will define:
#
#  LIBV4L2_FOUND - System has LibV4L2
#  LIBV4L2_INCLUDE_DIR - The LibV4L2 include directory
#  LIBV4L2_LIBRARY - The libraries needed to use LibV4L2
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (LIBV4L2_INCLUDE_DIR AND LIBV4L2_LIBRARY)
    # Already in cache, be silent
    set (LIBV4L2_FIND_QUIETLY TRUE)
endif (LIBV4L2_INCLUDE_DIR AND LIBV4L2_LIBRARY)

find_path (LIBV4L2_INCLUDE_DIR libv4l2.h)

find_library (LIBV4L2_LIBRARY v4l2)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (LibV4L2 DEFAULT_MSG LIBV4L2_INCLUDE_DIR LIBV4L2_LIBRARY)

mark_as_advanced(LIBV4L2_INCLUDE_DIR LIBV4L2_LIBRARY)
