
file(GLOB source_files "*.cpp" )
file(GLOB header_files "*.h" )

set_plugin_options(
    NAME bitmap
    CLASSPROP ResourceImporterBitMap
    TYPE resource
    SOURCES ${source_files} ${header_files} ${thirdparty_sources}
    )
