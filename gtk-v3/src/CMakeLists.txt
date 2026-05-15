# GTK v3 client source build.
# Compiles all GTK3-specific source files into the crossfire-client-gtk3 executable.
# Uses gtk+-3.0 libraries; GTK2-specific APIs have been replaced with GTK3 equivalents.
# Cross-platform: X11 dependency is conditional (not present on Windows/macOS).

vala_precompile(VALA_C snd.vala OPTIONS -g GENERATE_HEADER client-vala)

add_custom_command(
    OUTPUT resources.c
    COMMAND ${GLIB_COMPILE_RESOURCES}
        --generate-source
        --target ${CMAKE_CURRENT_BINARY_DIR}/resources.c
        --sourcedir=${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/resources.xml
)

add_executable(crossfire-client-gtk3
    ${VALA_C}
    account.c
    config.c
    create_char.c
    gtk3proto.h
    image.c
    image.h
    info.c
    info.h
    inventory.c
    keys.c
    magicmap.c
    main.c
    main.h
    map.c
    menubar.c
    metaserver.c
    pickup.c
    png.c
    skills.c
    sound.c
    spells.c
    stats.c
    sound.h
    cfsndserv.c
    resources.c
)

# Use GTK3 include dirs when building the GTK3 client.
target_include_directories(crossfire-client-gtk3
    PRIVATE ${PROJECT_BINARY_DIR}
            ${PROJECT_BINARY_DIR}/common
            ${PROJECT_SOURCE_DIR}/common
            ${GTK3_INCLUDE_DIRS}
            ${PNG_INCLUDE_DIRS}
            ${SDLMIXER_INCLUDE_DIRS}
            ${SDL_INCLUDE_DIR}
)

# Link against GTK3 libraries; X11 is optional (not available on Windows/macOS).
target_link_libraries(crossfire-client-gtk3 cfclient
    ${CURL_LDFLAGS}
    ${GTK3_LDFLAGS}
    ${LUA_LIBRARIES}
    ${PNG_LIBRARIES}
    ${SDL_LDFLAGS}
    ${SDLMIXER_LDFLAGS}
    ${X11_LIBRARIES}
    m # math
)

if(MINGW OR WIN32)
    target_link_libraries(crossfire-client-gtk3 wsock32)
endif()

install(TARGETS crossfire-client-gtk3 DESTINATION ${CMAKE_INSTALL_BINDIR})
