file( GLOB tests_executables *.cpp )
#Message(STATUS "CODE :: ${tests_executables}")
foreach( test_source_file ${tests_executables} )
	get_filename_component( test_name ${test_source_file} NAME )
	string( REPLACE ".cpp" "" test_name ${test_name} )
	add_executable( ${test_name} ${test_source_file} )
	target_link_libraries( ${test_name} ${Libraries} )
endforeach( test_source_file ${tests_executables} )
