
if(TARGET Gem::AMultiplayerPhysX5.Unified.Static)
add_compile_definitions(NETWORKFPC)
set(FILES
    Include/FirstPersonController/FirstPersonControllerBus.h
    Include/FirstPersonController/FirstPersonControllerTypeIds.h
    Include/FirstPersonController/FirstPersonControllerComponentBus.h
    Include/FirstPersonController/FirstPersonExtrasBus.h
    Include/FirstPersonController/FirstPersonExtrasComponentBus.h
    Include/FirstPersonController/CameraCoupledChildBus.h
    Include/FirstPersonController/CameraCoupledChildComponentBus.h
    Include/FirstPersonController/NetworkFPCBus.h
    Include/FirstPersonController/NetworkFPCControllerBus.h
    Include/FirstPersonController/NetworkFPCBotAnimationBus.h
    Include/FirstPersonController/NetworkFPCBotAnimationControllerBus.h
)
else()
set(FILES
    Include/FirstPersonController/FirstPersonControllerBus.h
    Include/FirstPersonController/FirstPersonControllerTypeIds.h
    Include/FirstPersonController/FirstPersonControllerComponentBus.h
    Include/FirstPersonController/FirstPersonExtrasBus.h
    Include/FirstPersonController/FirstPersonExtrasComponentBus.h
    Include/FirstPersonController/CameraCoupledChildBus.h
    Include/FirstPersonController/CameraCoupledChildComponentBus.h
)
endif()
