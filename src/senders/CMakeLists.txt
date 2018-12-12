file( GLOB senders_source_files "*.cpp" )
ADD_LIBRARY ( Senders
	${senders_source_files}
)