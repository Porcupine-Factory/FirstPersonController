# Determine if the Multiplayer gem is enabled
set(MULTIPLAYER_GEM_ENABLED FALSE)

# Check project.json for the Multiplayer gem
if(NOT MULTIPLAYER_GEM_ENABLED AND EXISTS "${CMAKE_SOURCE_DIR}/project.json")
    file(READ "${CMAKE_SOURCE_DIR}/project.json" _project_json_content)
    if(_project_json_content MATCHES "\"AMultiplayerPhysX5\"" OR
       _project_json_content MATCHES "\"AMultiplayerPhysX5==" OR
       _project_json_content MATCHES "\"AMultiplayerPhysX5>=" OR
       _project_json_content MATCHES "\"AMultiplayerPhysX5~=" OR
       _project_json_content MATCHES "\"AMultiplayerPhysX5>" OR
       _project_json_content MATCHES "\"AMultiplayerPhysX5<")
        set(MULTIPLAYER_GEM_ENABLED TRUE)
    endif()
endif()

# Fallback to checking enabled_gems.cmake
if(NOT MULTIPLAYER_GEM_ENABLED AND EXISTS "${CMAKE_SOURCE_DIR}/Code/enabled_gems.cmake")
    file(READ "${CMAKE_SOURCE_DIR}/Code/enabled_gems.cmake" _enabled_gems_content)
    if(_enabled_gems_content MATCHES "\"AMultiplayerPhysX5\"" OR _enabled_gems_content MATCHES "[ \t\r\n]AMultiplayerPhysX5[ \t\r\n]")
        set(MULTIPLAYER_GEM_ENABLED TRUE)
    endif()
endif()

# When building the engine workspace directly, standard gems are included
if(NOT MULTIPLAYER_GEM_ENABLED AND EXISTS "${CMAKE_SOURCE_DIR}/engine.json")
    if(EXISTS "${CMAKE_SOURCE_DIR}/Gems/AMultiplayerPhysX5")
        set(MULTIPLAYER_GEM_ENABLED TRUE)
    endif()
endif()

if(MULTIPLAYER_GEM_ENABLED)
set(FILES
    Source/FirstPersonControllerModuleInterface.cpp
    Source/FirstPersonControllerModuleInterface.h
    Source/Clients/FirstPersonControllerSystemComponent.cpp
    Source/Clients/FirstPersonControllerSystemComponent.h
    Source/Clients/FirstPersonControllerComponent.cpp
    Source/Clients/FirstPersonControllerComponent.h
    Source/Clients/FirstPersonExtrasComponent.cpp
    Source/Clients/FirstPersonExtrasComponent.h
    Source/Clients/CameraCoupledChildComponent.cpp
    Source/Clients/CameraCoupledChildComponent.h
    Source/Multiplayer/NetworkFPC.cpp
    Source/Multiplayer/NetworkFPC.h
    Source/Multiplayer/NetworkFPCBotAnimation.cpp
    Source/Multiplayer/NetworkFPCBotAnimation.h

    Source/AutoGen/NetworkFPC.AutoComponent.xml
    Source/AutoGen/NetworkFPCBotAnimation.AutoComponent.xml
)
else()
set(FILES
    Source/FirstPersonControllerModuleInterface.cpp
    Source/FirstPersonControllerModuleInterface.h
    Source/Clients/FirstPersonControllerSystemComponent.cpp
    Source/Clients/FirstPersonControllerSystemComponent.h
    Source/Clients/FirstPersonControllerComponent.cpp
    Source/Clients/FirstPersonControllerComponent.h
    Source/Clients/FirstPersonExtrasComponent.cpp
    Source/Clients/FirstPersonExtrasComponent.h
    Source/Clients/CameraCoupledChildComponent.cpp
    Source/Clients/CameraCoupledChildComponent.h
)
endif()
