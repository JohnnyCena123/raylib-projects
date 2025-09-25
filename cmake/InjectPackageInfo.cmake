if(NOT DEFINED EXE_PATH)
	message(FATAL_ERROR "Executable path is required.")
elseif(NOT DEFINED SOURCE_CPACK_PROPERTIES_FILEPATH)
	message(FATAL_ERROR "Source (.in) CPack Properties filepath is required.")
elseif(NOT DEFINED CPACK_PROPERTIES_FILEPATH)
	message(FATAL_ERROR "CPack Properties filepath is required.")
endif()

execute_process(
	COMMAND "${EXE_PATH}" --description --no-metadata --no-colors
	OUTPUT_VARIABLE DESCRIPTION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND "${EXE_PATH}" --description --dump --no-metadata --no-colors
	OUTPUT_VARIABLE MIN_DESCRIPTION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND "${EXE_PATH}" --repo --dump --no-colors
	OUTPUT_VARIABLE REPO
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\n" "\\n\n" DESCRIPTION "${DESCRIPTION}")                              # add the newline characters to the description, dont let them decay into spaces (thanks cmake parser)
string(REPLACE "\n" "\\n\n" MIN_DESCRIPTION "${MIN_DESCRIPTION}")                      # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(REPLACE "\"" "\\\"" DESCRIPTION "${DESCRIPTION}")                               # properly escape quote characters if exist
string(REPLACE "\"" "\\\"" MIN_DESCRIPTION "${MIN_DESCRIPTION}")                       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(ASCII 27 ESCAPE_CHAR)                                                           # theres no other way to produce this specific charcter - all the usual options (\033, \x1b, \e) are unfamiliar to the CMake parser
set(REGEXP "${ESCAPE_CHAR}[\\[\\]\\\\]?[0-9;?]*[a-zA-Z]?")                             # regular expression for ANSI escape codes
string(REGEX REPLACE "${REGEXP}" "" DESCRIPTION "${DESCRIPTION}")                      # strip out all of them
string(REGEX REPLACE "${REGEXP}" "" MIN_DESCRIPTION "${MIN_DESCRIPTION}")              # ^^^^^^^^^^^^^^^^^^^^^
configure_file("${SOURCE_CPACK_PROPERTIES_FILEPATH}" "${CPACK_PROPERTIES_FILEPATH}")   # write the generated description + summary to the the CPackProperties.cmake file

if(UNIX AND NOT APPLE)
	if(NOT DEFINED DESKTOP_FILE)
		message(WARNING "Please provide the desktop filepath via -DDESKTOP_FILE=<FILEPATH>")
	else()
		configure_file("${DESKTOP_FILE}.in"
			"${DESKTOP_FILE}")
	endif()
endif()
