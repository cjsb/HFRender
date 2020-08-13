cmake_minimum_required (VERSION 3.8)
project(HFRender)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

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
	${CMAKE_CURRENT_SOURCE_DIR}/frame_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/opengl_common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/render_context.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/render_surface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vertex_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/world.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/helpers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/automic_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/svo_vct.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/light.cpp
	${THIRD_PARTY_PATH}/src/glad/glad/glad.c
	${THIRD_PARTY_PATH}/src/tiny_obj_loader/tiny_obj_loader.cpp
	${THIRD_PARTY_PATH}/src/stb_image/stb_image.cpp
	${IMGUI_FILES}
)

set(HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/hf_render.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/frame_buffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/material.h
	${CMAKE_CURRENT_SOURCE_DIR}/model.h
	${CMAKE_CURRENT_SOURCE_DIR}/opengl_common.h
	${CMAKE_CURRENT_SOURCE_DIR}/render_context.h
	${CMAKE_CURRENT_SOURCE_DIR}/render_surface.h
	${CMAKE_CURRENT_SOURCE_DIR}/shader.h
	${CMAKE_CURRENT_SOURCE_DIR}/texture.h
	${CMAKE_CURRENT_SOURCE_DIR}/vertex_buffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/world.h
	${CMAKE_CURRENT_SOURCE_DIR}/helpers.h
	${CMAKE_CURRENT_SOURCE_DIR}/automic_buffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/svo_vct.h
	${CMAKE_CURRENT_SOURCE_DIR}/light.h
	${CMAKE_CURRENT_SOURCE_DIR}/aabb.h
)

add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

list(APPEND LINK_LIBRARYS glfw3.lib opengl32.lib)
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARYS})

