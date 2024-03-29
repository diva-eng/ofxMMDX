# declaration of function

set(VPVL2_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
function(get_source_directory output dir)
  set(${output} "${VPVL2_BASE_DIR}/${dir}" PARENT_SCOPE)
endfunction()

function(get_build_directory output source_dir)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  set(BUILD_DIRECTORY "build-${CMAKE_BUILD_TYPE_TOLOWER}")
  set(${output} "${${source_dir}}/${BUILD_DIRECTORY}" PARENT_SCOPE)
endfunction()

function(get_local_library_directory_named output source_dir lib_dir)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  get_build_directory(build_dir ${source_dir})
  if(MSVC)
    set(${output} "${build_dir}/${lib_dir}/${CMAKE_BUILD_TYPE_TOLOWER}" PARENT_SCOPE)
  else()
    set(${output} "${build_dir}/${lib_dir}" PARENT_SCOPE)
  endif()
endfunction()

function(get_local_library_directory output source_dir)
  get_local_library_directory_named(output_to_reassign ${source_dir} "lib")
  set(${output} ${output_to_reassign} PARENT_SCOPE)
endfunction()

function(get_install_directory output dir)
  get_source_directory(source_dir ${dir})
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  set(build_dir "build-${CMAKE_BUILD_TYPE_TOLOWER}")
  set(${output} "${source_dir}/${build_dir}/install-root" PARENT_SCOPE)
endfunction()

function(vpvl2_set_library_properties target public_headers)
  # create as a framework if build on darwin environment
  if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES PREFIX "" SUFFIX .dll IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
  elseif(APPLE)
    if(BUILD_SHARED_LIBS AND FRAMEWORK)
      install(TARGETS vpvl2 DESTINATION .)
      set_target_properties(${target} PROPERTIES FRAMEWORK true PROPERTIES PUBLIC_HEADER "${public_headers}")
    endif()
    set_target_properties(vpvl2 PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endfunction()

function(vpvl2_set_warnings)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUC)
  # set more warnings when clang or gcc is selected
    add_definitions(-W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings)
  elseif(MSVC)
    # disable some specified warnings on MSVC
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHsc /wd4068 /wd4819" CACHE STRING "disable warnings of C4068 (for clang pragma) and C4819" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /wd4068 /wd4819" CACHE STRING "disable warnings of C4068 (for clang pragma) and C4819" FORCE)
    # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl2/Common.h
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
  endif()
endfunction()

function(vpvl2_link_bullet target)
  target_link_libraries(${target} ${BULLET_DYNAMICS_LIB}
                                  ${BULLET_COLLISION_LIB}
                                  ${BULLET_SOFTBODY_LIB}
                                  ${BULLET_MULTITHREADED_LIB}
                                  ${BULLET_LINEARMATH_LIB})
endfunction()

function(vpvl2_find_bullet)
  get_install_directory(BULLET_INSTALL_DIR "bullet-src")
  find_path(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h PATH_SUFFIXES include/bullet PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(BULLET_LINEARMATH_LIB LinearMath PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(BULLET_COLLISION_LIB BulletCollision PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(BULLET_DYNAMICS_LIB BulletDynamics PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(BULLET_MULTITHREADED_LIB BulletMultiThreaded PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
  include_directories(${BULLET_INCLUDE_DIR})
endfunction()

function(vpvl2_link_assimp target)
  if(VPVL2_LINK_ASSIMP)
    target_link_libraries(${target} ${ASSIMP_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_assimp)
  if(VPVL2_LINK_ASSIMP)
    get_install_directory(ASSIMP_INSTALL_DIR "assimp-src")
    find_path(ASSIMP_INCLUDE_DIR NAMES assimp/Importer.hpp assimp/assimp.h PATH_SUFFIXES include PATHS ${ASSIMP_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(ASSIMP_LIBRARY assimp assimpD PATH_SUFFIXES lib64 lib32 lib PATHS ${ASSIMP_INSTALL_DIR} NO_DEFAULT_PATH)
    include_directories(${ASSIMP_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_vpvl target)
  if(VPVL2_LINK_VPVL)
    target_link_libraries(${target} ${VPVL_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_vpvl)
  if(VPVL2_LINK_VPVL)
    get_install_directory(VPVL_INSTALL_DIR "libvpvl")
    find_path(VPVL_INCLUDE_DIR NAMES vpvl/vpvl.h PATH_SUFFIXES include PATHS ${VPVL_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(VPVL_LIBRARY vpvl PATH_SUFFIXES lib64 lib32 lib PATHS ${VPVL_INSTALL_DIR} NO_DEFAULT_PATH)
    include_directories(${VPVL_INCLUDE_DIR} ${VPVL_CONFIG_DIR})
  endif()
endfunction()

function(vpvl2_link_glew target)
  if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
    target_link_libraries(${target} ${GLEW_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_glew)
  if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT AND VPVL2_LINK_GLEW AND NOT VPVL2_ENABLE_GLES2)
    if(MSVC)
      get_source_directory(GLEW_INSTALL_DIR "glew-src")
    else()
      get_install_directory(GLEW_INSTALL_DIR "glew-src")
    endif()
    if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
      # workaround of GLEW issue (cannot find function pointers correctly)
      find_library(GLEW_LIBRARY GLEW glew glew32s PATH_SUFFIXES lib64 lib32 lib)
      find_path(GLEW_INCLUDE_DIR GL/glew.h PATH_SUFFIXES include)
    else()
      find_library(GLEW_LIBRARY GLEW glew glew32s PATH_SUFFIXES lib64 lib32 lib PATHS ${GLEW_INSTALL_DIR} NO_DEFAULT_PATH)
      find_path(GLEW_INCLUDE_DIR GL/glew.h PATH_SUFFIXES include PATHS ${GLEW_INSTALL_DIR} NO_DEFAULT_PATH)
    endif()
    include_directories(${GLEW_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_nvtt target)
  if(VPVL2_LINK_NVTT)
    target_link_libraries(${target} ${NVTT_NVCORE_LIBRARY} ${NVTT_NVIMAGE_LIBRARY} ${NVTT_NVMATH_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_nvtt)
  if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT AND VPVL2_LINK_NVTT)
    get_source_directory(NVTT_SOURCE_DIRECTORY "nvtt-src")
    get_build_directory(NVTT_BUILD_DIRECTORY NVTT_SOURCE_DIRECTORY)
    set(NVTT_INCLUDE_DIRECTORY "${NVTT_SOURCE_DIRECTORY}/src")
    # for MSVC
    find_path(NVTT_CONFIG_INCLUDE_DIR nvconfig.h PATHS "${NVTT_BUILD_DIRECTORY}/src" NO_DEFAULT_PATH)
    find_path(NVTT_BASE_INCLUDE_DIR nvcore/nvcore.h PATHS ${NVTT_INCLUDE_DIRECTORY} NO_DEFAULT_PATH)
    find_path(NVTT_POSH_INCLUDE_DIR posh.h PATHS "${NVTT_SOURCE_DIRECTORY}/extern/poshlib" NO_DEFAULT_PATH)
    if (NVTT_CONFIG_INCLUDE_DIR AND NVTT_BASE_INCLUDE_DIR AND NVTT_POSH_INCLUDE_DIR)
      include_directories(${NVTT_CONFIG_INCLUDE_DIR} ${NVTT_BASE_INCLUDE_DIR} ${NVTT_POSH_INCLUDE_DIR})
    else()
      message(FATAL_ERROR "Required nvtt is not found.")
    endif()
    get_install_directory(NVTT_INSTALL_DIR "nvtt-src")
    find_library(NVTT_NVCORE_LIBRARY nvcore PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(NVTT_NVMATH_LIBRARY nvimage PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(NVTT_NVIMAGE_LIBRARY nvimage PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)
  endif()
endfunction()

function(vpvl2_find_sfml)
  if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT AND VPVL2_LINK_SFML)
    find_library(SFML_GRAPHICS_LIBRARY sfml-graphics)
    find_library(SFML_SYSTEM_LIBRARY sfml-system)
    find_library(SFML_WINDOW_LIBRARY sfml-window)
    find_path(SFML_INCLUDE_DIR SFML/System.h)
    include_directories(${SFML_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_find_openmp)
  if(VPVL2_ENABLE_OPENMP)
    find_package(OpenMP)
    if(OPENMP_FOUND)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
  endif()
endfunction()

function(vpvl2_link_icu target)
  if(ICU_LIBRARY_DATA AND ICU_LIBRARY_I18N AND ICU_LIBRARY_UC)
    target_link_libraries(${target} ${ICU_LIBRARY_I18N} ${ICU_LIBRARY_UC} ${ICU_LIBRARY_DATA})
  endif()
endfunction()

function(vpvl2_find_icu)
  if(VPVL2_ENABLE_EXTENSIONS_STRING)
    if(MSVC)
      get_source_directory(ICU_INSTALL_DIR "icu4c-src")
    else()
      get_install_directory(ICU_INSTALL_DIR "icu4c-src")
    endif()
    find_path(ICU_INCLUDE_DIR unicode/unistr.h PATH_SUFFIXES include PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(ICU_LIBRARY_DATA icudata icudt PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(ICU_LIBRARY_I18N icui18n icuin PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(ICU_LIBRARY_UC icuuc PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
    include_directories(${ICU_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_tbb target)
  if(TBB_LIBRARY)
    target_link_libraries(${target} ${TBB_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_tbb)
  if(VPVL2_LINK_INTEL_TBB)
    get_source_directory(TBB_SOURCE_DIRECTORY "tbb-src")
    find_library(TBB_LIBRARY tbb PATH_SUFFIXES lib PATHS ${TBB_SOURCE_DIRECTORY} NO_DEFAULT_PATH)
    find_path(TBB_INCLUDE_DIR tbb/tbb.h PATH_SUFFIXES include PATHS ${TBB_SOURCE_DIRECTORY} NO_DEFAULT_PATH)
    include_directories(${TBB_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_zlib target)
  if(ZLIB_LIBRARY)
    target_link_libraries(${target} ${ZLIB_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_zlib)
  if(NOT APPLE)
    get_install_directory(ZLIB_INSTALL_DIR "zlib-src")
    find_path(ZLIB_INCLUDE_DIR zlib.h PATH_SUFFIXES include PATHS ${ZLIB_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(ZLIB_LIBRARY z zlibstaticd zlibstatic PATH_SUFFIXES lib64 lib32 lib PATHS ${ZLIB_INSTALL_DIR} NO_DEFAULT_PATH)
  else()
    get_source_directory(ZLIB_SOURCE_DIRECTORY "zlib-src")
    get_build_directory(ZLIB_BUILD_DIRECTORY ZLIB_SOURCE_DIRECTORY)
    get_local_library_directory(ZLIB_LIBRARY_LOCAL_DIR ZLIB_SOURCE_DIRECTORY)
    find_library(ZLIB_LIBRARY z PATHS ${ZLIB_LIBRARY_LOCAL_DIR} NO_DEFAULT_PATH)
    find_path(ZLIB_INCLUDE_DIR zlib.h PATHS ${ZLIB_SOURCE_DIRECTORY} NO_DEFAULT_PATH)
    find_path(ZLIB_INCLUDE_CONFIG_DIR zconf.h PATHS ${ZLIB_BUILD_DIRECTORY} NO_DEFAULT_PATH)
  endif()
  include_directories(${ZLIB_INCLUDE_DIR} ${ZLIB_INCLUDE_CONFIG_DIR})
endfunction()

function(vpvl2_link_libxml2 target)
  if(LIBXML2_LIBRARY)
    target_link_libraries(${target} ${LIBXML2_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_libxml2)
  if(VPVL2_ENABLE_EXTENSIONS_PROJECT)
    get_install_directory(LIBXML2_INSTALL_DIR "libxml2-src")
    find_path(LIBXML2_INCLUDE_DIR NAMES libxml/xmlwriter.h PATH_SUFFIXES include/libxml2 PATHS ${LIBXML2_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(LIBXML2_LIBRARY xml2 libxml2_a libxml2 PATH_SUFFIXES lib64 lib32 lib PATHS ${LIBXML2_INSTALL_DIR} NO_DEFAULT_PATH)
    include_directories(${LIBXML2_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_alsoft target)
  if(ALSOFT_LIBRARY)
    target_link_libraries(${target} ${ALSOFT_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_alsoft)
  get_install_directory(ALSOFT_INSTALL_DIR "openal-soft-src")
  if(WIN32)
    find_path(ALSOFT_INCLUDE_DIR NAMES al.h PATH_SUFFIXES include/AL PATHS ${ALSOFT_INSTALL_DIR} NO_DEFAULT_PATH)
  else()
    find_path(ALSOFT_INCLUDE_DIR NAMES OpenAL/al.h AL/al.h PATH_SUFFIXES include PATHS ${ALSOFT_INSTALL_DIR} NO_DEFAULT_PATH)
  endif()
  find_library(ALSOFT_LIBRARY OpenAL32 openal PATH_SUFFIXES lib64 lib32 lib PATHS ${ALSOFT_INSTALL_DIR} NO_DEFAULT_PATH)
  include_directories(${ALSOFT_INCLUDE_DIR})
endfunction()

function(vpvl2_link_alure target)
  if(ALURE_LIBRARY)
    target_link_libraries(${target} ${ALURE_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_alure)
  get_install_directory(ALURE_INSTALL_DIR "alure-src")
  find_path(ALURE_INCLUDE_DIR NAMES OpenAL/alure.h AL/alure.h PATH_SUFFIXES include PATHS ${ALURE_INSTALL_DIR} NO_DEFAULT_PATH)
  find_library(ALURE_LIBRARY ALURE32-static alure-static ALURE32 alure PATH_SUFFIXES lib64 lib32 lib PATHS ${ALURE_INSTALL_DIR} NO_DEFAULT_PATH)
  include_directories(${ALURE_INCLUDE_DIR})
endfunction()

function(vpvl2_link_gl_runtime target)
  target_link_libraries(${target} ${OPENGL_gl_LIBRARY})
endfunction()

function(vpvl2_find_gl_runtime)
  if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
    if(VPVL2_ENABLE_OSMESA)
      get_source_directory(MESA3D_SOURCE_DIR "mesa-src")
      find_library(MESA3D_MESA_LIBRARY mesa PATH_SUFFIXES "embed-darwin-x86_64/mesa" "darwin-x86_64/mesa" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
      find_library(MESA3D_OSMESA_LIBRARY osmesa PATH_SUFFIXES "embed-darwin-x86_64/mesa/drivers/osmesa" "darwin-x86_64/mesa/drivers/osmesa" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
      set(OPENGL_gl_LIBRARY "${OPENGL_gl_mesa_LIBRARY} ${OPENGL_gl_osmesa_LIBRARY}")
      find_path(OPENGL_INCLUDE_DIR GL/osmesa.h PATH_SUFFIXES include PATHS ${MESA3D_SOURCE_DIR} NO_DEFAULT_PATH)
    elseif(VPVL2_ENABLE_GLES2)
      find_path(OPENGL_INCLUDE_DIR gl2.h PATH_SUFFIXES include/OpenGLES2 include/GLES2 include)
      if(NOT VPVL2_PLATFORM_EMSCRIPTEN)
        find_library(OPENGL_gl_LIBRARY NAMES ppapi_gles2 GLESv2)
      endif()
    else()
      find_package(OpenGL REQUIRED)
    endif()
    include_directories(${OPENGL_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_cg_runtime target)
  if(CG_LIBRARY AND CG_GL_LIBRARY)
    target_link_libraries(${target} ${CG_LIBRARY} ${CG_GL_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_cg_runtime)
  if(VPVL2_ENABLE_NVIDIA_CG AND VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
    find_package(Cg REQUIRED)
    include_directories(${CG_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_cl_runtime target)
  if(OPENCL_LIBRARY)
    target_link_libraries(${target} ${OPENCL_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_cl_runtime)
  if(VPVL2_ENABLE_OPENCL)
    find_library(OPENCL_LIBRARY OpenCL)
    find_path(OPENCL_INCLUDE_DIR cl.h PATH_SUFFIXES include/OpenCL include/CL include)
    include_directories(${OPENCL_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_find_gli)
  get_source_directory(GLI_SOURCE_DIRECTORY "gli-src")
  find_path(GLI_INCLUDE_DIR "gli/gli.hpp" PATHS ${GLI_SOURCE_DIRECTORY})
  include_directories(${GLI_INCLUDE_DIR})
endfunction()

function(vpvl2_find_glm)
  get_source_directory(GLM_SOURCE_DIRECTORY "glm-src")
  find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${GLM_SOURCE_DIRECTORY})
  include_directories(${GLM_INCLUDE_DIR})
endfunction()

function(vpvl2_link_glog target)
  if(GLOG_LIBRARY)
    target_link_libraries(${target} ${GLOG_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_glog)
  if(VPVL2_LINK_GLOG)
    if(MSVC)
      get_source_directory(GLOG_INSTALL_DIR "glog-src")
      find_path(GLOG_INCLUDE_DIR NAMES glog/logging.h PATH_SUFFIXES src/windows PATHS ${GLOG_INSTALL_DIR} NO_DEFAULT_PATH)
      find_library(GLOG_LIBRARY libglog PATH_SUFFIXES ${CMAKE_BUILD_TYPE} PATHS ${GLOG_INSTALL_DIR} NO_DEFAULT_PATH)
    else()
      get_install_directory(GLOG_INSTALL_DIR "glog-src")
      find_path(GLOG_INCLUDE_DIR NAMES glog/logging.h PATH_SUFFIXES include PATHS ${GLOG_INSTALL_DIR} NO_DEFAULT_PATH)
      find_library(GLOG_LIBRARY glog PATH_SUFFIXES lib64 lib32 lib PATHS ${GLOG_INSTALL_DIR} NO_DEFAULT_PATH)
    endif()
    include_directories(${GLOG_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_link_hlslxc target)
  if(HLSLXC_LIBRARY)
    target_link_libraries(${target} ${HLSLXC_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_hlslxc)
  if(VPVL2_ENABLE_EXTENSIONS_EFFECT)
    get_source_directory(HLSLXC_SRC_DIR "hlslxc-src")
    find_path(HLSLXC_INCLUDE_DIR toGLSL.h PATHS ${HLSLXC_SRC_DIR} NO_DEFAULT_PATH)
    find_library(HLSLXC_LIBRARY libHLSLcc PATH_SUFFIXES lib PATHS ${HLSLXC_SRC_DIR} NO_DEFAULT_PATH)
    include_directories(${HLSLXC_INCLUDE_DIR} "${HLSLXC_INCLUDE_DIR}/cbstring")
  endif()
endfunction()

function(vpvl2_link_regal target)
  if(VPVL2_LINK_REGAL)
    target_link_libraries(${target} ${REGAL_LIBRARY})
  endif()
endfunction()

function(vpvl2_find_regal)
  if(VPVL2_LINK_REGAL)
    get_install_directory(REGAL_INSTALL_DIR "regal-src")
    find_path(REGAL_INCLUDE_DIR NAMES GL/Regal.h PATH_SUFFIXES include PATHS ${REGAL_INSTALL_DIR} NO_DEFAULT_PATH)
    find_library(REGAL_LIBRARY Regal PATH_SUFFIXES lib64 lib32 lib PATHS ${REGAL_INSTALL_DIR} NO_DEFAULT_PATH)
    include_directories(${REGAL_INCLUDE_DIR})
  endif()
endfunction()

function(vpvl2_find_all)
  vpvl2_find_gli()
  vpvl2_find_glm()
  vpvl2_find_zlib()
  vpvl2_find_libxml2()
  vpvl2_find_tbb()
  vpvl2_find_bullet()
  vpvl2_find_assimp()
  vpvl2_find_glew()
  vpvl2_find_icu()
  vpvl2_find_nvtt()
  vpvl2_find_vpvl()
  vpvl2_find_alsoft()
  vpvl2_find_alure()
  vpvl2_find_glog()
  vpvl2_find_hlslxc()
  vpvl2_find_regal()
  vpvl2_find_openmp()
  vpvl2_find_cg_runtime()
  vpvl2_find_cl_runtime()
  vpvl2_find_gl_runtime()
endfunction()

function(vpvl2_link_all target)
  vpvl2_link_zlib(${target})
  vpvl2_link_libxml2(${target})
  vpvl2_link_tbb(${target})
  vpvl2_link_bullet(${target})
  vpvl2_link_assimp(${target})
  vpvl2_link_glew(${target})
  vpvl2_link_icu(${target})
  vpvl2_link_nvtt(${target})
  vpvl2_link_vpvl(${target})
  vpvl2_link_glog(${target})
  vpvl2_link_hlslxc(${target})
  vpvl2_link_regal(${target})
  vpvl2_link_cg_runtime(${target})
  vpvl2_link_cl_runtime(${target})
  vpvl2_link_gl_runtime(${target})
endfunction()
# end of functions
