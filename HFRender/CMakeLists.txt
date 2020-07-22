cmake_minimum_required (VERSION 3.8)
project(HFRender)

set(THIRD_PARTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty)

include_directories(${THIRD_PARTY_PATH}/src)
include_directories(${THIRD_PARTY_PATH}/src/glad)
link_directories(${THIRD_PARTY_PATH}/lib)

set(IMGUI_FILES
	${THIRD_PARTY_PATH}/src/ImGui/imgui_impl_glfw.cpp
	${THIRD_PARTY_PATH}/src/ImGui/imgui_impl_opengl3.cpp
	${THIRD_PARTY_PATH}/src/ImGui/imgui.cpp
	${THIRD_PARTY_PATH}/src/ImGui/imgui_draw.cpp
	${THIRD_PARTY_PATH}/src/ImGui/imgui_widgets.cpp
	${THIRD_PARTY_PATH}/src/ImGui/imgui_demo.cpp
)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/hf_render.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
	${THIRD_PARTY_PATH}/src/glad/glad/glad.c
	${IMGUI_FILES}
)

set(HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/hf_render.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera.h
)

add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

list(APPEND LINK_LIBRARYS glfw3.lib opengl32.lib)
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARYS})
