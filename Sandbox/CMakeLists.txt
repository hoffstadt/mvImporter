cmake_minimum_required (VERSION 3.13)

add_executable(sandbox)
set_target_properties(sandbox
  PROPERTIES
  CXX_STANDARD 17
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  )

target_compile_definitions(sandbox

	PRIVATE
		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
)

target_sources(sandbox

	PRIVATE

		"main.cpp"
)

target_include_directories(sandbox

	PRIVATE

		"../mv3D/"
		"../mv3D/vendor/"
		"../mv3D/src/core/"
		"../mv3D/src/renderer/"

		"../mv3D/vendor/imgui/"
		"../mv3D/vendor/imgui/backends/"
		"../mv3D/vendor/stb/"
		"../mv3D/vendor/implot/"

)

target_link_libraries(sandbox
	PRIVATE
		mv3d
)