# https://stackoverflow.com/questions/71299716/how-to-compile-hlsl-shaders-during-build-with-cmake
# Build HLSL shaders

#adds a target with no output so it will always be built
add_custom_target(shaders)

set(HLSL_SHADER_FILES VertexShader.hlsl PixelShader.hlsl)

#sets shader type and shader model properties to the source files
set_source_files_properties(VertexShader.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(PixelShader.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "4_0")

foreach(FILE ${HLSL_SHADER_FILES})
    #sets FILE_WE as the filename without directory nor longest extension
    get_filename_component(FILE_WE ${FILE} NAME_WE)

    #retrieves source file properties we set above into the specified variables
    get_source_file_property(ShaderType ${FILE} ShaderType)
    get_source_file_property(ShaderModel ${FILE} ShaderModel)

    #adds a custom command to a target such as a library or executable
    #the command becomes part of the target and will only execute when the target itself is built
    #if the target is already built, the command will not execute
    add_custom_command(TARGET shaders
            #the commmand to run
            COMMAND fxc.exe /nologo /Emain /T${ShaderType}_${ShaderModel} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
            #specify the primary input source file to the command
            MAIN_DEPENDENCY ${FILE}
            #display the given message before the commands are executed at build time
            COMMENT "HLSL ${FILE}"
            #execute the command with the given current working directory
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            #all arguments to the commands will be escaped properly for the build tool so that
            #the invoked command receives each argument unchanged
            VERBATIM)

endforeach(FILE)