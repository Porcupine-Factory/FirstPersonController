
if(TARGET Gem::AMultiplayerPhysX5.Unified.Static)
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
