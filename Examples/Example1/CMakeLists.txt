cmake_minimum_required (VERSION 3.13)

add_executable(example1)
set_target_properties(example1
  PROPERTIES
  CXX_STANDARD 17
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}$<$<CONFIG:Release>:/cmake-build-release/>$<$<CONFIG:Debug>:/cmake-build-debug/>mv3D"
  )

target_compile_definitions(example1

	PRIVATE
		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
)

target_sources(example1

	PRIVATE

		"main.cpp"
)

target_include_directories(example1

	PRIVATE

		"../../mv3D/"
		"../../mv3D/vendor/"
		"../../mv3D/src/core/"
		"../../mv3D/src/renderer/"
		"../../mv3D/vendor/imgui/"
		"../../mv3D/vendor/imgui/backends/"
		"../../mv3D/vendor/stb/"
		"../../mv3D/vendor/implot/"

)

target_link_libraries(example1
	PRIVATE
		mv3d
)