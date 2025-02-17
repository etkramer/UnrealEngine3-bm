# This must not contain spaces. You can rename it for your game, but it's
#  generally safe to leave this alone, too.
PROJECT(UnrealEngine3)

# Set this to ON if you're a licensee of UnrealEngine.
#  Epic Games should leave this as OFF. This determines whether to use
#  the Core/Inc/Licensee directory in builds or not.
SET(DEFAULT_LICENSEE OFF)

# This should match one of the #defines at the top of the header
#  Launch/Inc/LaunchPrivate.h ... presumably you added your own there, in
#  addition to the Epic-specified names. If you're a licensee, hardcoding
#  this here makes sense, but this is a default that is tied to a CMake option
#  if you need to build multiple games out of this codebase.
SET(DEFAULT_GAMENAME "UTGAME")

# Set this to the name of your final game's .exe ... if you wanted
#  mygame.exe, you'd specify "mygame" here. We'll append the ".exe" later.
# If you're a licensee, hardcoding this here makes sense, but this is a
#  default that is tied to a CMake option if you need to build multiple games
#  out of this codebase.
SET(DEFAULT_EXENAME "UT3")

# Set this to ON if you want to use precompiled headers. You probably do.
# This is a default that is tied to a CMake option if you need to build
#  multiple games out of this codebase.
SET(DEFAULT_PRECOMPILED_HEADERS ON)

# Set this to ON to force the Null RHI ... you probably want this OFF, unless
#  you're building only the dedicated server.
SET(DEFAULT_USE_NULL_RHI ON)

# Set this to ON to include lzopro support...you need to pay a licensing fee
#  for this, but if you own a license, you want to toggle this on.
SET(DEFAULT_USE_LZOPRO ON)


# -- Don't touch below this line if you don't know what you're doing. --

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCCompilerFlag)

# Cache values so you can change these in the config UI.
SET(UNREAL_BUILD_WITH_DEBUG_SYMBOLS ON CACHE BOOL "Build with debug symbols")
SET(UNREAL_LICENSEE "${DEFAULT_LICENSEE}" CACHE BOOL "ON if you're a licensee, OFF if you're Epic Games")
SET(UNREAL_GAMENAME "${DEFAULT_GAMENAME}" CACHE STRING "The #defined name of the game")
SET(UNREAL_EXENAME "${DEFAULT_EXENAME}" CACHE STRING "The name of the final .exe")
SET(UNREAL_PRECOMPILED_HEADERS "${DEFAULT_PRECOMPILED_HEADERS}" CACHE BOOL "Use PCH when building?")
SET(UNREAL_USE_NULL_RHI "${DEFAULT_USE_NULL_RHI}" CACHE BOOL "Force renderer to be Null RHI?")
SET(UNREAL_USE_LZOPRO "${DEFAULT_USE_LZOPRO}" CACHE BOOL "Build with lzopro support?")


SET(CMAKE_USE_RELATIVE_PATHS ON CACHE BOOL "Use relative paths" FORCE )


# I hate that they define "WIN32" ... we're about to move to Win64...I hope!
IF(WIN32 AND NOT WINDOWS)
    SET(WINDOWS TRUE)
ENDIF(WIN32 AND NOT WINDOWS)

# Bleh, let's do it for "APPLE" too.
IF(APPLE AND NOT MACOSX)
    SET(MACOSX TRUE)
ENDIF(APPLE AND NOT MACOSX)

IF(NOT CMAKE_BUILD_TYPE)
    SET(UNREAL_DEBUG_BUILD ON)
ENDIF(NOT CMAKE_BUILD_TYPE)
IF(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET(UNREAL_DEBUG_BUILD ON)
ENDIF(CMAKE_BUILD_TYPE EQUAL "Debug")

IF(MACOSX)  # CMake reports CMAKE_SYSTEM_NAME as "Darwin" here...
    SET(UNREAL_OS "macosx")
ELSE(MACOSX)
    STRING(TOLOWER "${CMAKE_SYSTEM_NAME}" UNREAL_OS)
ENDIF(MACOSX)

IF(UNREAL_PRECOMPILED_HEADERS)
    INCLUDE(./PCHSupport.cmake)
    IF(PCHSupport_FOUND)
        SET(UNREAL_PERMIT_PCH ON)
    ELSE(PCHSupport_FOUND)
        MESSAGE(STATUS "No precompiled header support found. We won't use them.")
        SET(UNREAL_PERMIT_PCH OFF)
    ENDIF(PCHSupport_FOUND)
ENDIF(UNREAL_PRECOMPILED_HEADERS)


# See what CPU we appear to be targeting...
#  It would be nice if CMake handled this for us.
MACRO(CHECK_CPU _CDEFS _NAME _VALUE)
    CHECK_C_SOURCE_COMPILES("
        #if ( ${_CDEFS} )
        int main(int argc, char **argv) { int yup = 1; return 0; }
        #else
        #error Not targeting this CPU architecture.
        #endif
        " ${_VALUE}
    )

    IF(${_VALUE})
        IF(UNREAL_CHOSE_CPU)
            MESSAGE(STATUS "We appear to see two different CPU architectures!")
            MESSAGE(STATUS "We saw '${UNREAL_CPU}' and '${_NAME}'.")
            MESSAGE(FATAL_ERROR "Please fix this before continuing.")
        ENDIF(UNREAL_CHOSE_CPU)
        SET(UNREAL_CHOSE_CPU TRUE)
        SET(UNREAL_CPU ${_NAME})
    ENDIF(${_VALUE})
ENDMACRO(CHECK_CPU)

CHECK_CPU(
    "defined(__i386__) || defined(__i686__) || defined(_M_X86)"
    "x86" UNREAL_IS_X86
)

CHECK_CPU(
    "defined(__x86_64__) || defined(_M_X64)"
    "amd64" UNREAL_IS_AMD64
)

CHECK_CPU(
    "defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__)"
    "powerpc" UNREAL_IS_POWERPC
)

IF(NOT UNREAL_CHOSE_CPU)
    MESSAGE(STATUS "We don't appear to be targeting a CPU that we support.")
    MESSAGE(FATAL_ERROR "Please fix this before continuing.")
ENDIF(NOT UNREAL_CHOSE_CPU)

SET(UNREAL_TARGET "${UNREAL_OS}-${UNREAL_CPU}")


# Look for some system libraries we'll need...

IF(MACOSX)
    SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} "-framework Cocoa")
ELSE(MACOSX)
    FIND_PACKAGE(Threads)
    SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    CHECK_LIBRARY_EXISTS("dl" "dlopen" "" HAVE_LIBDL)
    IF(HAVE_LIBDL)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} dl)
    ENDIF(HAVE_LIBDL)
    CHECK_LIBRARY_EXISTS("m" "sin" "" HAVE_LIBM)
    IF(HAVE_LIBM)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} m)
    ENDIF(HAVE_LIBM)
ENDIF(MACOSX)


# Other compiler wankery...

IF(CMAKE_COMPILER_IS_GNUCC)
    # By default, we add "-ggdb3" to the command line, even in release builds.
    #  Don't forget to strip the binaries before shipping!
    # Also, this is a massive amount of debug info...if your binary is
    #  several hundred megabytes larger than you expected, than that's
    #  normal, and should also remind you to strip it before shipping.  :)
    # Also, this makes linking take longer than the compile on this machine,
    #  So you might want to set this for specific modules (set
    #  UNREAL_DEBUG_SYMS_Core, UNREAL_DEBUG_SYMS_IpDrv, etc), or turn it off
    #  across the board with UNREAL_BUILD_WITH_DEBUG_SYMBOLS=OFF.
    IF(MACOSX)
        SET(DEBUG_SYMBOLS_FLAG "-gdwarf-2")
    ELSE(MACOSX)
        SET(DEBUG_SYMBOLS_FLAG "-ggdb3")
    ENDIF(MACOSX)

    # See if -fno-stack-protector is available to us.
    # It doesn't seem to work well, and it adds bulk to the binary.
    # (We'll leave it in debug builds though, maybe it'll catch something.)
    CHECK_C_COMPILER_FLAG("-fno-stack-protector" UNREAL_GCC_HAS_STACKPROT)

    ADD_DEFINITIONS(-pipe -fsigned-char)

    # !!! FIXME: try to lose this later...
    ADD_DEFINITIONS(-fno-strict-aliasing)

    IF(NOT UNREAL_DEBUG_BUILD)
        ADD_DEFINITIONS(-ffast-math -fno-math-errno -falign-loops=16)
        IF(UNREAL_IS_X86)
            IF(NOT MACOSX)  # Probably can let Apple make the call here...
                ADD_DEFINITIONS(-march=pentium4 -mtune=pentium4)
            ENDIF(NOT MACOSX)
        ENDIF(UNREAL_IS_X86)

        IF(UNREAL_IS_X86 OR UNREAL_IS_AMD64)
            ADD_DEFINITIONS(-mmmx -msse2 -mfpmath=sse)
        ENDIF(UNREAL_IS_X86 OR UNREAL_IS_AMD64)

        IF(MACOSX)
            ADD_DEFINITIONS(-mdynamic-no-pic)
        ENDIF(MACOSX)

        IF(UNREAL_GCC_HAS_STACKPROT)
            ADD_DEFINITIONS(-fno-stack-protector)
        ENDIF(UNREAL_GCC_HAS_STACKPROT)
    ENDIF(NOT UNREAL_DEBUG_BUILD)

    IF(MACOSX AND UNREAL_IS_POWERPC)
        ADD_DEFINITIONS(-faltivec -force_cpusubtype_ALL)
    ENDIF(MACOSX AND UNREAL_IS_POWERPC)

    IF(NOT MACOSX)
        ADD_DEFINITIONS(-pthread)
    ENDIF(NOT MACOSX)

    # CMake mailing list had this hack for getting rid of -rdynamic:
    #   http://public.kitware.com/pipermail/cmake/2006-July/010404.html
    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
        SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
    ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Don't use -rpath.
    SET(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE )

    # !!! FIXME: get this in later.
    #ADD_DEFINITIONS(-Wall)
    # !!! FIXME: yeah, that would be nice, too...
    #ADD_DEFINITIONS(-Werror)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# !!! FIXME: set this for non-gcc compilers.
#SET(DEBUG_SYMBOLS_FLAG "-ggdb3")


# Get some universal truths out of the way here...
ADD_DEFINITIONS(-DUNICODE=1 -DGSI_UNICODE=1 -D__UNREAL__=1 -D_REENTRANT=1)
ADD_DEFINITIONS("-DGAMENAME=${UNREAL_GAMENAME}")

IF(UNREAL_DEBUG_BUILD)
    ADD_DEFINITIONS(-DDEBUG=1 -D_DEBUG=1)
ELSE(UNREAL_DEBUG_BUILD)
    ADD_DEFINITIONS(-DNDEBUG=1 -D_NDEBUG=1)
ENDIF(UNREAL_DEBUG_BUILD)


# Some things wxWidgets wants defined...

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)
IF(UNIX)
    IF(MACOSX)
        ADD_DEFINITIONS(-D__WXMAC__)
    ELSE(MACOSX)
        ADD_DEFINITIONS(-D__WXGTK__)
    ENDIF(MACOSX)
ENDIF(UNIX)
IF(WINDOWS)
    ADD_DEFINITIONS(-D__WXWIN__)
ENDIF(WINDOWS)


# !!! FIXME
ADD_DEFINITIONS(-DWITH_TTS=0)
ADD_DEFINITIONS(-DWITH_SPEECH_RECOGNITION=0)
ADD_DEFINITIONS(-DUSING_REMOTECONTROL=0)
ADD_DEFINITIONS(-DUSE_BINK_CODEC=0)

IF(UNREAL_USE_NULL_RHI)
    ADD_DEFINITIONS(-DUSE_NULL_RHI=1)
ENDIF(UNREAL_USE_NULL_RHI)

# !!! FIXME: get this in there at some point...
#ADD_DEFINITIONS(-DFINAL_RELEASE=1)

# Use the original vorbis sources from xiph.org for now (the other directory
#  is Blacksword's SSE fork, and it's really broken on GCC and non-Intel
#  processors at the moment).
SET(LIBOGG_DIR "libogg-1.1.3")
SET(LIBVORBIS_DIR "libvorbis-1.1.2-original")


# UnrealEngine includes...

# !!! FIXME: get wxWidgets into the build properly.
# !!! FIXME: Move EditorPrivate.h to Editor/Inc so we can remove Editor/Src
INCLUDE_DIRECTORIES(
    .
    Core/Inc
    Engine/Inc
    IpDrv/Inc
    OnlineSubsystemPC/Inc
    OnlineSubsystemGameSpy/Inc
    UnrealScriptTest/Inc
    Editor/Inc
    Editor/Src
    D3DDrv/Inc
    OpenGLDrv/Inc
    SDLDrv/Inc
    GameFramework/Inc
    WarfareGame/Inc
    UTGame/Inc
    ExampleGame/Inc
    UnrealEd/Inc
    ALAudio/Inc
    Launch/Inc
    ../External/OpenGL/include
    ../External/Cg/include
    ../External/wxExtended/wxDockit/include
    ../External/wxWidgets-${UNREAL_TARGET}/include
    ../External/libPNG
    ../External/zlib/Inc
    ../External/lzopro/include
    ../External/${LIBOGG_DIR}/include
    ../External/${LIBVORBIS_DIR}/include
    ../External/PhysX/SDKs/Foundation/Include
    ../External/PhysX/SDKs/Physics/Include
    ../External/PhysX/SDKs/Cooking/Include
    ../External/PhysX/SDKs/Quickload/include
    ../External/PhysX/SDKs/PhysXLoader/include
    ../External/FaceFX/FxSDK/Inc
    ../External/Bink
    ../External/OpenAL-1.1/include
    ../External/GameSpy
    ../External/SDL12/include
)

IF(UNREAL_LICENSEE)
    INCLUDE_DIRECTORIES(./Core/Inc/Licensee)
ELSE(UNREAL_LICENSEE)
    INCLUDE_DIRECTORIES(./Core/Inc/Epic)
ENDIF(UNREAL_LICENSEE)


# Concessions for PhysX...

#ADD_DEFINITIONS(-DNX_USE_SDK_STATICLIBS=1)

# PhysX needs you to define this, but I'd rather they check
#  for things like _M_X64 or whatever in their headers...
# !!! FIXME: see about changing the PhysX headers for a future PhysX release.
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    ADD_DEFINITIONS(-DNX32)
    SET(UNREAL_CHOSE_PTRSIZE TRUE)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ADD_DEFINITIONS(-DNX64)
    SET(UNREAL_CHOSE_PTRSIZE TRUE)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
IF(NOT UNREAL_CHOSE_PTRSIZE)
    MESSAGE(FATAL_ERROR "Uhoh, we're not targeting a 32 or 64 bit system!")
ENDIF(NOT UNREAL_CHOSE_PTRSIZE)

# PhysX expects this #define...
# (And GameSpy expects _LINUX ...)
# !!! FIXME: would like to change that in a future PhysX release...
# !!! FIXME:  (look for __linux__ instead...)
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    ADD_DEFINITIONS(-DLINUX=1)
    ADD_DEFINITIONS(-D_LINUX=1)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

# GameSpy's SDK expects this.
IF(MACOSX)
    ADD_DEFINITIONS(-D_MACOSX=1)
ENDIF(MACOSX)


# Define middleware libraries we link against...
# Use a GLOB so we don't have to mess with specific filenames as these lists
#  change over time.

FILE(GLOB NOVODEX_LIBS
    "../External/PhysX/SDKs/lib/${UNREAL_TARGET}/*.*"
)

FILE(GLOB SPEEDTREERT_LIBS
    "../External/SpeedTreeRT/lib/${UNREAL_TARGET}/*.*"
)

FILE(GLOB FACEFX_LIBS
    "../External/FaceFX/FxSDK/lib/${UNREAL_TARGET}/*.*"
)

FILE(GLOB WXWIDGETS_LIBS
    "../External/wxWidgets-${UNREAL_TARGET}/lib/*.*"
)

IF(UNIX)
    FILE(GLOB SDL_LIBS
        "../External/SDL12/lib/${UNREAL_TARGET}/*.*"
    )
ENDIF(UNIX)


# Spit out some build details here...

MACRO(MESSAGE_BOOL_OPTION _NAME _VALUE)
    IF(${_VALUE})
        MESSAGE(STATUS "  ${_NAME}: ON")
    ELSE(${_VALUE})
        MESSAGE(STATUS "  ${_NAME}: OFF")
    ENDIF(${_VALUE})
ENDMACRO(MESSAGE_BOOL_OPTION)

MACRO(MESSAGE_STR_OPTION _NAME _VALUE)
    MESSAGE(STATUS "  ${_NAME}: '${${_VALUE}}'")
ENDMACRO(MESSAGE_STR_OPTION)

MESSAGE(STATUS "We will build with the following options:")
MESSAGE_STR_OPTION("Target" UNREAL_TARGET)
MESSAGE_STR_OPTION("Game #define" UNREAL_GAMENAME)
MESSAGE_STR_OPTION("Binary name" UNREAL_EXENAME)
MESSAGE_BOOL_OPTION("Licensee build" UNREAL_LICENSEE)
MESSAGE_BOOL_OPTION("Precompiled headers" UNREAL_PRECOMPILED_HEADERS)
MESSAGE_BOOL_OPTION("Debug build" UNREAL_DEBUG_BUILD)
MESSAGE_BOOL_OPTION("Debug symbols" UNREAL_BUILD_WITH_DEBUG_SYMBOLS)


# Okay, we're all set. Now just build up the lists of things we need to
#  build and glue together.

MACRO(REPORT_MODULE_DEBUG_SYMS _MODULE)
    IF(UNREAL_DEBUG_SYMS_${_MODULE} AND NOT UNREAL_BUILD_WITH_DEBUG_SYMBOLS)
        MESSAGE(STATUS "Building '${_MODULE}' with debug symbols")
    ENDIF(UNREAL_DEBUG_SYMS_${_MODULE} AND NOT UNREAL_BUILD_WITH_DEBUG_SYMBOLS)
    IF(NOT UNREAL_DEBUG_SYMS_${_MODULE} AND UNREAL_BUILD_WITH_DEBUG_SYMBOLS)
        MESSAGE(STATUS "Building '${_MODULE}' without debug symbols")
    ENDIF(NOT UNREAL_DEBUG_SYMS_${_MODULE} AND UNREAL_BUILD_WITH_DEBUG_SYMBOLS)
ENDMACRO(REPORT_MODULE_DEBUG_SYMS)

# _GAMENAME can be a name (WARGAME, UTGAME, EXAMPLEGAME, etc), or a list:
#  "WARGAME;UTGAME"
# Or "ALL" to use everywhere.
MACRO(ADD_UNREAL_MODULE _GAMENAME _MODULE _SRCS _PCHHEADER)
#IF("${_MODULE}" STREQUAL "OpenGLDrv")
    IF("${_GAMENAME}" STREQUAL "ALL")
        SET(_MODULE_MATCHES ON)
    ELSE("${_GAMENAME}" STREQUAL "ALL")
        SET(_MODULE_MATCHES OFF)
        FOREACH(LOOPER ${_GAMENAME})
            IF(UNREAL_GAMENAME STREQUAL "${LOOPER}")
                SET(_MODULE_MATCHES ON)
            ENDIF(UNREAL_GAMENAME STREQUAL "${LOOPER}")
        ENDFOREACH(LOOPER)
    ENDIF("${_GAMENAME}" STREQUAL "ALL")

    IF(_MODULE_MATCHES)
        #MESSAGE(STATUS "Module ${_MODULE} SRCS ${_SRCS}")
        ADD_LIBRARY(${_MODULE} STATIC ${_SRCS})

        OPTION(UNREAL_DEBUG_SYMS_${_MODULE} "Specifically build '${_MODULE}' with debug symbols" ${UNREAL_BUILD_WITH_DEBUG_SYMBOLS})
        IF(UNREAL_DEBUG_SYMS_${_MODULE})
            SET(EXTRA_PCH_CFLAGS_${_MODULE} "${DEBUG_SYMBOLS_FLAG}")
            SET_TARGET_PROPERTIES(${_MODULE} PROPERTIES COMPILE_FLAGS ${DEBUG_SYMBOLS_FLAG})
        ENDIF(UNREAL_DEBUG_SYMS_${_MODULE})

        REPORT_MODULE_DEBUG_SYMS("${_MODULE}")

        IF(UNREAL_PERMIT_PCH)
            IF(NOT "${_PCHHEADER}" STREQUAL "")
                ADD_PRECOMPILED_HEADER(${_MODULE} "${CMAKE_HOME_DIRECTORY}/${_PCHHEADER}" "${EXTRA_PCH_CFLAGS_${_MODULE}}")

                # !!! FIXME
                # Workaround for now...make this module depend on the precompiled
                #  headers of every previously added Unreal module...this makes PCH
                #  generation serialize, and thus entire modules can't be built in
                #  parallel until previous PCH are rebuilt, but it works around a race
                #  condition. We need a better solution here.
                IF("${UNREAL_MODULES_PCH}" STREQUAL "")
                    SET(UNREAL_MODULES_PCH "${_MODULE}_pch_dephelp;pch_Generate_${_MODULE}")
                ELSE("${UNREAL_MODULES_PCH}" STREQUAL "")
                    ADD_DEPENDENCIES("${_MODULE}_pch_dephelp" ${UNREAL_MODULES_PCH})
                    SET(UNREAL_MODULES_PCH "${_MODULE}_pch_dephelp;pch_Generate_${_MODULE};${UNREAL_MODULES_PCH}")
                ENDIF("${UNREAL_MODULES_PCH}" STREQUAL "")
            ENDIF(NOT "${_PCHHEADER}" STREQUAL "")
        ENDIF(UNREAL_PERMIT_PCH)

        SET(UNREAL_MODULES "${_MODULE};${UNREAL_MODULES}")

    ENDIF(_MODULE_MATCHES)
#ENDIF("${_MODULE}" STREQUAL "OpenGLDrv")
ENDMACRO(ADD_UNREAL_MODULE)


# External things that are fast and easy to build from source are just built
#  as part of Unreal, so we can share compiler flags and not mess around
#  with external builds. Some things are too complicated or big to build well
#  this way, and are prebuilt as static libraries outside of Unreal.

SET(LZOPRO_SRCS
    ../External/lzopro/src/lzo1_99.c
    ../External/lzopro/src/lzo1a_99.c
    ../External/lzopro/src/lzo1a.c
    ../External/lzopro/src/lzo1b_1.c
    ../External/lzopro/src/lzo1b_2.c
    ../External/lzopro/src/lzo1b_3.c
    ../External/lzopro/src/lzo1b_4.c
    ../External/lzopro/src/lzo1b_5.c
    ../External/lzopro/src/lzo1b_6.c
    ../External/lzopro/src/lzo1b_7.c
    ../External/lzopro/src/lzo1b_8.c
    ../External/lzopro/src/lzo1b_99.c
    ../External/lzopro/src/lzo1b_9.c
    ../External/lzopro/src/lzo1b_9x.c
    ../External/lzopro/src/lzo1b_cc.c
    ../External/lzopro/src/lzo1b_d1.c
    ../External/lzopro/src/lzo1b_d2.c
    ../External/lzopro/src/lzo1b_rr.c
    ../External/lzopro/src/lzo1b_xx.c
    ../External/lzopro/src/lzo1.c
    ../External/lzopro/src/lzo1c_1.c
    ../External/lzopro/src/lzo1c_2.c
    ../External/lzopro/src/lzo1c_3.c
    ../External/lzopro/src/lzo1c_4.c
    ../External/lzopro/src/lzo1c_5.c
    ../External/lzopro/src/lzo1c_6.c
    ../External/lzopro/src/lzo1c_7.c
    ../External/lzopro/src/lzo1c_8.c
    ../External/lzopro/src/lzo1c_99.c
    ../External/lzopro/src/lzo1c_9.c
    ../External/lzopro/src/lzo1c_9x.c
    ../External/lzopro/src/lzo1c_cc.c
    ../External/lzopro/src/lzo1c_d1.c
    ../External/lzopro/src/lzo1c_d2.c
    ../External/lzopro/src/lzo1c_rr.c
    ../External/lzopro/src/lzo1c_xx.c
    ../External/lzopro/src/lzo1f_1.c
    ../External/lzopro/src/lzo1f_9x.c
    ../External/lzopro/src/lzo1f_d1.c
    ../External/lzopro/src/lzo1f_d2.c
    ../External/lzopro/src/lzo1x_1.c
    ../External/lzopro/src/lzo1x_1k.c
    ../External/lzopro/src/lzo1x_1l.c
    ../External/lzopro/src/lzo1x_1o.c
    ../External/lzopro/src/lzo1x_9x.c
    ../External/lzopro/src/lzo1x_d1.c
    ../External/lzopro/src/lzo1x_d2.c
    ../External/lzopro/src/lzo1x_d3.c
    ../External/lzopro/src/lzo1x_d4_a00.c
    ../External/lzopro/src/lzo1x_d4_a01.c
    ../External/lzopro/src/lzo1x_d4_a02.c
    ../External/lzopro/src/lzo1x_d4_a03.c
    ../External/lzopro/src/lzo1x_d4_a04.c
    ../External/lzopro/src/lzo1x_d4_a05.c
    ../External/lzopro/src/lzo1x_d4_a06.c
    ../External/lzopro/src/lzo1x_d4_a07.c
    ../External/lzopro/src/lzo1x_d4.c
    ../External/lzopro/src/lzo1x_d4s.c
    ../External/lzopro/src/lzo1x_o.c
    ../External/lzopro/src/lzo1y_1.c
    ../External/lzopro/src/lzo1y_9x.c
    ../External/lzopro/src/lzo1y_d1.c
    ../External/lzopro/src/lzo1y_d2.c
    ../External/lzopro/src/lzo1y_d3.c
    ../External/lzopro/src/lzo1y_d4_a00.c
    ../External/lzopro/src/lzo1y_d4_a01.c
    ../External/lzopro/src/lzo1y_d4_a02.c
    ../External/lzopro/src/lzo1y_d4_a03.c
    ../External/lzopro/src/lzo1y_d4_a04.c
    ../External/lzopro/src/lzo1y_d4_a05.c
    ../External/lzopro/src/lzo1y_d4_a06.c
    ../External/lzopro/src/lzo1y_d4_a07.c
    ../External/lzopro/src/lzo1y_d4.c
    ../External/lzopro/src/lzo1y_d4s.c
    ../External/lzopro/src/lzo1y_o.c
    ../External/lzopro/src/lzo1z_9x.c
    ../External/lzopro/src/lzo1z_d1.c
    ../External/lzopro/src/lzo1z_d2.c
    ../External/lzopro/src/lzo1z_d3.c
    ../External/lzopro/src/lzo2a_9x.c
    ../External/lzopro/src/lzo2a_d1.c
    ../External/lzopro/src/lzo2a_d2.c
    ../External/lzopro/src/lzo_crc.c
    ../External/lzopro/src/lzo_init.c
    ../External/lzopro/src/lzo_ptr.c
    ../External/lzopro/src/lzo_str.c
    ../External/lzopro/src/lzo_util.c
    ../External/lzopro/src/pinit.c
    ../External/lzopro/src/plzo1x_1f.c
    ../External/lzopro/src/plzo1x_1g.c
    ../External/lzopro/src/plzo1x_1h.c
    ../External/lzopro/src/plzo1x_1i.c
    ../External/lzopro/src/plzo1x_1j.c
    ../External/lzopro/src/plzo1x_1k.c
    ../External/lzopro/src/plzo1x_1l.c
    ../External/lzopro/src/plzo1x_1m.c
    ../External/lzopro/src/plzo1x_1n.c
    ../External/lzopro/src/plzo1x_1o.c
    ../External/lzopro/src/plzo1x_1p.c
    ../External/lzopro/src/plzo1x_99.c
    ../External/lzopro/src/plzo1y_1f.c
    ../External/lzopro/src/plzo1y_1g.c
    ../External/lzopro/src/plzo1y_1h.c
    ../External/lzopro/src/plzo1y_1i.c
    ../External/lzopro/src/plzo1y_1j.c
    ../External/lzopro/src/plzo1y_1k.c
    ../External/lzopro/src/plzo1y_1l.c
    ../External/lzopro/src/plzo1y_1m.c
    ../External/lzopro/src/plzo1y_1n.c
    ../External/lzopro/src/plzo1y_1o.c
    ../External/lzopro/src/plzo1y_1p.c
)
IF(UNREAL_USE_LZOPRO)
    ADD_UNREAL_MODULE("ALL" "LzoPro" "${LZOPRO_SRCS}" "")
ENDIF(UNREAL_USE_LZOPRO)


SET(ZLIB_SRCS
    ../External/zlib/Src/adler32.c
    ../External/zlib/Src/compress.c
    ../External/zlib/Src/crc32.c
    ../External/zlib/Src/deflate.c
    ../External/zlib/Src/inffast.c
    ../External/zlib/Src/inflate.c
    ../External/zlib/Src/inftrees.c
    ../External/zlib/Src/trees.c
    ../External/zlib/Src/uncompr.c
    ../External/zlib/Src/zutil.c
)
ADD_UNREAL_MODULE("ALL" "Zlib" "${ZLIB_SRCS}" "")


SET(OGGVORBIS_SRCS
    ../External/${LIBOGG_DIR}/src/bitwise.c
    ../External/${LIBOGG_DIR}/src/framing.c
    ../External/${LIBVORBIS_DIR}/lib/analysis.c
    ../External/${LIBVORBIS_DIR}/lib/bitrate.c
    ../External/${LIBVORBIS_DIR}/lib/block.c
    ../External/${LIBVORBIS_DIR}/lib/codebook.c
    ../External/${LIBVORBIS_DIR}/lib/envelope.c
    ../External/${LIBVORBIS_DIR}/lib/floor0.c
    ../External/${LIBVORBIS_DIR}/lib/floor1.c
    ../External/${LIBVORBIS_DIR}/lib/info.c
    ../External/${LIBVORBIS_DIR}/lib/lpc.c
    ../External/${LIBVORBIS_DIR}/lib/lsp.c
    ../External/${LIBVORBIS_DIR}/lib/mapping0.c
    ../External/${LIBVORBIS_DIR}/lib/mdct.c
    ../External/${LIBVORBIS_DIR}/lib/psy.c
    ../External/${LIBVORBIS_DIR}/lib/registry.c
    ../External/${LIBVORBIS_DIR}/lib/res0.c
    ../External/${LIBVORBIS_DIR}/lib/sharedbook.c
    ../External/${LIBVORBIS_DIR}/lib/smallft.c
    ../External/${LIBVORBIS_DIR}/lib/synthesis.c
    ../External/${LIBVORBIS_DIR}/lib/vorbisfile.c
    ../External/${LIBVORBIS_DIR}/lib/window.c
)
ADD_UNREAL_MODULE("ALL" "OggVorbis" "${OGGVORBIS_SRCS}" "")



# Add modules here.

SET(CORE_SRCS
    Core/Src/BitArray.cpp
    Core/Src/Color.cpp
    Core/Src/Core.cpp
    Core/Src/Distributions.cpp
    Core/Src/FCallbackDevice.cpp
    Core/Src/FConfigCacheIni.cpp
    Core/Src/FFileManagerGeneric.cpp
    Core/Src/FFileManagerWindows.cpp
    Core/Src/FFileManagerUnix.cpp
    Core/Src/PerfCounter.cpp
    Core/Src/SavePackage.cpp
    Core/Src/SHMath.cpp
    Core/Src/StackTracker.cpp
    Core/Src/TrackAllocSections.cpp
    Core/Src/UExporter.cpp
    Core/Src/UFactory.cpp
    Core/Src/UHelpCommandlet.cpp
    Core/Src/UnAnsi.cpp
    Core/Src/UnArchive.cpp
    Core/Src/UnAsyncLoading.cpp
    Core/Src/UnAsyncLoadingWindows.cpp
    Core/Src/UnAsyncLoadingUnix.cpp
    Core/Src/UnAsyncWork.cpp
    Core/Src/UnBits.cpp
    Core/Src/UnBulkData.cpp
    Core/Src/UnClass.cpp
    Core/Src/UnCoreNative.cpp
    Core/Src/UnCoreNet.cpp
    Core/Src/UnCorSc.cpp
    Core/Src/UnGUID.cpp
    Core/Src/UnLinker.cpp
    Core/Src/UnMacCarbon.cpp
    Core/Src/UnMath.cpp
    Core/Src/UnMem.cpp
    Core/Src/UnMisc.cpp
    Core/Src/UnName.cpp
    Core/Src/UnObj.cpp
    Core/Src/UnObjectRedirector.cpp
    Core/Src/UnObjGC.cpp
    Core/Src/UnObjVer.cpp
    Core/Src/UnOutputDevices.cpp
    Core/Src/UnProp.cpp
    Core/Src/UnScriptPatcher.cpp
    Core/Src/UnStats.cpp
    Core/Src/UnStatsNotifyProviders.cpp
    Core/Src/UnStatsNotifyProvidersBase.cpp
    Core/Src/UnThreadingBase.cpp
    Core/Src/UnThreadingUnix.cpp
    Core/Src/UnThreadingWindows.cpp
    Core/Src/UnUnix.cpp
)
ADD_UNREAL_MODULE("ALL" "Core" "${CORE_SRCS}" "Core/Inc/CorePrivate.h")


SET(OPENGLDRV_SRCS
    OpenGLDrv/Src/OpenGLCommands.cpp
    OpenGLDrv/Src/OpenGLDevice.cpp
    OpenGLDrv/Src/OpenGLDrv.cpp
    OpenGLDrv/Src/OpenGLIndexBuffer.cpp
    OpenGLDrv/Src/OpenGLQuery.cpp
    OpenGLDrv/Src/OpenGLRenderTarget.cpp
    OpenGLDrv/Src/OpenGLShaderCompiler.cpp
    OpenGLDrv/Src/OpenGLShaders.cpp
    OpenGLDrv/Src/OpenGLState.cpp
    OpenGLDrv/Src/OpenGLTexture.cpp
    OpenGLDrv/Src/OpenGLUtil.cpp
    OpenGLDrv/Src/OpenGLVertexBuffer.cpp
    OpenGLDrv/Src/OpenGLVertexDeclaration.cpp
    OpenGLDrv/Src/OpenGLViewport.cpp
)
IF(NOT UNREAL_USE_NULL_RHI)
    IF(UNIX)
        ADD_UNREAL_MODULE("ALL" "OpenGLDrv" "${OPENGLDRV_SRCS}" "OpenGLDrv/Inc/OpenGLDrvPrivate.h")
        FILE(GLOB CG_LIBS
            "../External/Cg/lib/${UNREAL_TARGET}/*.*"
        )
    ENDIF(UNIX)
ENDIF(NOT UNREAL_USE_NULL_RHI)


SET(ENGINE_SRCS
    Engine/Src/AFileLog.cpp
    Engine/Src/AnimationCompressionAlgorithm.cpp
    Engine/Src/AnimationCompressionAlgorithm_BitwiseCompressOnly.cpp
    Engine/Src/AnimationCompressionAlgorithm_RemoveEverySecondKey.cpp
    Engine/Src/AnimationCompressionAlgorithm_RemoveTrivialKeys.cpp
    Engine/Src/AnimationCompressionAlgorithm_RevertToRaw.cpp
    Engine/Src/AnimationUtils.cpp
    Engine/Src/BranchingPCFShadowRendering.cpp
    Engine/Src/BSPOps.cpp
    Engine/Src/CanvasScene.cpp
    Engine/Src/ChartCreation.cpp
    Engine/Src/ConvexVolume.cpp
    Engine/Src/CoverGroupRenderingComponent.cpp
    Engine/Src/CoverMeshComponent.cpp
    Engine/Src/DebugRenderSceneProxy.cpp
    Engine/Src/DecalActor.cpp
    Engine/Src/DecalComponent.cpp
    Engine/Src/DecalManager.cpp
    Engine/Src/DecalRenderData.cpp
    Engine/Src/DemoRecording.cpp
    Engine/Src/DirectionalLightComponent.cpp
    Engine/Src/DistortionRendering.cpp
    Engine/Src/DOFAndBloomEffect.cpp
    Engine/Src/DownloadableContent.cpp
    Engine/Src/DrawFrustumComponent.cpp
    Engine/Src/DrawLightRadiusComponent.cpp
    Engine/Src/DynamicLightEnvironmentComponent.cpp
    Engine/Src/DynamicRHI.cpp
    Engine/Src/Engine.cpp
    Engine/Src/FogVolumeDensityComponents.cpp
    Engine/Src/FullScreenMovie.cpp
    Engine/Src/GPUSkinVertexFactory.cpp
    Engine/Src/HeightFogComponent.cpp
    Engine/Src/HitProxies.cpp
    Engine/Src/HModel.cpp
    Engine/Src/ImageUtils.cpp
    Engine/Src/LensFlare.cpp
    Engine/Src/LensFlareRendering.cpp
    Engine/Src/LensFlareVertexFactory.cpp
    Engine/Src/LevelUtils.cpp
    Engine/Src/LightComponent.cpp
    Engine/Src/LightEnvironmentComponent.cpp
    Engine/Src/LocalDecalVertexFactory.cpp
    Engine/Src/Localization.cpp
    Engine/Src/LocalizationExport.cpp
    Engine/Src/LocalVertexFactory.cpp
    Engine/Src/LocalVertexFactoryShaderParms.cpp
    Engine/Src/Material.cpp
    Engine/Src/MaterialExpressions.cpp
    Engine/Src/MaterialInstance.cpp
    Engine/Src/MaterialInstanceConstant.cpp
    Engine/Src/MaterialInstanceTimeVarying.cpp
    Engine/Src/MaterialShared.cpp
    Engine/Src/MeshUtils.cpp
    Engine/Src/NullRHI.cpp
    Engine/Src/PathRenderingComponent.cpp
    Engine/Src/PointLightComponent.cpp
    Engine/Src/PortalVolume.cpp
    Engine/Src/PreviewScene.cpp
    Engine/Src/PrimitiveComponent.cpp
    Engine/Src/RHI.cpp
    Engine/Src/RawIndexBuffer.cpp
    Engine/Src/RenderingThread.cpp
    Engine/Src/RenderResource.cpp
    Engine/Src/ReverbVolume.cpp
    Engine/Src/RouteRenderingComponent.cpp
    Engine/Src/ScopedObjectStateChange.cpp
    Engine/Src/ScreenRendering.cpp
    Engine/Src/ShaderCompiler.cpp
    Engine/Src/ShaderComplexityRendering.cpp
    Engine/Src/ShaderManager.cpp
    Engine/Src/SkyLightComponent.cpp
    Engine/Src/SpotLightComponent.cpp
    Engine/Src/SpriteComponent.cpp
    Engine/Src/SubtitleStorage.cpp
    Engine/Src/Surface.cpp
    Engine/Src/SystemSettings.cpp
    Engine/Src/TerrainVertexFactory.cpp
    Engine/Src/Texture.cpp
    Engine/Src/Texture2D.cpp
    Engine/Src/Texture2DComposite.cpp
    Engine/Src/TextureCube.cpp
    Engine/Src/TextureFlipbook.cpp
    Engine/Src/TextureMovie.cpp
    Engine/Src/TextureRenderTarget.cpp
    Engine/Src/TextureRenderTarget2D.cpp
    Engine/Src/TextureRenderTargetCube.cpp
    Engine/Src/TileRendering.cpp
    Engine/Src/UberPostProcessEffect.cpp
    Engine/Src/UnActor.cpp
    Engine/Src/UnActorComponent.cpp
    Engine/Src/UnActorFactory.cpp
    Engine/Src/UnAnimPlay.cpp
    Engine/Src/UnAnimTree.cpp
    Engine/Src/UnAnimTreeDraw.cpp
    Engine/Src/UnAudio.cpp
    Engine/Src/UnAudioEffect.cpp
    Engine/Src/UnAudioNodes.cpp
    Engine/Src/UnAudioNodesDraw.cpp
    Engine/Src/UnBrushComponent.cpp
    Engine/Src/UnCamera.cpp
    Engine/Src/UnCar.cpp
    Engine/Src/UnCDKey.cpp
    Engine/Src/UnClient.cpp
    Engine/Src/UnCodecs.cpp
    Engine/Src/UnCollision.cpp
    Engine/Src/UnConsoleSupportContainer.cpp
    Engine/Src/UnConsoleTools.cpp
    Engine/Src/UnContentStreaming.cpp
    Engine/Src/UnController.cpp
    Engine/Src/UnCurveEdPresetCurve.cpp
    Engine/Src/UnDistributions.cpp
    Engine/Src/UnDOFEffect.cpp
    Engine/Src/UnEngine.cpp
    Engine/Src/UnErrorChecking.cpp
    Engine/Src/UnFont.cpp
    Engine/Src/UnForceFeedbackWaveform.cpp
    Engine/Src/UnFPoly.cpp
    Engine/Src/UnGame.cpp
    Engine/Src/UnInterpCurveEdSetup.cpp
    Engine/Src/UnInterpolation.cpp
    Engine/Src/UnInterpolationCurveEd.cpp
    Engine/Src/UnInterpolationDraw.cpp
    Engine/Src/UnkDOP.cpp
    Engine/Src/UnLevAct.cpp
    Engine/Src/UnLevel.cpp
    Engine/Src/UnLevTic.cpp
    Engine/Src/UnLight.cpp
    Engine/Src/UnLightMap.cpp
    Engine/Src/UnLinkedObjDrawUtils.cpp
    Engine/Src/UnMaterialEffect.cpp
    Engine/Src/UnModel.cpp
    Engine/Src/UnModelCollision.cpp
    Engine/Src/UnModelComponent.cpp
    Engine/Src/UnModelLight.cpp
    Engine/Src/UnModelRender.cpp
    Engine/Src/UnMorph.cpp
    Engine/Src/UnMorphMesh.cpp
    Engine/Src/UnMorphTools.cpp
    Engine/Src/UnMotionBlurEffect.cpp
    Engine/Src/UnNavigationPoint.cpp
    Engine/Src/UnNovodexGeomUtils.cpp
    Engine/Src/UnOctree.cpp
    Engine/Src/UnOnlineStats.cpp
    Engine/Src/UnPath.cpp
    Engine/Src/UnPawn.cpp
    Engine/Src/UnPhysic.cpp
    Engine/Src/UnPlayer.cpp
    Engine/Src/UnPostProcess.cpp
    Engine/Src/UnPrefab.cpp
    Engine/Src/UnReach.cpp
    Engine/Src/UnRenderUtils.cpp
    Engine/Src/UnRoute.cpp
    Engine/Src/UnSceneCapture.cpp
    Engine/Src/UnScript.cpp
    Engine/Src/UnSelection.cpp
    Engine/Src/UnSequence.cpp
    Engine/Src/UnSequenceDraw.cpp
    Engine/Src/UnSequenceMusic.cpp
    Engine/Src/UnShadowMap.cpp
    Engine/Src/UnSkelControl.cpp
    Engine/Src/UnSkeletalAnim.cpp
    Engine/Src/UnSkeletalComponent.cpp
    Engine/Src/UnSkeletalMesh.cpp
    Engine/Src/UnSkeletalMeshCollision.cpp
    Engine/Src/UnSkeletalMeshMerge.cpp
    Engine/Src/UnSkeletalRender.cpp
    Engine/Src/UnSkeletalRenderCPUSkin.cpp
    Engine/Src/UnSkeletalRenderGPUSkin.cpp
    Engine/Src/UnSkeletalTools.cpp
    Engine/Src/UnStandardObjectPropagator.cpp
    Engine/Src/UnStatChart.cpp
    Engine/Src/UnStaticMesh.cpp
    Engine/Src/UnStaticMeshBuild.cpp
    Engine/Src/UnStaticMeshCollision.cpp
    Engine/Src/UnStaticMeshLight.cpp
    Engine/Src/UnStaticMeshRender.cpp
    Engine/Src/UnStatsRender.cpp
    Engine/Src/UnSubtitleManager.cpp
    Engine/Src/UnSVehicle.cpp
    Engine/Src/UnTank.cpp
    Engine/Src/UnTerrain.cpp
    Engine/Src/UnTerrainBVTree.cpp
    Engine/Src/UnTerrainRender.cpp
    Engine/Src/UnTexCompress.cpp
    Engine/Src/UnURL.cpp
    Engine/Src/UnVehicle.cpp
    Engine/Src/UnWorld.cpp
    Engine/Src/UOnlineProfileSettings.cpp
    Engine/Src/UOnlineSubsystem.cpp
    Engine/Src/USettings.cpp
    Engine/Src/VertexFactory.cpp
    Engine/Src/VoiceInterfaceCommon.cpp
    Engine/Src/VSMShadowRendering.cpp
    Engine/Src/UnNovodexLibs.cpp
    Engine/Src/UnNovodexSupport.cpp
    Engine/Src/UnPhysActor.cpp
    Engine/Src/UnPhysAnim.cpp
    Engine/Src/UnPhysAsset.cpp
    Engine/Src/UnPhysAssetTools.cpp
    Engine/Src/UnPhysCollision.cpp
    Engine/Src/UnPhysComponent.cpp
    Engine/Src/UnPhysConstraint.cpp
    Engine/Src/UnPhysDrawing.cpp
    Engine/Src/UnPhysLevel.cpp
    Engine/Src/UnPhysUtils.cpp
    Engine/Src/MaterialShader.cpp
    Engine/Src/MeshMaterialShader.cpp
    Engine/Src/UnCanvas.cpp
    Engine/Src/UnIn.cpp
    Engine/Src/UnInteraction.cpp
    Engine/Src/UnUIAnimations.cpp
    Engine/Src/UnUIComponents.cpp
    Engine/Src/UnUIContainers.cpp
    Engine/Src/UnUIDataStores.cpp
    Engine/Src/UnUIEventSystem.cpp
    Engine/Src/UnUILists.cpp
    Engine/Src/UnUIObjects.cpp
    Engine/Src/UnUIPrefab.cpp
    Engine/Src/UnUIStrings.cpp
    Engine/Src/UnUIStyles.cpp
    Engine/Src/UnUserInterface.cpp
    Engine/Src/BasePassRendering.cpp
    Engine/Src/BatchedElements.cpp
    Engine/Src/DecalRendering.cpp
    Engine/Src/DepthRendering.cpp
    Engine/Src/DynamicMeshBuilder.cpp
    Engine/Src/FogRendering.cpp
    Engine/Src/FogVolumeRendering.cpp
    Engine/Src/LightFunctionRendering.cpp
    Engine/Src/LightRendering.cpp
    Engine/Src/Scene.cpp
    Engine/Src/SceneCore.cpp
    Engine/Src/SceneFilterRendering.cpp
    Engine/Src/SceneHitProxyRendering.cpp
    Engine/Src/SceneOcclusion.cpp
    Engine/Src/ScenePostProcessing.cpp
    Engine/Src/SceneRendering.cpp
    Engine/Src/SceneRenderTargets.cpp
    Engine/Src/ShadowRendering.cpp
    Engine/Src/ShadowVolumeRendering.cpp
    Engine/Src/TextureDensityRendering.cpp
    Engine/Src/TranslucentRendering.cpp
    Engine/Src/VelocityRendering.cpp
    Engine/Src/ParticleBeam2EmitterInstance.cpp
    Engine/Src/ParticleBeamTrailVertexFactory.cpp
    Engine/Src/ParticleDataManager.cpp
    Engine/Src/ParticleEmitterInstances.cpp
    Engine/Src/ParticleInstancedMeshVertexFactory.cpp
    Engine/Src/ParticleModules_Collision.cpp
    Engine/Src/ParticleModules_Material.cpp
    Engine/Src/ParticleModules_NxFluids.cpp
    Engine/Src/ParticleModules_Orbit.cpp
    Engine/Src/ParticleNxFluidEmitterInstance.cpp
    Engine/Src/ParticleSubUVVertexFactory.cpp
    Engine/Src/ParticleTrail2EmitterInstance.cpp
    Engine/Src/ParticleVertexFactory.cpp
    Engine/Src/UnParticleBeamModules.cpp
    Engine/Src/UnParticleComponents.cpp
    Engine/Src/UnParticleModules.cpp
    Engine/Src/UnParticleSystemRender.cpp
    Engine/Src/UnParticleTrailModules.cpp
    Engine/Src/SpeedTree.cpp
    Engine/Src/SpeedTreeActorFactory.cpp
    Engine/Src/SpeedTreeComponent.cpp
    Engine/Src/SpeedTreeComponentFactory.cpp
    Engine/Src/SpeedTreeStaticLighting.cpp
    Engine/Src/SpeedTreeVertexFactory.cpp
    Engine/Src/FoliageComponent.cpp
    Engine/Src/FoliageFactory.cpp
    Engine/Src/FoliageRendering.cpp
    Engine/Src/FoliageVertexFactory.cpp
    Engine/Src/UnTerrainFoliage.cpp
    Engine/Src/UnBunch.cpp
    Engine/Src/UnChan.cpp
    Engine/Src/UnConn.cpp
    Engine/Src/UnDownload.cpp
    Engine/Src/UnNetDrv.cpp
    Engine/Src/UnPenLev.cpp
    Engine/Src/UnFaceFXAnimSet.cpp
    Engine/Src/UnFaceFXAsset.cpp
    Engine/Src/UnFaceFXMaterialParameterProxy.cpp
    Engine/Src/UnFaceFXMorphTargetProxy.cpp
    Engine/Src/UnFaceFXSupport.cpp
    Engine/Src/Fonix.cpp
)
ADD_UNREAL_MODULE("ALL" "Engine" "${ENGINE_SRCS}" "Engine/Inc/EnginePrivate.h")


SET(SDLDRV_SRCS
    SDLDrv/Src/SDLDrv.cpp
    SDLDrv/Src/SDLClient.cpp
    SDLDrv/Src/SDLViewport.cpp
)
IF(NOT UNREAL_USE_NULL_RHI)
    IF(UNIX)
        ADD_UNREAL_MODULE("ALL" "SDLDrv" "${SDLDRV_SRCS}" "SDLDrv/Inc/SDLDrv.h")
    ENDIF(UNIX)
ENDIF(NOT UNREAL_USE_NULL_RHI)


SET(IPDRV_SRCS
    IpDrv/Src/FDebugServer.cpp
    IpDrv/Src/FMultiThreadedRingBuffer.cpp
    IpDrv/Src/FRemotePropagator.cpp
    IpDrv/Src/FRemotePropagatorNet.cpp
    IpDrv/Src/HTTPDownload.cpp
    IpDrv/Src/InternetLink.cpp
    IpDrv/Src/IpDrv.cpp
    IpDrv/Src/OnlineGameInterfaceImpl.cpp
    IpDrv/Src/OnlineSubsystemCommonImpl.cpp
    IpDrv/Src/TcpNetDriver.cpp
    IpDrv/Src/UnSocket.cpp
    IpDrv/Src/UnSocketBSD.cpp
    IpDrv/Src/UnStatsNotifyProviders_UDP.cpp
)
ADD_UNREAL_MODULE("ALL" "IpDrv" "${IPDRV_SRCS}" "IpDrv/Inc/UnIpDrv.h")


SET(ALAUDIO_SRCS
    ALAudio/Src/ALAudio.cpp
    ALAudio/Src/ALAudioDecompress.cpp
    ALAudio/Src/ALAudioDevice.cpp
    ALAudio/Src/ALAudioEffects.cpp
)
ADD_UNREAL_MODULE("ALL" "ALAudio" "${ALAUDIO_SRCS}" "ALAudio/Src/ALAudioPrivate.h")


SET(UNREALSCRIPTTEST_SRCS
    UnrealScriptTest/Src/ComponentTest.cpp
    UnrealScriptTest/Src/UnrealScriptTest.cpp
)
ADD_UNREAL_MODULE("ALL" "UnrealScriptTest" "${UNREALSCRIPTTEST_SRCS}" "UnrealScriptTest/Inc/UnrealScriptTest.h")


SET(EDITOR_SRCS
    Editor/Src/AnalyzeCookedContentCommandlet.cpp
    Editor/Src/DiffPackagesCommandlet.cpp
    Editor/Src/DragTool_BoxSelect.cpp
    Editor/Src/DragTool_Measure.cpp
    Editor/Src/Editor.cpp
    Editor/Src/FEdObjectPropagator.cpp
    Editor/Src/GroupUtils.cpp
    Editor/Src/LocalizationCommandlets.cpp
    Editor/Src/MouseDeltaTracker.cpp
    Editor/Src/PlayLevel.cpp
    Editor/Src/ScopedTransaction.cpp
    Editor/Src/StaticLightingAggregateMesh.cpp
    Editor/Src/StaticLightingTextureMapping.cpp
    Editor/Src/StaticLightingVertexMapping.cpp
    Editor/Src/TTFontImport.cpp
    Editor/Src/UAnalyzeScriptCommandlet.cpp
    Editor/Src/UBatchExportCommandlet.cpp
    Editor/Src/UBrushBuilder.cpp
    Editor/Src/UContentCommandlets.cpp
    Editor/Src/UFixAmbiguousMaterialParameters.cpp
    Editor/Src/UMakeCommandlet.cpp
    Editor/Src/UnAudioCompress.cpp
    Editor/Src/UnBsp.cpp
    Editor/Src/UnColladaExporter.cpp
    Editor/Src/UnColladaImporter.cpp
    Editor/Src/UnColladaSceneGraph.cpp
    Editor/Src/UnColladaSkeletalMesh.cpp
    Editor/Src/UnColladaStaticMesh.cpp
    Editor/Src/UnContentCookers.cpp
    Editor/Src/UnEdCnst.cpp
    Editor/Src/UnEdComponents.cpp
    Editor/Src/UnEdCsg.cpp
    Editor/Src/UnEdDragTools.cpp
    Editor/Src/UnEdExp.cpp
    Editor/Src/UnEdExpTerrain.cpp
    Editor/Src/UnEdFact.cpp
    Editor/Src/UnEdFactMorph.cpp
    Editor/Src/UnEdFactTerrain.cpp
    Editor/Src/UnEditor.cpp
    Editor/Src/UnEdModes.cpp
    Editor/Src/UnEdModeTools.cpp
    Editor/Src/UnEdObject.cpp
    Editor/Src/UnEdSrv.cpp
    Editor/Src/UnEdTran.cpp
    Editor/Src/UnEdViewport.cpp
    Editor/Src/UnFixupRedirectsCommandlet.cpp
    Editor/Src/UnGeom.cpp
    Editor/Src/UnGeomModifiers.cpp
    Editor/Src/UnMeshEd.cpp
    Editor/Src/UnPackageUtilities.cpp
    Editor/Src/UnParams.cpp
    Editor/Src/UnPrecompileShadersCommandlet.cpp
    Editor/Src/UnScrCom.cpp
    Editor/Src/UnScrPrecom.cpp
    Editor/Src/UnShadow.cpp
    Editor/Src/UnSkelMeshEdit.cpp
    Editor/Src/UnStaticMeshEdit.cpp
    Editor/Src/UnTerrainEdit.cpp
    Editor/Src/UnVisi.cpp
    Editor/Src/UnWidget.cpp
    Editor/Src/USetMaterialUsageCommandlet.cpp
    Editor/Src/UStripSourceCommandlet.cpp
)
# !!! FIXME: Change "NONE" to "ALL"
ADD_UNREAL_MODULE("NONE" "Editor" "${EDITOR_SRCS}" "Editor/Src/EditorPrivate.h")


SET(GAMEFRAMEWORK_SRCS
    GameFramework/Src/GameBreakableActor.cpp
    GameFramework/Src/GameController.cpp
    GameFramework/Src/GameFramework.cpp
    GameFramework/Src/GameHUD.cpp
    GameFramework/Src/GamePawn.cpp
    GameFramework/Src/GamePlayerInput.cpp
    GameFramework/Src/GameProjectile.cpp
    GameFramework/Src/GameSkelControls.cpp
    GameFramework/Src/GameVehicle.cpp
    GameFramework/Src/GameWeapon.cpp
)
ADD_UNREAL_MODULE("ALL" "GameFramework" "${GAMEFRAMEWORK_SRCS}" "GameFramework/Inc/GameFramework.h")


SET(ONLINESUBSYSTEMPC_SRCS
    OnlineSubsystemPC/Src/OnlineSubsystemPC.cpp
    OnlineSubsystemPC/Src/OnlineSubsystemPCPackage.cpp
    OnlineSubsystemPC/Src/VoiceInterfacePC.cpp
)
ADD_UNREAL_MODULE("EXAMPLEGAME" "OnlineSubsystemPC" "${ONLINESUBSYSTEMPC_SRCS}" "OnlineSubsystemPC/Inc/OnlineSubsystemPC.h")
#ADD_UNREAL_MODULE("WARGAME" "OnlineSubsystemPC" "${ONLINESUBSYSTEMPC_SRCS}" "OnlineSubsystemPC/Inc/OnlineSubsystemPC.h")


SET(GAMESPY_SRCS
    ../External/GameSpy/serverbrowsing/sb_crypt.c
    ../External/GameSpy/serverbrowsing/sb_queryengine.c
    ../External/GameSpy/serverbrowsing/sb_serverbrowsing.c
    ../External/GameSpy/serverbrowsing/sb_server.c
    ../External/GameSpy/serverbrowsing/sb_serverlist.c
    ../External/GameSpy/webservices/AuthService.c
    ../External/GameSpy/common/gsAssert.c
    ../External/GameSpy/common/gsAvailable.c
    ../External/GameSpy/common/gsCore.c
    ../External/GameSpy/common/gsCrypt.c
    ../External/GameSpy/common/gsDebug.c
    ../External/GameSpy/common/gsLargeInt.c
    ../External/GameSpy/common/gsMemory.c
    ../External/GameSpy/common/gsPlatform.c
    ../External/GameSpy/common/gsPlatformSocket.c
    ../External/GameSpy/common/gsPlatformThread.c
    ../External/GameSpy/common/gsPlatformUtil.c
    ../External/GameSpy/common/gsRC4.c
    ../External/GameSpy/common/gsSHA1.c
    ../External/GameSpy/common/gsSoap.c
    ../External/GameSpy/common/gsSSL.c
    ../External/GameSpy/common/gsStringUtil.c
    ../External/GameSpy/common/gsUdpEngine.c
    ../External/GameSpy/common/gsXML.c
    ../External/GameSpy/sake/sakeMain.c
    ../External/GameSpy/sake/sakeRequest.c
    ../External/GameSpy/sake/sakeRequestMisc.c
    ../External/GameSpy/sake/sakeRequestModify.c
    ../External/GameSpy/sake/sakeRequestRead.c
    ../External/GameSpy/ghttp/ghttpBuffer.c
    ../External/GameSpy/ghttp/ghttpCallbacks.c
    ../External/GameSpy/ghttp/ghttpCommon.c
    ../External/GameSpy/ghttp/ghttpConnection.c
    ../External/GameSpy/ghttp/ghttpEncryption.c
    ../External/GameSpy/ghttp/ghttpMain.c
    ../External/GameSpy/ghttp/ghttpPost.c
    ../External/GameSpy/ghttp/ghttpProcess.c
    ../External/GameSpy/qr2/qr2.c
    ../External/GameSpy/qr2/qr2regkeys.c
    ../External/GameSpy/gp/gp.c
    ../External/GameSpy/gp/gpiBuddy.c
    ../External/GameSpy/gp/gpiBuffer.c
    ../External/GameSpy/gp/gpi.c
    ../External/GameSpy/gp/gpiCallback.c
    ../External/GameSpy/gp/gpiConnect.c
    ../External/GameSpy/gp/gpiInfo.c
    ../External/GameSpy/gp/gpiOperation.c
    ../External/GameSpy/gp/gpiPeer.c
    ../External/GameSpy/gp/gpiProfile.c
    ../External/GameSpy/gp/gpiSearch.c
    ../External/GameSpy/gp/gpiTransfer.c
    ../External/GameSpy/gp/gpiUnique.c
    ../External/GameSpy/gp/gpiUtility.c
    ../External/GameSpy/sc/sciInterface.c
    ../External/GameSpy/sc/sciMain.c
    ../External/GameSpy/sc/sciReport.c
    ../External/GameSpy/sc/sciSerialize.c
    ../External/GameSpy/sc/sciWebServices.c
    ../External/GameSpy/darray.c
    ../External/GameSpy/hashtable.c
    ../External/GameSpy/md5c.c
    ../External/GameSpy/nonport.c
)
ADD_UNREAL_MODULE("UTGAME" "GameSpy" "${GAMESPY_SRCS}" "")


SET(ONLINESUBSYSTEMGAMESPY_SRCS
    OnlineSubsystemGameSpy/Src/OnlineSubsystemGameSpy.cpp
    OnlineSubsystemGameSpy/Src/OnlineSubsystemGameSpyPackage.cpp
    OnlineSubsystemGameSpy/Src/UOnlineGameInterfaceGameSpy.cpp
    OnlineSubsystemGameSpy/Src/VoiceInterfaceGameSpy.cpp
)
ADD_UNREAL_MODULE("UTGAME" "OnlineSubsystemGameSpy" "${ONLINESUBSYSTEMGAMESPY_SRCS}" "OnlineSubsystemGameSpy/Inc/OnlineSubsystemGameSpy.h")


SET(WARFAREGAME_SRCS
    WarfareGame/Src/DelayedUnpauser.cpp
    WarfareGame/Src/UnUI.cpp
    WarfareGame/Src/UnVince.cpp
    WarfareGame/Src/UnWarHud.cpp
    WarfareGame/Src/UnWarInput.cpp
    WarfareGame/Src/UnWarPath.cpp
    WarfareGame/Src/UWarfareAnimNodes.cpp
    WarfareGame/Src/WarAI.cpp
    WarfareGame/Src/WarCamera.cpp
    WarfareGame/Src/WarCheckpoint.cpp
    WarfareGame/Src/WarCheckpointIO.cpp
    WarfareGame/Src/WarCheckpointManager.cpp
    WarfareGame/Src/WarDifficultyScaling.cpp
    WarfareGame/Src/WarfareAutomatedTesting.cpp
    WarfareGame/Src/WarfareGame.cpp
    WarfareGame/Src/WarfareGameSequence.cpp
    WarfareGame/Src/WarfareSpecialMoves.cpp
    WarfareGame/Src/WarNet.cpp
    WarfareGame/Src/WarSound.cpp
    WarfareGame/Src/WarVehicle.cpp
)
ADD_UNREAL_MODULE("WARGAME" "WarfareGame" "${WARFAREGAME_SRCS}" "WarfareGame/Inc/WarfareGame.h")


SET(EXAMPLEGAME_SRCS
    ExampleGame/Src/ExampleGame.cpp
    ExampleGame/Src/SampleClass.cpp
)
ADD_UNREAL_MODULE("EXAMPLEGAME" "ExampleGame" "${EXAMPLEGAME_SRCS}" "ExampleGame/Inc/ExampleGame.h")


SET(UTGAME_SRCS
    UTGame/Src/AUTBot.cpp
    UTGame/Src/DemoGame.cpp
    UTGame/Src/UTCustomChar.cpp
    UTGame/Src/UTFlock.cpp
    UTGame/Src/UTGame.cpp
    UTGame/Src/UTGib.cpp
    UTGame/Src/UTHellBender.cpp
    UTGame/Src/UTHoverBoard.cpp
    UTGame/Src/UTLeaderboardWriteBase.cpp
    UTGame/Src/UTLeviathan.cpp
    UTGame/Src/UTNecrisRaptor.cpp
    UTGame/Src/UTNet.cpp
    UTGame/Src/UTNightShade.cpp
    UTGame/Src/UTOnslaught.cpp
    UTGame/Src/UTPathing.cpp
    UTGame/Src/UTPawn.cpp
    UTGame/Src/UTPlayerController.cpp
    UTGame/Src/UTScorpion.cpp
    UTGame/Src/UTSkelControls.cpp
    UTGame/Src/UTSPMA.cpp
    UTGame/Src/UTTurret.cpp
    UTGame/Src/UTUI_Base.cpp
    UTGame/Src/UTUI_BaseWidgets.cpp
    UTGame/Src/UTUI_DataStores.cpp
    UTGame/Src/UTUI_FrontEnd.cpp
    UTGame/Src/UTUI_Hud.cpp
    UTGame/Src/UTUI_SinglePlayer.cpp
    UTGame/Src/UTUI_VehicleHuds.cpp
    UTGame/Src/UTVehicle.cpp
    UTGame/Src/UTWalker.cpp
    UTGame/Src/UTWeapon.cpp
    UTGame/Src/UUTAnimNodes.cpp
)
ADD_UNREAL_MODULE("UTGAME" "UTGame" "${UTGAME_SRCS}" "UTGame/Inc/UTGame.h")


SET(DOESNT_COMPILE_YET
    Engine/Src/UnPNG.cpp
    Engine/Debugger/UnDebuggerCore.cpp
    Engine/Debugger/UnDelphiInterface.cpp
    Engine/Debugger/UnWTInterface.cpp
    WarfareGame/Src/WordwrapUtil.cpp
)

SET(LAUNCH_SRCS
    Launch/Src/Launch.cpp
    Launch/Src/LaunchEngineLoop.cpp
)

ADD_EXECUTABLE(${UNREAL_EXENAME} ${LAUNCH_SRCS})

OPTION(UNREAL_DEBUG_SYMS_Launch "Specifically build 'Launch' with debug symbols" ${UNREAL_BUILD_WITH_DEBUG_SYMBOLS})
IF(UNREAL_DEBUG_SYMS_Launch)
    SET_TARGET_PROPERTIES(${UNREAL_EXENAME} PROPERTIES COMPILE_FLAGS ${DEBUG_SYMBOLS_FLAG})
    REPORT_MODULE_DEBUG_SYMS("Launch")
ENDIF(UNREAL_DEBUG_SYMS_Launch)


TARGET_LINK_LIBRARIES(${UNREAL_EXENAME}
    ${UNREAL_MODULES}
    ${UNREAL_MODULES}
    ${OPTIONAL_LIBS}
    ${NOVODEX_LIBS}
    ${SPEEDTREERT_LIBS}
    ${FACEFX_LIBS}
    ${SDL_LIBS}
    ${CG_LIBS}
)
#${WXWIDGETS_LIBS}

# End of CMakeLists.txt ...

