
file(GLOB source_files "*.cpp" )
file(GLOB header_files "*.h" )

set_plugin_options(
    NAME obj
    CLASSPROP ResourceImporterOBJ
    TYPE resource
    SOURCES ${source_files} ${header_files} ${thirdparty_sources}
    )
