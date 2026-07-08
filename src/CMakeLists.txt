SET(DEMO_NAME rga_drm_img_display)
add_executable(${DEMO_NAME}
${DEMO_NAME}.cpp
)
target_link_libraries(${DEMO_NAME}
    utils_obj
    ${RGA_LIB}
    drm
)
