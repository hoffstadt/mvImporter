cmake_minimum_required (VERSION 3.13)

add_executable(sandbox2)
set_target_properties(sandbox2
  PROPERTIES
  CXX_STANDARD 17
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  )

target_compile_definitions(sandbox2

	PRIVATE
		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
)

target_sources(sandbox2

	PRIVATE

		"main.cpp"
)

target_include_directories(sandbox2

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

target_link_libraries(sandbox2
	PRIVATE
		mv3d
)