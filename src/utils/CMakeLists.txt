file( GLOB utils_source_files "*.cpp" )
ADD_LIBRARY ( Utils
	${utils_source_files}
)