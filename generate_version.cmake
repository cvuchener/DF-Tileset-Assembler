if (GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always
		        RESULT_VARIABLE git_describe_result
			OUTPUT_VARIABLE git_describe_output
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	if ("${git_describe_result}" EQUAL "0")
		set(VERSION_STRING ${git_describe_output})
	endif()
endif()
if (NOT DEFINED VERSION_STRING)
	message(STATUS "Could not use git repo version, use fixed ${DEFAULT_VERSION}")
	set(VERSION_STRING ${DEFAULT_VERSION})
endif()
configure_file(${INPUT_FILE} ${OUTPUT_FILE} @ONLY)
