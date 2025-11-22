
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: o3de_pal_dir will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/FirstPersonController/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/FirstPersonController/Code
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# The FirstPersonController.API target declares the common interface that users of this gem should depend on in their targets
ly_add_target(
    NAME FirstPersonController.API INTERFACE
    NAMESPACE Gem
    FILES_CMAKE
        firstpersoncontroller_api_files.cmake
        ${pal_dir}/firstpersoncontroller_api_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            Include
    BUILD_DEPENDENCIES
        INTERFACE
           AZ::AzCore
)

# The FirstPersonController.Private.Object target is an internal target
# It should not be used outside of this Gems CMakeLists.txt
ly_add_target(
    NAME FirstPersonController.Private.Object STATIC
    NAMESPACE Gem
    FILES_CMAKE
        firstpersoncontroller_private_files.cmake
        firstpersoncontroller_autogen_files.cmake
        ${pal_dir}/firstpersoncontroller_private_files.cmake
    TARGET_PROPERTIES
        O3DE_PRIVATE_TARGET TRUE
    INCLUDE_DIRECTORIES
        PRIVATE
            Include
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            Gem::StartingPointInput.Static
            Gem::PhysX5.Static
            Gem::Atom_RPI.Public
            Gem::MultiplayerPhysX5.Client
            # For autogen components
            PRIVATE
            Gem::MultiplayerPhysX5.Client.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp

)

# Here add FirstPersonController target, it depends on the Private Object library and Public API interface
ly_add_target(
    NAME FirstPersonController ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        firstpersoncontroller_shared_files.cmake
        ${pal_dir}/firstpersoncontroller_shared_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::FirstPersonController.API
        PRIVATE
            Gem::FirstPersonController.Private.Object
)

# By default, we will specify that the above target FirstPersonController would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME FirstPersonController.Clients NAMESPACE Gem TARGETS Gem::FirstPersonController)
ly_create_alias(NAME FirstPersonController.Servers NAMESPACE Gem TARGETS Gem::FirstPersonController)

# For the Client and Server variants of FirstPersonController Gem, an alias to the FirstPersonController.API target will be made
ly_create_alias(NAME FirstPersonController.Clients.API NAMESPACE Gem TARGETS Gem::FirstPersonController.API)
ly_create_alias(NAME FirstPersonController.Servers.API NAMESPACE Gem TARGETS Gem::FirstPersonController.API)

# If we are on a host platform, we want to add the host tools targets like the FirstPersonController.Editor MODULE target
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    # The FirstPersonController.Editor.API target can be used by other gems that want to interact with the FirstPersonController.Editor module
    ly_add_target(
        NAME FirstPersonController.Editor.API INTERFACE
        NAMESPACE Gem
        FILES_CMAKE
            firstpersoncontroller_editor_api_files.cmake
            ${pal_dir}/firstpersoncontroller_editor_api_files.cmake
        INCLUDE_DIRECTORIES
            INTERFACE
                Include
        BUILD_DEPENDENCIES
            INTERFACE
                AZ::AzToolsFramework
    )

    # The FirstPersonController.Editor.Private.Object target is an internal target
    # which is only to be used by this gems CMakeLists.txt and any subdirectories
    # Other gems should not use this target
    ly_add_target(
        NAME FirstPersonController.Editor.Private.Object STATIC
        NAMESPACE Gem
        FILES_CMAKE
            firstpersoncontroller_editor_private_files.cmake
        TARGET_PROPERTIES
            O3DE_PRIVATE_TARGET TRUE
        INCLUDE_DIRECTORIES
            PRIVATE
                Include
                Source
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::FirstPersonController.Private.Object
                Gem::StartingPointInput.Static
                Gem::PhysX5.Static
                Gem::Atom_RPI.Public
    )

    ly_add_target(
        NAME FirstPersonController.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            firstpersoncontroller_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::FirstPersonController.Editor.API
            PRIVATE
                Gem::FirstPersonController.Editor.Private.Object
    )

    # By default, we will specify that the above target FirstPersonController would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME FirstPersonController.Tools    NAMESPACE Gem TARGETS Gem::FirstPersonController.Editor)
    ly_create_alias(NAME FirstPersonController.Builders NAMESPACE Gem TARGETS Gem::FirstPersonController.Editor)

    # For the Tools and Builders variants of FirstPersonController Gem, an alias to the FirstPersonController.Editor API target will be made
    ly_create_alias(NAME FirstPersonController.Tools.API NAMESPACE Gem TARGETS Gem::FirstPersonController.Editor.API)
    ly_create_alias(NAME FirstPersonController.Builders.API NAMESPACE Gem TARGETS Gem::FirstPersonController.Editor.API)

endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    # We globally support tests, see if we support tests on this platform for FirstPersonController.Tests
    if(PAL_TRAIT_FIRSTPERSONCONTROLLER_TEST_SUPPORTED)
        # We support FirstPersonController.Tests on this platform, add dependency on the Private Object target
        ly_add_target(
            NAME FirstPersonController.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE
                firstpersoncontroller_tests_files.cmake
            INCLUDE_DIRECTORIES
                PRIVATE
                    Tests
                    Source
            BUILD_DEPENDENCIES
                PRIVATE
                    AZ::AzTest
                    AZ::AzFramework
                    Gem::FirstPersonController.Private.Object
        )

        # Add FirstPersonController.Tests to googletest
        ly_add_googletest(
            NAME Gem::FirstPersonController.Tests
        )
    endif()

    # If we are a host platform we want to add tools test like editor tests here
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        # We are a host platform, see if Editor tests are supported on this platform
        if(PAL_TRAIT_FIRSTPERSONCONTROLLER_EDITOR_TEST_SUPPORTED)
            # We support FirstPersonController.Editor.Tests on this platform, add FirstPersonController.Editor.Tests target which depends on
            # private FirstPersonController.Editor.Private.Object target
            ly_add_target(
                NAME FirstPersonController.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                FILES_CMAKE
                    firstpersoncontroller_editor_tests_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::FirstPersonController.Private.Object
            )

            # Add FirstPersonController.Editor.Tests to googletest
            ly_add_googletest(
                NAME Gem::FirstPersonController.Editor.Tests
            )
        endif()
    endif()
endif()
