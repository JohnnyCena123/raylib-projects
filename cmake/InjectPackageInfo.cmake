if(NOT DEFINED EXE_PATH)
	message(FATAL_ERROR "Executable path is required.")
elseif(NOT DEFINED CPACK_PROPERTIES_FILEPATH)
	message(FATAL_ERROR "CPack Properties filepath is required.")
endif()

execute_process(
	COMMAND "${EXE_PATH}" --description --no-metadata
	OUTPUT_VARIABLE DESCRIPTION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND "${EXE_PATH}" --description --minimal-output --no-metadata
	OUTPUT_VARIABLE MIN_DESCRIPTION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND "${EXE_PATH}" --repo --minimal-output
	OUTPUT_VARIABLE REPO
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(STRIP "${DESCRIPTION}" DESCRIPTION)                                  # strip out leading/trailing whitespace
string(STRIP "${MIN_DESCRIPTION}" MIN_DESCRIPTION)                          # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(STRIP "${REPO}" REPO)                                                # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(REPLACE "\n" "\\n\n" DESCRIPTION "${DESCRIPTION}")                   # add the newline characters to the description, dont let them decay into spaces (thanks cmake parser)
string(REPLACE "\n" "\\n\n" MIN_DESCRIPTION "${MIN_DESCRIPTION}")           # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(REPLACE "\"" "\\\"" DESCRIPTION "${DESCRIPTION}")                    # properly escape quote characters if exist
string(REPLACE "\"" "\\\"" MIN_DESCRIPTION "${MIN_DESCRIPTION}")            # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
string(ASCII 27 ESCAPE_CHAR)                                                # theres no other way to produce this specific charcter - all the usual options (\033, \x1b, \e) are unfamiliar to the CMake parser 
set(REGEXP "${ESCAPE_CHAR}\\[[0-9;?]*[a-zA-Z]")                             # regular expression for ANSI escape codes
string(REGEX REPLACE "${REGEXP}" "" DESCRIPTION "${DESCRIPTION}")           # strip out all of them
string(REGEX REPLACE "${REGEXP}" "" MIN_DESCRIPTION "${MIN_DESCRIPTION}")   # ^^^^^^^^^^^^^^^^^^^^^
file(WRITE "${CPACK_PROPERTIES_FILEPATH}"                                   # write the generated description + summary to the the CPackProperties.cmake file
    "set(CPACK_PACKAGE_DESCRIPTION \"${DESCRIPTION}\")\n"                   # write the description
    "set(CPACK_PACKAGE_DESCRIPTION_SUMMARY \"${MIN_DESCRIPTION}\")\n"       # write the summary
)

if(UNIX AND NOT APPLE)
	if(NOT DEFINED DESKTOP_FILE)
		message(WARNING "Please provide the desktop filepath via -DDESKTOP_FILE=<FILEPATH>")
	else()
		file(APPEND "${DESKTOP_FILE}" 
		"\nComment=${MIN_DESCRIPTION}. Source code: ${REPO}.")
	endif()
endif()
