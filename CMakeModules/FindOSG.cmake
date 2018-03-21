# This module defines

# OSG_LIBRARY
# OSG_FOUND, if false, do not try to link to osg
# OSG_INCLUDE_DIRS, where to find the headers
# OSG_INCLUDE_DIR, where to find the source headers
# OSG_GEN_INCLUDE_DIR, where to find the generated headers

# to use this module, set variables to point to the osg build
# directory, and source directory, respectively
# OSGDIR or OSG_SOURCE_DIR: osg source directory, typically OpenSceneGraph
# OSG_DIR or OSG_BUILD_DIR: osg build directory, place in which you've
#    built osg via cmake 

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgUtil/SceneView>

###### headers ######

SET(OSG_DIR "" CACHE PATH "Set to base OpenSceneGraph install path")

MACRO( FIND_OSG_INCLUDE THIS_OSG_INCLUDE_DIR THIS_OSG_INCLUDE_FILE )

FIND_PATH( ${THIS_OSG_INCLUDE_DIR} ${THIS_OSG_INCLUDE_FILE}
    PATHS
        ${OSG_DIR}
        $ENV{OSG_SOURCE_DIR}
        $ENV{OSGDIR}
        $ENV{OSG_DIR}
        /usr/local/
        /usr/
        /sw/ # Fink
        /opt/local/ # DarwinPorts
        /opt/csw/ # Blastwave
        /opt/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/
        ~/Library/Frameworks
        /Library/Frameworks
    PATH_SUFFIXES
        /include/
)

ENDMACRO( FIND_OSG_INCLUDE THIS_OSG_INCLUDE_DIR THIS_OSG_INCLUDE_FILE )

FIND_OSG_INCLUDE( OSG_GEN_INCLUDE_DIR   osg/Config )
FIND_OSG_INCLUDE( OSG_INCLUDE_DIR       osg/Node )

file (READ ${OSG_GEN_INCLUDE_DIR}/osg/Version _osg_version_header_contents)
IF(_osg_version_header_contents)
    string (REGEX MATCH "OPENSCENEGRAPH_MAJOR_VERSION[ \t]+([0-9]+)" OPENSCENEGRAPH_MAJOR_VERSION ${_osg_version_header_contents})
    string (REGEX MATCH "([0-9]+)" OPENSCENEGRAPH_MAJOR_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION})

    string (REGEX MATCH "OPENSCENEGRAPH_MINOR_VERSION[ \t]+([0-9]+)" OPENSCENEGRAPH_MINOR_VERSION ${_osg_version_header_contents})
    string (REGEX MATCH "([0-9]+)" OPENSCENEGRAPH_MINOR_VERSION ${OPENSCENEGRAPH_MINOR_VERSION})

    string (REGEX MATCH "OPENSCENEGRAPH_PATCH_VERSION[ \t]+([0-9]+)" OPENSCENEGRAPH_PATCH_VERSION ${_osg_version_header_contents})
    string (REGEX MATCH "([0-9]+)" OPENSCENEGRAPH_PATCH_VERSION ${OPENSCENEGRAPH_PATCH_VERSION})

    string (REGEX MATCH "OPENSCENEGRAPH_SOVERSION[ \t]+([0-9]+)" OPENSCENEGRAPH_SOVERSION ${_osg_version_header_contents})
    string (REGEX MATCH "([0-9]+)" OPENSCENEGRAPH_SOVERSION ${OPENSCENEGRAPH_SOVERSION})

    SET(OPENSCENEGRAPH_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}.${OPENSCENEGRAPH_PATCH_VERSION})
    SET(OSG_PLUGINS osgPlugins-${OPENSCENEGRAPH_VERSION})
endif()

###### libraries ######

MACRO( FIND_OSG_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME} ${MYLIBRARYNAME}d
    PATHS
        ${OSG_DIR}
        $ENV{OSG_BUILD_DIR}
        $ENV{OSG_DIR}
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware
    PATH_SUFFIXES
        /lib/
        /lib64/
        /build/lib/
        /build/lib64/
        /Build/lib/
        /Build/lib64/
     )

ENDMACRO(FIND_OSG_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSG_LIBRARY( OSG_LIBRARY osg )
FIND_OSG_LIBRARY( OSG_LIBRARY_DEBUG osgd)

FIND_OSG_LIBRARY( OSGUTIL_LIBRARY osgUtil )
FIND_OSG_LIBRARY( OSGUTIL_LIBRARY_DEBUG osgUtild)

FIND_OSG_LIBRARY( OSGDB_LIBRARY osgDB )
FIND_OSG_LIBRARY( OSGDB_LIBRARY_DEBUG osgDBd)

FIND_OSG_LIBRARY( OSGTEXT_LIBRARY osgText )
FIND_OSG_LIBRARY( OSGTEXT_LIBRARY_DEBUG osgTextd )

FIND_OSG_LIBRARY( OSGTERRAIN_LIBRARY osgTerrain )
FIND_OSG_LIBRARY( OSGTERRAIN_LIBRARY_DEBUG osgTerraind )

FIND_OSG_LIBRARY( OSGFX_LIBRARY osgFX )
FIND_OSG_LIBRARY( OSGFX_LIBRARY_DEBUG osgFXd )

FIND_OSG_LIBRARY( OSGSIM_LIBRARY osgSim )
FIND_OSG_LIBRARY( OSGSIM_LIBRARY_DEBUG osgSimd )

FIND_OSG_LIBRARY( OSGVIEWER_LIBRARY osgViewer )
FIND_OSG_LIBRARY( OSGVIEWER_LIBRARY_DEBUG osgViewerd )

FIND_OSG_LIBRARY( OSGGA_LIBRARY osgGA )
FIND_OSG_LIBRARY( OSGGA_LIBRARY_DEBUG osgGAd )

FIND_OSG_LIBRARY( OSGWIDGET_LIBRARY osgWidget )
FIND_OSG_LIBRARY( OSGWIDGET_LIBRARY_DEBUG osgWidgetd )

FIND_OSG_LIBRARY( OSGSHADOW_LIBRARY osgShadow )
FIND_OSG_LIBRARY( OSGSHADOW_LIBRARY_DEBUG osgShadowd )

FIND_OSG_LIBRARY( OSGMANIPULATOR_LIBRARY osgManipulator )
FIND_OSG_LIBRARY( OSGMANIPULATOR_LIBRARY_DEBUG osgManipulatord )

FIND_OSG_LIBRARY( OSGPARTICLE_LIBRARY osgParticle )
FIND_OSG_LIBRARY( OSGPARTICLE_LIBRARY_DEBUG osgParticled )

FIND_OSG_LIBRARY( OSGANIMATION_LIBRARY osgAnimation )
FIND_OSG_LIBRARY( OSGANIMATION_LIBRARY_DEBUG osgAnimationd )

IF(OPENSCENEGRAPH_VERSION VERSION_LESS "3.5.6")
  FIND_OSG_LIBRARY( OSGQT_LIBRARY osgQt )
  FIND_OSG_LIBRARY( OSGQT_LIBRARY_DEBUG osgQtd )
ELSE(OPENSCENEGRAPH_VERSION VERSION_LESS "3.5.6")
  IF(Qt5Widgets_FOUND)
    FIND_OSG_LIBRARY( OSGQT_LIBRARY osgQt5 )
    FIND_OSG_LIBRARY( OSGQT_LIBRARY_DEBUG osgQt5d )
  ELSE(Qt5Widgets_FOUND)
    FIND_OSG_LIBRARY( OSGQT_LIBRARY osgQt )
    FIND_OSG_LIBRARY( OSGQT_LIBRARY_DEBUG osgQtd )
  ENDIF(Qt5Widgets_FOUND)
ENDIF(OPENSCENEGRAPH_VERSION VERSION_LESS "3.5.6")

FIND_OSG_LIBRARY( OPENTHREADS_LIBRARY OpenThreads )
FIND_OSG_LIBRARY( OPENTHREADS_LIBRARY_DEBUG OpenThreadsd )

SET( OSG_FOUND "NO" )
IF( OSG_LIBRARY AND OSG_INCLUDE_DIR )
    SET( OSG_FOUND "YES" )
    SET( OSG_INCLUDE_DIRS ${OSG_INCLUDE_DIR} ${OSG_GEN_INCLUDE_DIR} )
    GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH )
ENDIF( OSG_LIBRARY AND OSG_INCLUDE_DIR )


