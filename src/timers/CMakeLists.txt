file( GLOB timers_source_files "*.cpp" )
ADD_LIBRARY ( Timers
	${timers_source_files}
)