file( GLOB systems_source_files "*.cpp" )
ADD_LIBRARY ( Systems
	${systems_source_files}
)