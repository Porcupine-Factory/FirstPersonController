/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/NameConstants.h>
#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/InputDeviceId.h>

#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>

#include <PhysX/CharacterControllerBus.h>
#include <PhysX/Material/PhysXMaterial.h>
#include <System/PhysXSystem.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonControllerComponent::Reflect(AZ::ReflectContext* rc)
    {
        if(auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonControllerComponent, AZ::Component>()
              // Input Bindings group
              ->Field("Forward Key", &FirstPersonControllerComponent::m_strForward)
              ->Field("Back Key", &FirstPersonControllerComponent::m_strBack)
              ->Field("Left Key", &FirstPersonControllerComponent::m_strLeft)
              ->Field("Right Key", &FirstPersonControllerComponent::m_strRight)
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_strYaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_strPitch)
              ->Field("Sprint Key", &FirstPersonControllerComponent::m_strSprint)
              ->Field("Crouch Key", &FirstPersonControllerComponent::m_strCrouch)
              ->Field("Jump Key", &FirstPersonControllerComponent::m_strJump)

              // Camera group
              ->Field("Camera Smooth Follow", &FirstPersonControllerComponent::m_cameraSmoothFollow)
              ->Field("Camera Entity", &FirstPersonControllerComponent::m_cameraEntityId)
                  ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonControllerComponent::SetCameraEntity)
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yawSensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitchSensitivity)
              ->Field("Camera Rotation Damp Factor", &FirstPersonControllerComponent::m_rotationDamp)

              // Direction Scale Factors group
              ->Field("Forward Scale", &FirstPersonControllerComponent::m_forwardScale)
              ->Field("Back Scale", &FirstPersonControllerComponent::m_backScale)
              ->Field("Left Scale", &FirstPersonControllerComponent::m_leftScale)
              ->Field("Right Scale", &FirstPersonControllerComponent::m_rightScale)

              // X&Y Movement group
              ->Field("Top Walking Speed", &FirstPersonControllerComponent::m_speed)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
              ->Field("Walking Acceleration", &FirstPersonControllerComponent::m_accel)
                  ->Attribute(AZ::Edit::Attributes::Suffix, AZStd::string::format(" m%ss%s%s", Physics::NameConstants::GetInterpunct().c_str(), Physics::NameConstants::GetSuperscriptMinus().c_str(), Physics::NameConstants::GetSuperscriptTwo().c_str()))
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Opposing Direction Deceleration Factor", &FirstPersonControllerComponent::m_opposingDecel)
              ->Field("Add Velocity For Physics Timestep Instead Of Tick", &FirstPersonControllerComponent::m_addVelocityForTimestepVsTick)
              ->Field("X&Y Movement Tracks Surface Inclines", &FirstPersonControllerComponent::m_velocityXCrossYTracksNormal)
              ->Field("Speed Reduced When Moving Up Inclines", &FirstPersonControllerComponent::m_movingUpInclineSlowed)
              ->Field("Instant Velocity Rotation", &FirstPersonControllerComponent::m_instantVelocityRotation)

              // Sprinting group
              ->Field("Sprint Forward Scale", &FirstPersonControllerComponent::m_sprintScaleForward)
              ->Field("Sprint Back Scale", &FirstPersonControllerComponent::m_sprintScaleBack)
              ->Field("Sprint Left Scale", &FirstPersonControllerComponent::m_sprintScaleLeft)
              ->Field("Sprint Right Scale", &FirstPersonControllerComponent::m_sprintScaleRight)
              ->Field("Sprint Acceleration Scale", &FirstPersonControllerComponent::m_sprintAccelScale)
              ->Field("Sprint Max Time", &FirstPersonControllerComponent::m_sprintMaxTime)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " s")
              ->Field("Sprint Cooldown Time", &FirstPersonControllerComponent::m_sprintTotalCooldownTime)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " s")
              ->Field("Sprint Backwards", &FirstPersonControllerComponent::m_sprintBackwards)
              ->Field("Sprint While Crouched", &FirstPersonControllerComponent::m_sprintWhileCrouched)

              // Crouching group
              ->Field("Crouch Movement Speed Scale", &FirstPersonControllerComponent::m_crouchScale)
              ->Field("Crouch Distance", &FirstPersonControllerComponent::m_crouchDistance)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Crouch Time", &FirstPersonControllerComponent::m_crouchTime)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " s")
              ->Field("Crouch Start Speed", &FirstPersonControllerComponent::m_crouchDownInitVelocity)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
              ->Field("Stand Time", &FirstPersonControllerComponent::m_standTime)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " s")
              ->Field("Stand Start Speed", &FirstPersonControllerComponent::m_crouchUpInitVelocity)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
              ->Field("Crouch Standing Head Clearance", &FirstPersonControllerComponent::m_uncrouchHeadSphereCastOffset)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Crouch Enable Toggle", &FirstPersonControllerComponent::m_crouchEnableToggle)
              ->Field("Crouch Jump Causes Standing", &FirstPersonControllerComponent::m_crouchJumpCausesStanding)
              ->Field("Crouch Sprint Causes Standing", &FirstPersonControllerComponent::m_crouchSprintCausesStanding)
              ->Field("Crouch Priority When Sprint Pressed", &FirstPersonControllerComponent::m_crouchPriorityWhenSprintPressed)

              // Jumping group
              ->Field("Grounded Collision Group", &FirstPersonControllerComponent::m_groundedCollisionGroupId)
              ->Field("Jump Head Hit Collision Group", &FirstPersonControllerComponent::m_headCollisionGroupId)
              ->Field("Gravity", &FirstPersonControllerComponent::m_gravity)
                  ->Attribute(AZ::Edit::Attributes::Suffix, AZStd::string::format(" m%ss%s%s", Physics::NameConstants::GetInterpunct().c_str(), Physics::NameConstants::GetSuperscriptMinus().c_str(), Physics::NameConstants::GetSuperscriptTwo().c_str()))
              ->Field("Jump Initial Velocity", &FirstPersonControllerComponent::m_jumpInitialVelocity)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
              ->Field("Second Jump Initial Velocity", &FirstPersonControllerComponent::m_jumpSecondInitialVelocity)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
              ->Field("Jump Held Gravity Factor", &FirstPersonControllerComponent::m_jumpHeldGravityFactor)
              ->Field("Jump Falling Gravity Factor", &FirstPersonControllerComponent::m_jumpFallingGravityFactor)
              ->Field("X&Y Acceleration Jump Factor", &FirstPersonControllerComponent::m_jumpAccelFactor)
              ->Field("Ground Sphere Casts' Radius Percentage Increase", &FirstPersonControllerComponent::m_groundSphereCastsRadiusPercentageIncrease)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " %")
              ->Field("Grounded Offset", &FirstPersonControllerComponent::m_groundedSphereCastOffset)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Ground Close Offset", &FirstPersonControllerComponent::m_groundCloseSphereCastOffset)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Jump Hold Distance", &FirstPersonControllerComponent::m_jumpHoldDistance)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Jump Head Hit Detection Distance", &FirstPersonControllerComponent::m_jumpHeadSphereCastOffset)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
              ->Field("Jump Head Hit Sets Apogee", &FirstPersonControllerComponent::m_headHitSetsApogee)
              ->Field("Jump Head Hit Ignore Dynamic Rigid Bodies", &FirstPersonControllerComponent::m_jumpHeadIgnoreDynamicRigidBodies)
              ->Field("Jump While Crouched", &FirstPersonControllerComponent::m_jumpWhileCrouched)
              ->Field("Enable Double Jump", &FirstPersonControllerComponent::m_doubleJumpEnabled)
              ->Field("Coyote Time", &FirstPersonControllerComponent::m_coyoteTime)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                ->Attribute(AZ::Edit::Attributes::Min, 0.f)
              ->Field("Apply Gravity During Coyote Time", &FirstPersonControllerComponent::m_applyGravityDuringCoyoteTime)
              ->Field("Last Ground Normal Applies During Coyote Time", &FirstPersonControllerComponent::m_coyoteTimeTracksLastNormal)
              ->Field("Update X&Y Velocity When Ascending", &FirstPersonControllerComponent::m_updateXYAscending)
              ->Field("Update X&Y Velocity When Descending", &FirstPersonControllerComponent::m_updateXYDescending)
              ->Field("Update X&Y Velocity Only When Ground Close", &FirstPersonControllerComponent::m_updateXYOnlyNearGround)

              // Impulse group
              ->Field("Enable Impulse", &FirstPersonControllerComponent::m_enableImpulses)
              ->Field("Use Friction For Deceleration", &FirstPersonControllerComponent::m_impulseDecelUsesFriction)
              ->Field("Mass", &FirstPersonControllerComponent::m_characterMass)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.00001f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetMassUnit())
              ->Field("Impulse Linear Damping", &FirstPersonControllerComponent::m_impulseLinearDamp)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.f)
              ->Field("Impulse Constant Deceleration", &FirstPersonControllerComponent::m_impulseConstantDecel)
                  ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, AZStd::string::format(" m%ss%s%s", Physics::NameConstants::GetInterpunct().c_str(), Physics::NameConstants::GetSuperscriptMinus().c_str(), Physics::NameConstants::GetSuperscriptTwo().c_str()))

              // Hit Detection group
              ->Field("Enable Hit Detection", &FirstPersonControllerComponent::m_enableCharacterHits)
              ->Field("Capsule Radius Detection Percentage Increase", &FirstPersonControllerComponent::m_hitRadiusPercentageIncrease)
                  ->Attribute(AZ::Edit::Attributes::Min, -100.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " %")
              ->Field("Capsule Height Detection Percentage Increase", &FirstPersonControllerComponent::m_hitHeightPercentageIncrease)
                  ->Attribute(AZ::Edit::Attributes::Min, -100.f)
                  ->Attribute(AZ::Edit::Attributes::Suffix, " %")
              ->Field("Hit Detection Group", &FirstPersonControllerComponent::m_characterHitCollisionGroupId)

              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonControllerComponent>("First Person Controller",
                    "The First Person Controller component gives you a fully-featured character controller for your first person game")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Input Bindings")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strForward,
                        "Forward Key", "Key for moving forward. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strBack,
                        "Back Key", "Key for moving backward. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strLeft,
                        "Left Key", "Key for moving left. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strRight,
                        "Right Key", "Key for moving right. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strYaw,
                        "Camera Yaw Rotate Input", "Camera (and Character) left/right rotation control. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strPitch,
                        "Camera Pitch Rotate Input", "Camera up/down rotation control. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strSprint,
                        "Sprint Key", "Key for sprinting. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strCrouch,
                        "Crouch Key", "Key for crouching. Must match an Event Name in the .inputbindings file.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strJump,
                        "Jump Key", "Key for jumping. Must match an Event Name in the .inputbindings file.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Camera")
                    ->Attribute(AutoExpand, false)
                    ->DataElement(nullptr, &FirstPersonControllerComponent::m_cameraSmoothFollow,
                        "Camera Smooth Follow", "If enabled, the camera follows the character using linear interpolation on the frame tick; otherwise, the camera follows its parent transform.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(0,
                        &FirstPersonControllerComponent::m_cameraEntityId,
                        "Camera Entity", "The camera entity to use for the first-person view.")
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &FirstPersonControllerComponent::GetCameraNotSmoothFollow)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yawSensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitchSensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rotationDamp,
                        "Camera Rotation Damp Factor", "The 'smoothness' of the camera rotation. Applies a damp factor to the camera rotation. Setting this to anything greater than or equal to 100 will disable this effect.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "X&Y Movement")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_speed,
                        "Top Walking Speed", "Determines maximum walking speed of the character.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_accel,
                        "Walking Acceleration", "Determines how quickly the character will reach the desired velocity.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_decel,
                        "Deceleration Factor", "Determines how quickly the character will stop. The product of this number and Walking Acceleration determines the resulting deceleration. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_opposingDecel,
                        "Opposing Direction Deceleration Factor", "Determines the deceleration when opposing the current direction of motion. The product of this number and Walking Acceleration creates the deceleration that's used. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_addVelocityForTimestepVsTick,
                        "Add Velocity For Physics Timestep Instead Of Tick", "If this is enabled then the velocity will be applied on each physics timestep, if it is disabled then the velocity will be applied on each tick (frame).")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_velocityXCrossYTracksNormal,
                        "X&Y Movement Tracks Surface Inclines", "Determines whether the character's X&Y movement will be tilted in order to follow inclines. This will apply up to the max angle that is specified in the PhysX Character Controller component.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                      ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_movingUpInclineSlowed,
                        "Speed Reduced When Moving Up Inclines", "Determines whether the character's X&Y movement speed is reduced when going up inclines.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetVelocityXCrossYTracksNormal)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_instantVelocityRotation,
                        "Instant Velocity Rotation", "Determines whether the velocity vector can rotate instantaneously with respect to the world coordinate system, if set to false then the acceleration and deceleration will apply when rotating the character.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Direction Scale Factors")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_forwardScale,
                        "Forward Scale", "Forward movement scale factor.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_backScale,
                        "Back Scale", "Back movement scale factor.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_leftScale,
                        "Left Scale", "Left movement scale factor.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rightScale,
                        "Right Scale", "Right movement scale factor.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Sprinting")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleForward,
                        "Sprint Forward Scale", "Determines the sprint factor applied in the forward direction. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleBack,
                        "Sprint Back Scale", "Determines the sprint factor applied in the back direction. It is suggested to use a number greater than or equal to 1.0 for this. When set to 1.0, it makes it so that there will be no sprint applied when moving only backwards (not considering left/right component to the movement).")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleLeft,
                        "Sprint Left Scale", "Determines the sprint factor applied in the left direction. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleRight,
                        "Sprint Right Scale", "Determines the sprint factor applied in the right direction. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintAccelScale,
                        "Sprint Acceleration Scale", "Determines how quickly the character will reach the desired velocity while sprinting. It is suggested to use a number greater than or equal to 1.0 for this.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintMaxTime,
                        "Sprint Max Time", "The maximum consecutive sprinting time before beginning Sprint Cooldown. The underlying quantity of Stamina is set by this number.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintTotalCooldownTime,
                        "Sprint Cooldown Time", "The time required to wait before sprinting or using Stamina once Sprint Max Time has been reached or Stamina hits 0%.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintBackwards,
                        "Sprint Backwards", "Determines whether sprint can be applied when there is any backwards component to the movement. Enabling this does not inherently make it so you can sprint backwards. You will also have to set Sprint Back Scale to something greater than 1.0 to have that effect. If Sprint Back, Left, and Right Scale are all set to 1.0, then this effectively does nothing.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintWhileCrouched,
                        "Sprint While Crouched", "Determines whether the character can sprint while crouched.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouching")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchScale,
                        "Crouch Movement Speed Scale", "Determines how much slow the character will move when crouched. The product of this number and the top walk speed is the top crouch walk speed.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchDistance,
                        "Crouch Distance", "Determines the distance the camera will move on the Z axis and the reduction in the PhysX Character Controller's capsule collider height. This number cannot be greater than the capsule's height minus two times its radius.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchTime,
                        "Crouch Time", "Determines the time it takes to crouch down from standing.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchDownInitVelocity,
                        "Crouch Start Speed", "The initial speed for the crouching motion. If this speed is too slow to reach the Crouch Distance within the Crouch Time then the slowest constant velocity will be used instead and a warning will be printed. If the Crouch Distance can be reached at a slower velocity then a constant deceleration will be applied to meet the Crouch Time.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_standTime,
                        "Stand Time", "Determines the time it takes to stand up from crouching.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchUpInitVelocity,
                        "Stand Start Speed", "The initial speed for the standing motion. If this speed is too slow to reach the (standing) Crouch Distance within the Stand Time then the slowest constant velocity will be used instead and a warning will be printed. If the (standing) Crouch Distance can be reached at a slower velocity then a constant deceleration will be applied to meet the Stand Time.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_uncrouchHeadSphereCastOffset,
                        "Crouch Standing Head Clearance", "Determines the distance above the player's head to detect whether there is an obstruction and prevent them from fully standing up if there is.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchEnableToggle,
                        "Crouch Enable Toggle", "Determines whether the crouch key toggles crouching. Disabling this requires the crouch key to be held to maintain crouch.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchJumpCausesStanding,
                        "Crouch Jump Causes Standing", "Determines whether pressing jump while crouched causes the character to stand up, and then jump once fully standing if the jump key is held. Disabling this will prevent jumping while crouched.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchSprintCausesStanding,
                        "Crouch Sprint Causes Standing", "Determines whether pressing sprint while crouched causes the character to stand up, and then sprint once fully standing.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchPriorityWhenSprintPressed,
                        "Crouch Priority When Sprint Pressed", "Determines whether pressing crouch while sprint is held causes the character to crouch.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Jumping")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundedCollisionGroupId,
                        "Grounded Collision Group", "The collision group which will be used for the ground detection.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_headCollisionGroupId,
                        "Jump Head Hit Collision Group", "The collision group which will be used for the jump head hit detection.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpWhileCrouched,
                        "Jump While Crouched", "Allow jumping while crouched.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_gravity,
                        "Gravity", "Z Acceleration due to gravity, set this to zero if using the PhysX Character Gameplay component's gravity instead.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpInitialVelocity,
                        "Jump Initial Velocity", "The velocity used when initiating the jump.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpSecondInitialVelocity,
                        "Second Jump Initial Velocity", "The initial velocity that's used for the second jump.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetDoubleJump)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeldGravityFactor,
                        "Jump Held Gravity Factor", "The factor applied to the character's gravity for the beginning of the jump.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpFallingGravityFactor,
                        "Jump Falling Gravity Factor", "The factor applied to the character's gravity when the character is falling. This applies during any ungrounded descent, whereas during a jump ascent, the Gravity value is used along with the Jump Held Gravity Factor.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpAccelFactor,
                        "X&Y Acceleration Jump Factor", "X&Y acceleration factor while in the air. This depends on whether Update X&Y Velocity When Ascending, Update X&Y Velocity When Descending, or Update X&Y Velocity Only When Ground Close is enabled.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHoldDistance,
                        "Jump Hold Distance", "Effectively determines the time that jump may be held. During this initial period of the jump, the Jump Held Gravity Factor is applied, making the maximum height greater. If the number entered here exceeds the calculated apogee, you will get a warning message.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundedSphereCastOffset,
                        "Grounded Offset", "Determines the offset distance between the bottom of the character and ground.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundCloseSphereCastOffset,
                        "Ground Close Offset", "Determines the offset distance between the bottom of the character and ground.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundSphereCastsRadiusPercentageIncrease,
                        "Ground Sphere Casts' Radius Percentage Increase", "The percentage increase in the radius of the ground and ground close sphere casts over the PhysX Character Controller's capsule radius.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeadSphereCastOffset,
                        "Jump Head Hit Detection Distance", "The distance above the character's head where an obstruction will be detected for jumping. The apogee of the jump occurs when there is a collision.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_headHitSetsApogee,
                        "Jump Head Hit Sets Apogee", "Determines whether a collision with the head hit sphere cast causes the character's jump velocity to imminently stop, defining that point as the apogee of a jump.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeadIgnoreDynamicRigidBodies,
                        "Jump Head Hit Ignore Dynamic Rigid Bodies", "Determines whether or not non-kinematic (dynamic) rigid bodies are ignored by the jump head collision detection system.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_doubleJumpEnabled,
                        "Enable Double Jump", "Turn this on to enable double jumping.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_coyoteTime,
                        "Coyote Time", "Grace period after walking off a ledge during which a jump is still allowed.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_applyGravityDuringCoyoteTime,
                        "Apply Gravity During Coyote Time", "If disabled, gravity is not applied during the coyote time, allowing the character to 'hang' briefly when walking off a ledge.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetCoyoteTimeGreaterThanZero)

                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_coyoteTimeTracksLastNormal,
                        "Last Ground Normal Applies During Coyote Time", "Determines if the last normal vector that the character was in contact with when walking off a ledge is kept applied during coyote time.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetCoyoteTimeGreaterThanZeroAndNoGravityDuring)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYAscending,
                        "Update X&Y Velocity When Ascending", "Allows movement in X&Y during a jump’s ascent.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYDescending,
                        "Update X&Y Velocity When Descending", "Allows movement in X&Y during a jump’s descent.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYOnlyNearGround,
                        "Update X&Y Velocity Only When Ground Close", "Allows movement in X&Y only if close to an acceptable ground entity. According to the distance set in Jump Hold Distance. If the ascending and descending options are disabled, then this will effectively do nothing.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Impulse")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_enableImpulses,
                        "Enable Impulses", "Determines whether impulses can be applied to the character via the EBus (e.g. scripts). Dynamic / simulated rigid bodies will not apply impulses to the character without using the EBus.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_impulseDecelUsesFriction,
                        "Use Friction For Deceleration", "Use the PhysX collider's coefficient of friction beneath the character to determine the constant deceleration the character will experience when an impulse is applied. This calculation will be used instead of value entered in 'Impulse Constant Deceleration', but can still be used along with 'Impulse Linear Damping' if it is non-zero.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableImpulses)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_characterMass,
                        "Mass", "Mass of the character for impulse calculations.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableImpulses)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_impulseLinearDamp,
                        "Impulse Linear Damping", "Slows down the character after an impulse the same way as is done by the PhysX Dynamic Rigid Body component, using a first-order homogeneous linear recurrence relation. Specifically, the velocity decays by a factor of (1 - Linear Damping / Fixed Time Step). Linear damping behaves like to Stokes' Law whereas constant deceleration behaves the same as kinetic friction. This is used in combination with Impulse Constant Deceleration, set either to zero to use just one or the other.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableImpulses)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_impulseConstantDecel,
                        "Impulse Constant Deceleration", "The constant rate at which the component of the character's velocity that's due to impulses is reduced over time. A constant deceleration behaves the same as kinetic friction whereas linear damping behaves like Stokes' Law. This is used in combination with Impulse Linear Damping, set either to zero to use just one or the other. If 'Use Friction For Deceleration' is turned on then this constant will not be used.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableImpulsesAndNotDecelUsesFriction)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Hit Detection")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_enableCharacterHits,
                        "Enable Hit Detection", "Determines whether collisions with the character will be detected by a capsule shapecast.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_hitRadiusPercentageIncrease,
                        "Capsule Radius Detection Percentage Increase", "Percentage to increase the character's capsule collider radius by to determine hits / collisions.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableCharacterHits)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_hitHeightPercentageIncrease,
                        "Capsule Height Detection Percentage Increase", "Percentage to increase the character's capsule collider height by to determine hits / collisions.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableCharacterHits)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_characterHitCollisionGroupId,
                        "Hit Detection Group", "Collision group that will be detected by the capsule shapecast.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &FirstPersonControllerComponent::GetEnableCharacterHits);
            }
        }

        if(auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonControllerComponentNotificationBus>("FirstPersonControllerComponentNotificationBus")
                ->Handler<FirstPersonControllerComponentNotificationHandler>();

            bc->EBus<FirstPersonControllerComponentRequestBus>("FirstPersonControllerComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Controller")
                ->Event("Get Character EntityId", &FirstPersonControllerComponentRequests::GetCharacterEntityId)
                ->Event("Get Active Camera EntityId", &FirstPersonControllerComponentRequests::GetActiveCameraEntityId)
                ->Event("Get Active Camera Entity Pointer", &FirstPersonControllerComponentRequests::GetActiveCameraEntityPtr)
                ->Event("Set Camera Entity", &FirstPersonControllerComponentRequests::SetCameraEntity)
                ->Event("Get Camera Smooth Follow", &FirstPersonControllerComponentRequests::GetCameraSmoothFollow)
                ->Event("Set Camera Smooth Follow", &FirstPersonControllerComponentRequests::SetCameraSmoothFollow)
                ->Event("Set Do Not Update On Parent Changed Behavior", &FirstPersonControllerComponentRequests::SetParentChangeDoNotUpdate)
                ->Event("Set Update On Parent Changed Behavior", &FirstPersonControllerComponentRequests::SetParentChangeUpdate)
                ->Event("Get On Parent Changed Behavior", &FirstPersonControllerComponentRequests::GetParentChangeBehavior)
                ->Event("Get Eye Height", &FirstPersonControllerComponentRequests::GetEyeHeight)
                ->Event("Set Eye Height", &FirstPersonControllerComponentRequests::SetEyeHeight)
                ->Event("Get Camera Local Z Travel Distance", &FirstPersonControllerComponentRequests::GetCameraLocalZTravelDistance)
                ->Event("Get Camera Rotation Transform", &FirstPersonControllerComponentRequests::GetCameraRotationTransform)
                ->Event("Reacquire Child EntityIds", &FirstPersonControllerComponentRequests::ReacquireChildEntityIds)
                ->Event("Get Child EntityIds", &FirstPersonControllerComponentRequests::GetChildEntityIds)
                ->Event("Reacquire Capsule Dimensions", &FirstPersonControllerComponentRequests::ReacquireCapsuleDimensions)
                ->Event("Reacquire Max Slope Angle", &FirstPersonControllerComponentRequests::ReacquireMaxSlopeAngle)
                ->Event("Get Forward Event Name", &FirstPersonControllerComponentRequests::GetForwardEventName)
                ->Event("Set Forward Event Name", &FirstPersonControllerComponentRequests::SetForwardEventName)
                ->Event("Get Forward Scale", &FirstPersonControllerComponentRequests::GetForwardScale)
                ->Event("Set Forward Scale", &FirstPersonControllerComponentRequests::SetForwardScale)
                ->Event("Get Forward Input Value", &FirstPersonControllerComponentRequests::GetForwardInputValue)
                ->Event("Set Forward Input Value", &FirstPersonControllerComponentRequests::SetForwardInputValue)
                ->Event("Get Back Event Name", &FirstPersonControllerComponentRequests::GetBackEventName)
                ->Event("Set Back Event Name", &FirstPersonControllerComponentRequests::SetBackEventName)
                ->Event("Get Back Scale", &FirstPersonControllerComponentRequests::GetBackScale)
                ->Event("Set Back Scale", &FirstPersonControllerComponentRequests::SetBackScale)
                ->Event("Get Back Input Value", &FirstPersonControllerComponentRequests::GetBackInputValue)
                ->Event("Set Back Input Value", &FirstPersonControllerComponentRequests::SetBackInputValue)
                ->Event("Get Left Event Name", &FirstPersonControllerComponentRequests::GetLeftEventName)
                ->Event("Set Left Event Name", &FirstPersonControllerComponentRequests::SetLeftEventName)
                ->Event("Get Left Scale", &FirstPersonControllerComponentRequests::GetLeftScale)
                ->Event("Set Left Scale", &FirstPersonControllerComponentRequests::SetLeftScale)
                ->Event("Get Left Input Value", &FirstPersonControllerComponentRequests::GetLeftInputValue)
                ->Event("Set Left Input Value", &FirstPersonControllerComponentRequests::SetLeftInputValue)
                ->Event("Get Right Event Name", &FirstPersonControllerComponentRequests::GetRightEventName)
                ->Event("Set Right Event Name", &FirstPersonControllerComponentRequests::SetRightEventName)
                ->Event("Get Right Scale", &FirstPersonControllerComponentRequests::GetRightScale)
                ->Event("Set Right Scale", &FirstPersonControllerComponentRequests::SetRightScale)
                ->Event("Get Right Input Value", &FirstPersonControllerComponentRequests::GetRightInputValue)
                ->Event("Set Right Input Value", &FirstPersonControllerComponentRequests::SetRightInputValue)
                ->Event("Get Yaw Event Name", &FirstPersonControllerComponentRequests::GetYawEventName)
                ->Event("Set Yaw Event Name", &FirstPersonControllerComponentRequests::SetYawEventName)
                ->Event("Get Yaw Input Value", &FirstPersonControllerComponentRequests::GetYawInputValue)
                ->Event("Set Yaw Input Value", &FirstPersonControllerComponentRequests::SetYawInputValue)
                ->Event("Get Pitch Event Name", &FirstPersonControllerComponentRequests::GetPitchEventName)
                ->Event("Set Pitch Event Name", &FirstPersonControllerComponentRequests::SetPitchEventName)
                ->Event("Get Pitch Input Value", &FirstPersonControllerComponentRequests::GetPitchInputValue)
                ->Event("Set Pitch Input Value", &FirstPersonControllerComponentRequests::SetPitchInputValue)
                ->Event("Get Sprint Event Name", &FirstPersonControllerComponentRequests::GetSprintEventName)
                ->Event("Set Sprint Event Name", &FirstPersonControllerComponentRequests::SetSprintEventName)
                ->Event("Get Sprint Input Value", &FirstPersonControllerComponentRequests::GetSprintInputValue)
                ->Event("Set Sprint Input Value", &FirstPersonControllerComponentRequests::SetSprintInputValue)
                ->Event("Get Crouch Event Name", &FirstPersonControllerComponentRequests::GetCrouchEventName)
                ->Event("Set Crouch Event Name", &FirstPersonControllerComponentRequests::SetCrouchEventName)
                ->Event("Get Crouch Input Value", &FirstPersonControllerComponentRequests::GetCrouchInputValue)
                ->Event("Set Crouch Input Value", &FirstPersonControllerComponentRequests::SetCrouchInputValue)
                ->Event("Get Jump Event Name", &FirstPersonControllerComponentRequests::GetJumpEventName)
                ->Event("Set Jump Event Name", &FirstPersonControllerComponentRequests::SetJumpEventName)
                ->Event("Get Jump Input Value", &FirstPersonControllerComponentRequests::GetJumpInputValue)
                ->Event("Set Jump Input Value", &FirstPersonControllerComponentRequests::SetJumpInputValue)
                ->Event("Get Grounded", &FirstPersonControllerComponentRequests::GetGrounded)
                ->Event("Set Grounded For Tick", &FirstPersonControllerComponentRequests::SetGroundedForTick)
                ->Event("Get Ground Scene Query Hits", &FirstPersonControllerComponentRequests::GetGroundSceneQueryHits)
                ->Event("Get Ground Close Scene Query Hits", &FirstPersonControllerComponentRequests::GetGroundCloseSceneQueryHits)
                ->Event("Get Ground Close Coyote Time Scene Query Hits", &FirstPersonControllerComponentRequests::GetGroundCloseCoyoteTimeSceneQueryHits)
                ->Event("Get Ground Sum Normals Direction", &FirstPersonControllerComponentRequests::GetGroundSumNormalsDirection)
                ->Event("Get Ground Close Sum Normals Direction", &FirstPersonControllerComponentRequests::GetGroundCloseSumNormalsDirection)
                ->Event("Get Scene Query Hit Result Flags", &FirstPersonControllerComponentRequests::GetSceneQueryHitResultFlags)
                ->Event("Get Scene Query Hit EntityId", &FirstPersonControllerComponentRequests::GetSceneQueryHitEntityId)
                ->Event("Get Scene Query Hit Normal", &FirstPersonControllerComponentRequests::GetSceneQueryHitNormal)
                ->Event("Get Scene Query Hit Position", &FirstPersonControllerComponentRequests::GetSceneQueryHitPosition)
                ->Event("Get Scene Query Hit Distance", &FirstPersonControllerComponentRequests::GetSceneQueryHitDistance)
                ->Event("Get Scene Query Hit MaterialId", &FirstPersonControllerComponentRequests::GetSceneQueryHitMaterialId)
                ->Event("Get Scene Query Hit Material Pointer", &FirstPersonControllerComponentRequests::GetSceneQueryHitMaterialPtr)
                ->Event("Get Scene Query Hit Material Asset", &FirstPersonControllerComponentRequests::GetSceneQueryHitMaterialAsset)
                ->Event("Get Scene Query Hit Material Asset Id", &FirstPersonControllerComponentRequests::GetSceneQueryHitMaterialAssetId)
                ->Event("Get Scene Query Hit Dynamic Friction", &FirstPersonControllerComponentRequests::GetSceneQueryHitDynamicFriction)
                ->Event("Get Scene Query Hit Static Friction", &FirstPersonControllerComponentRequests::GetSceneQueryHitStaticFriction)
                ->Event("Get Scene Query Hit Restitution", &FirstPersonControllerComponentRequests::GetSceneQueryHitRestitution)
                ->Event("Get Scene Query Hit Shape Pointer", &FirstPersonControllerComponentRequests::GetSceneQueryHitShapePtr)
                ->Event("Get Scene Query Hit Is In Group Name", &FirstPersonControllerComponentRequests::GetSceneQueryHitIsInGroupName)
                ->Event("Get Scene Query Hit Simulated Body Handle", &FirstPersonControllerComponentRequests::GetSceneQueryHitSimulatedBodyHandle)
                ->Event("Get Layer Name Is In Group Name", &FirstPersonControllerComponentRequests::GetLayerNameIsInGroupName)
                ->Event("Get Ground Close", &FirstPersonControllerComponentRequests::GetGroundClose)
                ->Event("Set Ground Close For Tick", &FirstPersonControllerComponentRequests::SetGroundCloseForTick)
                ->Event("Get Grounded Collision Group Name", &FirstPersonControllerComponentRequests::GetGroundedCollisionGroupName)
                ->Event("Set Grounded Collision Group Name", &FirstPersonControllerComponentRequests::SetGroundedCollisionGroup)
                ->Event("Get Air Time", &FirstPersonControllerComponentRequests::GetAirTime)
                ->Event("Get Gravity", &FirstPersonControllerComponentRequests::GetGravity)
                ->Event("Set Gravity", &FirstPersonControllerComponentRequests::SetGravity)
                ->Event("Get Previous Target Velocity Using World", &FirstPersonControllerComponentRequests::GetPrevTargetVelocityWorld)
                ->Event("Get Previous Target Velocity Using Character Heading", &FirstPersonControllerComponentRequests::GetPrevTargetVelocityHeading)
                ->Event("Get Velocity Close Tolerance", &FirstPersonControllerComponentRequests::GetVelocityCloseTolerance)
                ->Event("Set Velocity Close Tolerance", &FirstPersonControllerComponentRequests::SetVelocityCloseTolerance)
                ->Event("Tilt Vector2 Using XcrossY Direction", &FirstPersonControllerComponentRequests::TiltVectorXCrossY)
                ->Event("Get Velocity XcrossY Direction", &FirstPersonControllerComponentRequests::GetVelocityXCrossYDirection)
                ->Event("Set Velocity XcrossY Direction", &FirstPersonControllerComponentRequests::SetVelocityXCrossYDirection)
                ->Event("Get Velocity XcrossY Tracks Normal", &FirstPersonControllerComponentRequests::GetVelocityXCrossYTracksNormal)
                ->Event("Set Velocity XcrossY Tracks Normal", &FirstPersonControllerComponentRequests::SetVelocityXCrossYTracksNormal)
                ->Event("Get Speed Reduced When Moving Up Inclines", &FirstPersonControllerComponentRequests::GetSpeedReducedWhenMovingUpInclines)
                ->Event("Set Speed Reduced When Moving Up Inclines", &FirstPersonControllerComponentRequests::SetSpeedReducedWhenMovingUpInclines)
                ->Event("Get Velocity Z Positive Direction", &FirstPersonControllerComponentRequests::GetVelocityZPosDirection)
                ->Event("Set Velocity Z Positive Direction", &FirstPersonControllerComponentRequests::SetVelocityZPosDirection)
                ->Event("Get Sphere Casts Axis Direction", &FirstPersonControllerComponentRequests::GetSphereCastsAxisDirectionPose)
                ->Event("Set Sphere Casts Axis Direction", &FirstPersonControllerComponentRequests::SetSphereCastsAxisDirectionPose)
                ->Event("Get Vector Angles Between Vectors Radians", &FirstPersonControllerComponentRequests::GetVectorAnglesBetweenVectorsRadians)
                ->Event("Get Vector Angles Between Vectors Degrees", &FirstPersonControllerComponentRequests::GetVectorAnglesBetweenVectorsDegrees)
                ->Event("Create Ellipse Scaled Vector2", &FirstPersonControllerComponentRequests::CreateEllipseScaledVector)
                ->Event("Get Jump Held Gravity Factor", &FirstPersonControllerComponentRequests::GetJumpHeldGravityFactor)
                ->Event("Set Jump Held Gravity Factor", &FirstPersonControllerComponentRequests::SetJumpHeldGravityFactor)
                ->Event("Get Jump Falling Gravity Factor", &FirstPersonControllerComponentRequests::GetJumpFallingGravityFactor)
                ->Event("Set Jump Falling Gravity Factor", &FirstPersonControllerComponentRequests::SetJumpFallingGravityFactor)
                ->Event("Get Acceleration XY Jump Factor", &FirstPersonControllerComponentRequests::GetJumpAccelFactor)
                ->Event("Set Acceleration XY Jump Factor", &FirstPersonControllerComponentRequests::SetJumpAccelFactor)
                ->Event("Get Update Velocity XY When Ascending", &FirstPersonControllerComponentRequests::GetUpdateXYAscending)
                ->Event("Set Update Velocity XY When Ascending", &FirstPersonControllerComponentRequests::SetUpdateXYAscending)
                ->Event("Get Update Velocity XY When Descending", &FirstPersonControllerComponentRequests::GetUpdateXYDescending)
                ->Event("Set Update Velocity XY When Descending", &FirstPersonControllerComponentRequests::SetUpdateXYDescending)
                ->Event("Get Update Velocity XY Only Near Ground", &FirstPersonControllerComponentRequests::GetUpdateXYOnlyNearGround)
                ->Event("Set Update Velocity XY Only Near Ground", &FirstPersonControllerComponentRequests::SetUpdateXYOnlyNearGround)
                ->Event("Get Add Velocity For Physics Timestep Vs Tick", &FirstPersonControllerComponentRequests::GetAddVelocityForTimestepVsTick)
                ->Event("Set Add Velocity For Physics Timestep Vs Tick", &FirstPersonControllerComponentRequests::SetAddVelocityForTimestepVsTick)
                ->Event("Get Physics Timestep Scale Factor", &FirstPersonControllerComponentRequests::GetPhysicsTimestepScaleFactor)
                ->Event("Set Physics Timestep Scale Factor", &FirstPersonControllerComponentRequests::SetPhysicsTimestepScaleFactor)
                ->Event("Get Script Sets Target Velocity XY", &FirstPersonControllerComponentRequests::GetScriptSetsTargetVelocityXY)
                ->Event("Set Script Sets Target Velocity XY", &FirstPersonControllerComponentRequests::SetScriptSetsTargetVelocityXY)
                ->Event("Get Target XY Velocity", &FirstPersonControllerComponentRequests::GetTargetVelocityXY)
                ->Event("Set Target XY Velocity", &FirstPersonControllerComponentRequests::SetTargetVelocityXY)
                ->Event("Get Target XY Velocity Using World", &FirstPersonControllerComponentRequests::GetTargetVelocityXYWorld)
                ->Event("Get Corrected Velocity XY", &FirstPersonControllerComponentRequests::GetCorrectedVelocityXY)
                ->Event("Set Corrected Velocity XY", &FirstPersonControllerComponentRequests::SetCorrectedVelocityXY)
                ->Event("Get Corrected Velocity Z", &FirstPersonControllerComponentRequests::GetCorrectedVelocityZ)
                ->Event("Set Corrected Velocity Z", &FirstPersonControllerComponentRequests::SetCorrectedVelocityZ)
                ->Event("Get Apply Velocity XY", &FirstPersonControllerComponentRequests::GetApplyVelocityXY)
                ->Event("Set Apply Velocity XY", &FirstPersonControllerComponentRequests::SetApplyVelocityXY)
                ->Event("Get Add Velocity Using World", &FirstPersonControllerComponentRequests::GetAddVelocityWorld)
                ->Event("Set Add Velocity Using World", &FirstPersonControllerComponentRequests::SetAddVelocityWorld)
                ->Event("Get Add Velocity Using Character Heading", &FirstPersonControllerComponentRequests::GetAddVelocityHeading)
                ->Event("Set Add Velocity Using Character Heading", &FirstPersonControllerComponentRequests::SetAddVelocityHeading)
                ->Event("Get Apply Velocity Z", &FirstPersonControllerComponentRequests::GetApplyVelocityZ)
                ->Event("Set Apply Velocity Z", &FirstPersonControllerComponentRequests::SetApplyVelocityZ)
                ->Event("Get Enable Impulses", &FirstPersonControllerComponentRequests::GetEnableImpulses)
                ->Event("Set Enable Impulses", &FirstPersonControllerComponentRequests::SetEnableImpulses)
                ->Event("Get Use Friction For Deceleration", &FirstPersonControllerComponentRequests::GetImpulseDecelUsesFriction)
                ->Event("Set Use Friction For Deceleration", &FirstPersonControllerComponentRequests::SetImpulseDecelUsesFriction)
                ->Event("Get Linear Impulse", &FirstPersonControllerComponentRequests::GetLinearImpulse)
                ->Event("Set Linear Impulse", &FirstPersonControllerComponentRequests::SetLinearImpulse)
                ->Event("Apply Linear Impulse", &FirstPersonControllerComponentRequests::ApplyLinearImpulse)
                ->Event("Get Initial Velocity From Impulse", &FirstPersonControllerComponentRequests::GetInitVelocityFromImpulse)
                ->Event("Set Initial Velocity From Impulse", &FirstPersonControllerComponentRequests::SetInitVelocityFromImpulse)
                ->Event("Get Velocity From Impulse", &FirstPersonControllerComponentRequests::GetVelocityFromImpulse)
                ->Event("Set Velocity From Impulse", &FirstPersonControllerComponentRequests::SetVelocityFromImpulse)
                ->Event("Get Impulse Linear Damping", &FirstPersonControllerComponentRequests::GetImpulseLinearDamp)
                ->Event("Set Impulse Linear Damping", &FirstPersonControllerComponentRequests::SetImpulseLinearDamp)
                ->Event("Get Impulse Constant Deceleration", &FirstPersonControllerComponentRequests::GetImpulseConstantDecel)
                ->Event("Set Impulse Constant Deceleration", &FirstPersonControllerComponentRequests::SetImpulseConstantDecel)
                ->Event("Get Impulse Total Lerp Time", &FirstPersonControllerComponentRequests::GetImpulseTotalLerpTime)
                ->Event("Set Impulse Total Lerp Time", &FirstPersonControllerComponentRequests::SetImpulseTotalLerpTime)
                ->Event("Get Impulse Lerp Time", &FirstPersonControllerComponentRequests::GetImpulseLerpTime)
                ->Event("Set Impulse Lerp Time", &FirstPersonControllerComponentRequests::SetImpulseLerpTime)
                ->Event("Get Character Mass", &FirstPersonControllerComponentRequests::GetCharacterMass)
                ->Event("Set Character Mass", &FirstPersonControllerComponentRequests::SetCharacterMass)
                ->Event("Get Enable Character Hits", &FirstPersonControllerComponentRequests::GetEnableCharacterHits)
                ->Event("Set Enable Character Hits", &FirstPersonControllerComponentRequests::SetEnableCharacterHits)
                ->Event("Get Hit Radius Percentage Increase", &FirstPersonControllerComponentRequests::GetHitRadiusPercentageIncrease)
                ->Event("Set Hit Radius Percentage Increase", &FirstPersonControllerComponentRequests::SetHitRadiusPercentageIncrease)
                ->Event("Get Hit Height Percentage Increase", &FirstPersonControllerComponentRequests::GetHitHeightPercentageIncrease)
                ->Event("Set Hit Height Percentage Increase", &FirstPersonControllerComponentRequests::SetHitHeightPercentageIncrease)
                ->Event("Get Hit Extra Projection Percentage", &FirstPersonControllerComponentRequests::GetHitExtraProjectionPercentage)
                ->Event("Set Hit Extra Projection Percentage", &FirstPersonControllerComponentRequests::SetHitExtraProjectionPercentage)
                ->Event("Get Character Hit Collision Group Name", &FirstPersonControllerComponentRequests::GetCharacterHitCollisionGroupName)
                ->Event("Set Character Hit Collision Group By Name", &FirstPersonControllerComponentRequests::SetCharacterHitCollisionGroupByName)
                ->Event("Get Character Hit By", &FirstPersonControllerComponentRequests::GetCharacterHitBy)
                ->Event("Set Character Hit By", &FirstPersonControllerComponentRequests::SetCharacterHitBy)
                ->Event("Get Character Scene Query Hits", &FirstPersonControllerComponentRequests::GetCharacterSceneQueryHits)
                ->Event("Get Initial Jump Velocity", &FirstPersonControllerComponentRequests::GetJumpInitialVelocity)
                ->Event("Set Initial Jump Velocity", &FirstPersonControllerComponentRequests::SetJumpInitialVelocity)
                ->Event("Get Second Jump Initial Velocity", &FirstPersonControllerComponentRequests::GetJumpSecondInitialVelocity)
                ->Event("Set Second Jump Initial Velocity", &FirstPersonControllerComponentRequests::SetJumpSecondInitialVelocity)
                ->Event("Get Jump Requires Repress For Current Jump", &FirstPersonControllerComponentRequests::GetJumpReqRepress)
                ->Event("Set Jump Requires Repress For Current Jump", &FirstPersonControllerComponentRequests::SetJumpReqRepress)
                ->Event("Get Jump Held", &FirstPersonControllerComponentRequests::GetJumpHeld)
                ->Event("Set Jump Held", &FirstPersonControllerComponentRequests::SetJumpHeld)
                ->Event("Get Double Jump", &FirstPersonControllerComponentRequests::GetDoubleJump)
                ->Event("Set Double Jump", &FirstPersonControllerComponentRequests::SetDoubleJump)
                ->Event("Get Final Jump Performed", &FirstPersonControllerComponentRequests::GetFinalJumpPerformed)
                ->Event("Set Final Jump Performed", &FirstPersonControllerComponentRequests::SetFinalJumpPerformed)
                ->Event("Get Grounded Offset", &FirstPersonControllerComponentRequests::GetGroundedOffset)
                ->Event("Set Grounded Offset", &FirstPersonControllerComponentRequests::SetGroundedOffset)
                ->Event("Get Ground Close Offset", &FirstPersonControllerComponentRequests::GetGroundCloseOffset)
                ->Event("Set Ground Close Offset", &FirstPersonControllerComponentRequests::SetGroundCloseOffset)
                ->Event("Get Ground Close Coyote Time Offset", &FirstPersonControllerComponentRequests::GetGroundCloseCoyoteTimeOffset)
                ->Event("Set Ground Close Coyote Time Offset", &FirstPersonControllerComponentRequests::SetGroundCloseCoyoteTimeOffset)
                ->Event("Get Jump Hold Distance", &FirstPersonControllerComponentRequests::GetJumpHoldDistance)
                ->Event("Set Jump Hold Distance", &FirstPersonControllerComponentRequests::SetJumpHoldDistance)
                ->Event("Get Jump Head Hit Sphere Cast Offset", &FirstPersonControllerComponentRequests::GetJumpHeadSphereCastOffset)
                ->Event("Set Jump Head Hit Sphere Cast Offset", &FirstPersonControllerComponentRequests::SetJumpHeadSphereCastOffset)
                ->Event("Get Head Hit Sets Apogee", &FirstPersonControllerComponentRequests::GetHeadHitSetsApogee)
                ->Event("Set Head Hit Sets Apogee", &FirstPersonControllerComponentRequests::SetHeadHitSetsApogee)
                ->Event("Get Fell From Height", &FirstPersonControllerComponentRequests::GetFellFromHeight)
                ->Event("Set Fell From Height", &FirstPersonControllerComponentRequests::SetFellFromHeight)
                ->Event("Get Head Hit", &FirstPersonControllerComponentRequests::GetHeadHit)
                ->Event("Set Head Hit", &FirstPersonControllerComponentRequests::SetHeadHit)
                ->Event("Get Jump Head Ignore Dynamic Rigid Bodies", &FirstPersonControllerComponentRequests::GetJumpHeadIgnoreDynamicRigidBodies)
                ->Event("Set Jump Head Ignore Dynamic Rigid Bodies", &FirstPersonControllerComponentRequests::SetJumpHeadIgnoreDynamicRigidBodies)
                ->Event("Get Head Hit Collision Group Name", &FirstPersonControllerComponentRequests::GetHeadCollisionGroupName)
                ->Event("Set Head Hit Collision Group By Name", &FirstPersonControllerComponentRequests::SetHeadCollisionGroupByName)
                ->Event("Get Head Hit EntityIds", &FirstPersonControllerComponentRequests::GetHeadHitEntityIds)
                ->Event("Get Jump While Crouched", &FirstPersonControllerComponentRequests::GetJumpWhileCrouched)
                ->Event("Set Jump While Crouched", &FirstPersonControllerComponentRequests::SetJumpWhileCrouched)
                ->Event("Get Coyote Time", &FirstPersonControllerComponentRequests::GetCoyoteTime)
                ->Event("Set Coyote Time", &FirstPersonControllerComponentRequests::SetCoyoteTime)
                ->Event("Get Ungrounded Due To Jump", &FirstPersonControllerComponentRequests::GetUngroundedDueToJump)
                ->Event("Set Ungrounded Due To Jump", &FirstPersonControllerComponentRequests::SetUngroundedDueToJump)
                ->Event("Get Apply Gravity During Coyote", &FirstPersonControllerComponentRequests::GetApplyGravityDuringCoyoteTime)
                ->Event("Set Apply Gravity During Coyote", &FirstPersonControllerComponentRequests::SetApplyGravityDuringCoyoteTime)
                ->Event("Get Coyote Time Tracks Last Normal", &FirstPersonControllerComponentRequests::GetCoyoteTimeTracksLastNormal)
                ->Event("Set Coyote Time Tracks Last Normal", &FirstPersonControllerComponentRequests::SetCoyoteTimeTracksLastNormal)
                ->Event("Get Stand Prevented", &FirstPersonControllerComponentRequests::GetStandPrevented)
                ->Event("Set Stand Prevented", &FirstPersonControllerComponentRequests::SetStandPrevented)
                ->Event("Get Stand Ignore Dynamic Rigid Bodies", &FirstPersonControllerComponentRequests::GetStandIgnoreDynamicRigidBodies)
                ->Event("Set Stand Ignore Dynamic Rigid Bodies", &FirstPersonControllerComponentRequests::SetStandIgnoreDynamicRigidBodies)
                ->Event("Get Stand Collision Group Name", &FirstPersonControllerComponentRequests::GetStandCollisionGroupName)
                ->Event("Set Stand Collision Group By Name", &FirstPersonControllerComponentRequests::SetStandCollisionGroupByName)
                ->Event("Get Collision Group Name", &FirstPersonControllerComponentRequests::GetCollisionGroupName)
                ->Event("Get Stand Prevented EntityIds", &FirstPersonControllerComponentRequests::GetStandPreventedEntityIds)
                ->Event("Get Ground Sphere Casts Radius Percentage Increase", &FirstPersonControllerComponentRequests::GetGroundSphereCastsRadiusPercentageIncrease)
                ->Event("Set Ground Sphere Casts Radius Percentage Increase", &FirstPersonControllerComponentRequests::SetGroundSphereCastsRadiusPercentageIncrease)
                ->Event("Get Ground Close Coyote Time Radius Percentage Increase", &FirstPersonControllerComponentRequests::GetGroundCloseCoyoteTimeRadiusPercentageIncrease)
                ->Event("Set Ground Close Coyote Time Radius Percentage Increase", &FirstPersonControllerComponentRequests::SetGroundCloseCoyoteTimeRadiusPercentageIncrease)
                ->Event("Get Max Grounded Angle Degrees", &FirstPersonControllerComponentRequests::GetMaxGroundedAngleDegrees)
                ->Event("Set Max Grounded Angle Degrees", &FirstPersonControllerComponentRequests::SetMaxGroundedAngleDegrees)
                ->Event("Get Top Walk Speed", &FirstPersonControllerComponentRequests::GetTopWalkSpeed)
                ->Event("Set Top Walk Speed", &FirstPersonControllerComponentRequests::SetTopWalkSpeed)
                ->Event("Get Walk Acceleration", &FirstPersonControllerComponentRequests::GetWalkAcceleration)
                ->Event("Set Walk Acceleration", &FirstPersonControllerComponentRequests::SetWalkAcceleration)
                ->Event("Get Total Lerp Time", &FirstPersonControllerComponentRequests::GetTotalLerpTime)
                ->Event("Set Total Lerp Time", &FirstPersonControllerComponentRequests::SetTotalLerpTime)
                ->Event("Get Lerp Time", &FirstPersonControllerComponentRequests::GetLerpTime)
                ->Event("Set Lerp Time", &FirstPersonControllerComponentRequests::SetLerpTime)
                ->Event("Get Deceleration Factor", &FirstPersonControllerComponentRequests::GetDecelerationFactor)
                ->Event("Set Deceleration Factor", &FirstPersonControllerComponentRequests::SetDecelerationFactor)
                ->Event("Get Opposing Direction Deceleration Factor", &FirstPersonControllerComponentRequests::GetOpposingDecel)
                ->Event("Set Opposing Direction Deceleration Factor", &FirstPersonControllerComponentRequests::SetOpposingDecel)
                ->Event("Get Accelerating Via First Person Controller", &FirstPersonControllerComponentRequests::GetAccelerating)
                ->Event("Get Deceleration Factor Applied", &FirstPersonControllerComponentRequests::GetDecelerationFactorApplied)
                ->Event("Get Opposing Direction Deceleration Factor Applied", &FirstPersonControllerComponentRequests::GetOpposingDecelFactorApplied)
                ->Event("Get Instant Velocity Rotation", &FirstPersonControllerComponentRequests::GetInstantVelocityRotation)
                ->Event("Set Instant Velocity Rotation", &FirstPersonControllerComponentRequests::SetInstantVelocityRotation)
                ->Event("Get Velocity XY Ignores Obstacles", &FirstPersonControllerComponentRequests::GetVelocityXYIgnoresObstacles)
                ->Event("Set Velocity XY Ignores Obstacles", &FirstPersonControllerComponentRequests::SetVelocityXYIgnoresObstacles)
                ->Event("Get Gravity Ignores Obstacles", &FirstPersonControllerComponentRequests::GetGravityIgnoresObstacles)
                ->Event("Set Gravity Ignores Obstacles", &FirstPersonControllerComponentRequests::SetGravityIgnoresObstacles)
                ->Event("Get Positive Z Velocity Ignores Obstacles", &FirstPersonControllerComponentRequests::GetPosZIgnoresObstacles)
                ->Event("Set Positive Z Velocity Ignores Obstacles", &FirstPersonControllerComponentRequests::SetPosZIgnoresObstacles)
                ->Event("Get Jump Allowed When Gravity Prevented By Obstacle", &FirstPersonControllerComponentRequests::GetJumpAllowedWhenGravityPrevented)
                ->Event("Set Jump Allowed When Gravity Prevented By Obstacle", &FirstPersonControllerComponentRequests::SetJumpAllowedWhenGravityPrevented)
                ->Event("Get Ran Into Something", &FirstPersonControllerComponentRequests::GetVelocityXYObstructed)
                ->Event("Set Ran Into Something", &FirstPersonControllerComponentRequests::SetVelocityXYObstructed)
                ->Event("Get Gravity Prevented By Obstacle", &FirstPersonControllerComponentRequests::GetGravityPrevented)
                ->Event("Set Gravity Prevented By Obstacle", &FirstPersonControllerComponentRequests::SetGravityPrevented)
                ->Event("Get Sprint Scale Forward", &FirstPersonControllerComponentRequests::GetSprintScaleForward)
                ->Event("Set Sprint Scale Forward", &FirstPersonControllerComponentRequests::SetSprintScaleForward)
                ->Event("Get Sprint Scale Back", &FirstPersonControllerComponentRequests::GetSprintScaleBack)
                ->Event("Set Sprint Scale Back", &FirstPersonControllerComponentRequests::SetSprintScaleBack)
                ->Event("Get Sprint Scale Left", &FirstPersonControllerComponentRequests::GetSprintScaleLeft)
                ->Event("Set Sprint Scale Left", &FirstPersonControllerComponentRequests::SetSprintScaleLeft)
                ->Event("Get Sprint Scale Right", &FirstPersonControllerComponentRequests::GetSprintScaleRight)
                ->Event("Set Sprint Scale Right", &FirstPersonControllerComponentRequests::SetSprintScaleRight)
                ->Event("Get Sprint Acceleration Scale", &FirstPersonControllerComponentRequests::GetSprintAccelScale)
                ->Event("Set Sprint Acceleration Scale", &FirstPersonControllerComponentRequests::SetSprintAccelScale)
                ->Event("Get Sprint Accumulated Acceleration", &FirstPersonControllerComponentRequests::GetSprintAccumulatedAccel)
                ->Event("Set Sprint Accumulated Acceleration", &FirstPersonControllerComponentRequests::SetSprintAccumulatedAccel)
                ->Event("Get Sprint Max Time", &FirstPersonControllerComponentRequests::GetSprintMaxTime)
                ->Event("Set Sprint Max Time", &FirstPersonControllerComponentRequests::SetSprintMaxTime)
                ->Event("Get Sprint Held Time", &FirstPersonControllerComponentRequests::GetSprintHeldTime)
                ->Event("Set Sprint Held Time", &FirstPersonControllerComponentRequests::SetSprintHeldTime)
                ->Event("Get Sprint Regeneration Rate", &FirstPersonControllerComponentRequests::GetSprintRegenRate)
                ->Event("Set Sprint Regeneration Rate", &FirstPersonControllerComponentRequests::SetSprintRegenRate)
                ->Event("Get Stamina Percentage", &FirstPersonControllerComponentRequests::GetStaminaPercentage)
                ->Event("Set Stamina Percentage", &FirstPersonControllerComponentRequests::SetStaminaPercentage)
                ->Event("Get Stamina Increasing", &FirstPersonControllerComponentRequests::GetStaminaIncreasing)
                ->Event("Get Stamina Decreasing", &FirstPersonControllerComponentRequests::GetStaminaDecreasing)
                ->Event("Get Sprint Uses Stamina", &FirstPersonControllerComponentRequests::GetSprintUsesStamina)
                ->Event("Set Sprint Uses Stamina", &FirstPersonControllerComponentRequests::SetSprintUsesStamina)
                ->Event("Get Regenerate Stamina Automatically", &FirstPersonControllerComponentRequests::GetRegenerateStaminaAutomatically)
                ->Event("Set Regenerate Stamina Automatically", &FirstPersonControllerComponentRequests::SetRegenerateStaminaAutomatically)
                ->Event("Get Sprinting", &FirstPersonControllerComponentRequests::GetSprinting)
                ->Event("Get Sprint Total Cooldown Time", &FirstPersonControllerComponentRequests::GetSprintTotalCooldownTime)
                ->Event("Set Sprint Total Cooldown Time", &FirstPersonControllerComponentRequests::SetSprintTotalCooldownTime)
                ->Event("Get Sprint Cooldown Timer", &FirstPersonControllerComponentRequests::GetSprintCooldownTimer)
                ->Event("Set Sprint Cooldown Timer", &FirstPersonControllerComponentRequests::SetSprintCooldownTimer)
                ->Event("Get Sprint Pause Time", &FirstPersonControllerComponentRequests::GetSprintPauseTime)
                ->Event("Set Sprint Pause Time", &FirstPersonControllerComponentRequests::SetSprintPauseTime)
                ->Event("Get Sprint Pause", &FirstPersonControllerComponentRequests::GetSprintPause)
                ->Event("Set Sprint Pause", &FirstPersonControllerComponentRequests::SetSprintPause)
                ->Event("Get Sprint Backwards", &FirstPersonControllerComponentRequests::GetSprintBackwards)
                ->Event("Set Sprint Backwards", &FirstPersonControllerComponentRequests::SetSprintBackwards)
                ->Event("Get Sprint While Crouched", &FirstPersonControllerComponentRequests::GetSprintWhileCrouched)
                ->Event("Set Sprint While Crouched", &FirstPersonControllerComponentRequests::SetSprintWhileCrouched)
                ->Event("Get Sprint Via Script", &FirstPersonControllerComponentRequests::GetSprintViaScript)
                ->Event("Set Sprint Via Script", &FirstPersonControllerComponentRequests::SetSprintViaScript)
                ->Event("Get Enable Disable Sprint", &FirstPersonControllerComponentRequests::GetSprintEnableDisable)
                ->Event("Set Enable Disable Sprint", &FirstPersonControllerComponentRequests::SetSprintEnableDisable)
                ->Event("Get Crouching", &FirstPersonControllerComponentRequests::GetCrouching)
                ->Event("Set Crouching", &FirstPersonControllerComponentRequests::SetCrouching)
                ->Event("Get Crouched", &FirstPersonControllerComponentRequests::GetCrouched)
                ->Event("Get Standing", &FirstPersonControllerComponentRequests::GetStanding)
                ->Event("Get Crouched Percentage", &FirstPersonControllerComponentRequests::GetCrouchedPercentage)
                ->Event("Get Crouch Script Locked", &FirstPersonControllerComponentRequests::GetCrouchScriptLocked)
                ->Event("Set Crouch Script Locked", &FirstPersonControllerComponentRequests::SetCrouchScriptLocked)
                ->Event("Get Crouch Scale", &FirstPersonControllerComponentRequests::GetCrouchScale)
                ->Event("Set Crouch Scale", &FirstPersonControllerComponentRequests::SetCrouchScale)
                ->Event("Get Crouch Distance", &FirstPersonControllerComponentRequests::GetCrouchDistance)
                ->Event("Set Crouch Distance", &FirstPersonControllerComponentRequests::SetCrouchDistance)
                ->Event("Get Crouch Time", &FirstPersonControllerComponentRequests::GetCrouchTime)
                ->Event("Set Crouch Time", &FirstPersonControllerComponentRequests::SetCrouchTime)
                ->Event("Get Crouch Start Speed", &FirstPersonControllerComponentRequests::GetCrouchStartSpeed)
                ->Event("Set Crouch Start Speed", &FirstPersonControllerComponentRequests::SetCrouchStartSpeed)
                ->Event("Get Crouch End Speed", &FirstPersonControllerComponentRequests::GetCrouchEndSpeed)
                ->Event("Get Stand Time", &FirstPersonControllerComponentRequests::GetStandTime)
                ->Event("Set Stand Time", &FirstPersonControllerComponentRequests::SetStandTime)
                ->Event("Get Stand Start Speed", &FirstPersonControllerComponentRequests::GetStandStartSpeed)
                ->Event("Set Stand Start Speed", &FirstPersonControllerComponentRequests::SetStandStartSpeed)
                ->Event("Get Stand End Speed", &FirstPersonControllerComponentRequests::GetStandEndSpeed)
                ->Event("Get Crouching Down Move", &FirstPersonControllerComponentRequests::GetCrouchingDownMove)
                ->Event("Get Standing Up Move", &FirstPersonControllerComponentRequests::GetStandingUpMove)
                ->Event("Get Crouch Enable Toggle", &FirstPersonControllerComponentRequests::GetCrouchEnableToggle)
                ->Event("Set Crouch Enable Toggle", &FirstPersonControllerComponentRequests::SetCrouchEnableToggle)
                ->Event("Get Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchJumpCausesStanding)
                ->Event("Set Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchJumpCausesStanding)
                ->Event("Get Crouch Pend Jumps", &FirstPersonControllerComponentRequests::GetCrouchPendJumps)
                ->Event("Set Crouch Pend Jumps", &FirstPersonControllerComponentRequests::SetCrouchPendJumps)
                ->Event("Get Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchSprintCausesStanding)
                ->Event("Set Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchSprintCausesStanding)
                ->Event("Get Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::GetCrouchPriorityWhenSprintPressed)
                ->Event("Set Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::SetCrouchPriorityWhenSprintPressed)
                ->Event("Get Crouch When Not Grounded", &FirstPersonControllerComponentRequests::GetCrouchWhenNotGrounded)
                ->Event("Set Crouch When Not Grounded", &FirstPersonControllerComponentRequests::SetCrouchWhenNotGrounded)
                ->Event("Get Enable Camera And Character Rotation", &FirstPersonControllerComponentRequests::GetEnableCameraCharacterRotation)
                ->Event("Set Enable Camera And Character Rotation", &FirstPersonControllerComponentRequests::SetEnableCameraCharacterRotation)
                ->Event("Get Character And Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::GetCharacterAndCameraYawSensitivity)
                ->Event("Set Character And Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::SetCharacterAndCameraYawSensitivity)
                ->Event("Get Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::GetCameraPitchSensitivity)
                ->Event("Set Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::SetCameraPitchSensitivity)
                ->Event("Get Camera Pitch Max Angle Radians", &FirstPersonControllerComponentRequests::GetCameraPitchMaxAngleRadians)
                ->Event("Set Camera Pitch Max Angle Radians", &FirstPersonControllerComponentRequests::SetCameraPitchMaxAngleRadians)
                ->Event("Get Camera Pitch Max Angle Degrees", &FirstPersonControllerComponentRequests::GetCameraPitchMaxAngleDegrees)
                ->Event("Set Camera Pitch Max Angle Degrees", &FirstPersonControllerComponentRequests::SetCameraPitchMaxAngleDegrees)
                ->Event("Get Camera Pitch Min Angle Radians", &FirstPersonControllerComponentRequests::GetCameraPitchMinAngleRadians)
                ->Event("Set Camera Pitch Min Angle Radians", &FirstPersonControllerComponentRequests::SetCameraPitchMinAngleRadians)
                ->Event("Get Camera Pitch Min Angle Degrees", &FirstPersonControllerComponentRequests::GetCameraPitchMinAngleDegrees)
                ->Event("Set Camera Pitch Min Angle Degrees", &FirstPersonControllerComponentRequests::SetCameraPitchMinAngleDegrees)
                ->Event("Get Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::GetCameraRotationDampFactor)
                ->Event("Set Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::SetCameraRotationDampFactor)
                ->Event("Get Character Transform Interface Pointer", &FirstPersonControllerComponentRequests::GetCharacterTransformInterfacePtr)
                ->Event("Get Character Transform", &FirstPersonControllerComponentRequests::GetCharacterTransform)
                ->Event("Set Character Transform", &FirstPersonControllerComponentRequests::SetCharacterTransform)
                ->Event("Get Character World Translation", &FirstPersonControllerComponentRequests::GetCharacterWorldTranslation)
                ->Event("Set Character World Translation", &FirstPersonControllerComponentRequests::SetCharacterWorldTranslation)
                ->Event("Update Character And Camera Yaw", &FirstPersonControllerComponentRequests::UpdateCharacterAndCameraYaw)
                ->Event("Update Camera Pitch", &FirstPersonControllerComponentRequests::UpdateCameraPitch)
                ->Event("Get Character Heading", &FirstPersonControllerComponentRequests::GetHeading)
                ->Event("Set Character Heading For Tick", &FirstPersonControllerComponentRequests::SetHeadingForTick)
                ->Event("Get Camera Pitch", &FirstPersonControllerComponentRequests::GetPitch);

            bc->Class<FirstPersonControllerComponent>()->RequestBus("FirstPersonControllerComponentRequestBus");
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
        if(m_addVelocityForTimestepVsTick)
        {
            Physics::DefaultWorldBus::BroadcastResult(m_attachedSceneHandle, &Physics::DefaultWorldRequests::GetDefaultSceneHandle);
            if(m_attachedSceneHandle == AzPhysics::InvalidSceneHandle)
            {
                AZ_Error("First Person Controller Component", false, "Failed to retrieve default scene.");
                return;
            }

            m_sceneSimulationStartHandler = AzPhysics::SceneEvents::OnSceneSimulationStartHandler(
                [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, float fixedDeltaTime)
                {
                    OnSceneSimulationStart(fixedDeltaTime);
                }, aznumeric_cast<int32_t>(AzPhysics::SceneEvents::PhysicsStartFinishSimulationPriority::Physics));

            m_sceneSimulationFinishHandler = AzPhysics::SceneEvents::OnSceneSimulationFinishHandler(
                [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, [[maybe_unused]] float fixedDeltaTime)
                {
                    OnSceneSimulationFinish(fixedDeltaTime);
                }, aznumeric_cast<int32_t>(AzPhysics::SceneEvents::PhysicsStartFinishSimulationPriority::Physics));

            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            if(sceneInterface != nullptr)
            {
                sceneInterface->RegisterSceneSimulationStartHandler(m_attachedSceneHandle, m_sceneSimulationStartHandler);
                sceneInterface->RegisterSceneSimulationFinishHandler(m_attachedSceneHandle, m_sceneSimulationFinishHandler);
            }
        }

        AzFramework::NativeWindowHandle windowHandle = nullptr;
        windowHandle = AZ::RPI::ViewportContextRequests::Get()->GetDefaultViewportContext()->GetWindowHandle();
        if(windowHandle)
        {
            float refreshRate = 60.f;
            AzFramework::WindowRequestBus::EventResult(refreshRate, windowHandle, &AzFramework::WindowRequestBus::Events::GetDisplayRefreshRate);

            const AzPhysics::SystemConfiguration* config = AZ::Interface<AzPhysics::SystemInterface>::Get()->GetConfiguration();

            // Disable camera smooth follow if the physics timestep is less than or equal to the refresh time
            if(config->m_fixedTimestep <= 1.f/refreshRate)
                m_cameraSmoothFollow = false;
        }

        Physics::CharacterNotificationBus::Handler::BusConnect(GetEntityId());
        Physics::CollisionRequestBus::BroadcastResult(
            m_groundedCollisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, m_groundedCollisionGroupId);
        Physics::CollisionRequestBus::BroadcastResult(
            m_headCollisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, m_headCollisionGroupId);
        Physics::CollisionRequestBus::BroadcastResult(
            m_characterHitCollisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, m_characterHitCollisionGroupId);

        UpdateJumpMaxHoldTime();

        AssignConnectInputEvents();

        Camera::CameraNotificationBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();

        InputChannelEventListener::Connect();
        AZStd::shared_ptr<AzFramework::InputChannelEventFilterInclusionList> filter;
        AzFramework::InputChannelEventListener::SetFilter(filter);

        FirstPersonControllerComponentRequestBus::Handler::BusConnect(GetEntityId());

        if(m_cameraEntityId.IsValid())
        {
            AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
        }

        // Debug log to verify m_cameraSmoothFollow value at activation
        //AZ_Printf("First Person Controller Component", "Activate: m_cameraSmoothFollow=%s",
        //    m_cameraSmoothFollow ? "true" : "false");
    }

    void FirstPersonControllerComponent::OnCharacterActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        Physics::CharacterNotificationBus::Handler::BusDisconnect();

        // Obtain the PhysX Character Controller's capsule height and radius
        // and use those dimensions for the ground detection shapecast capsule
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetHeight);
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleRadius, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetRadius);
        Physics::CharacterRequestBus::EventResult(m_maxGroundedAngleDegrees, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetSlopeLimitDegrees);

        m_capsuleCurrentHeight = m_capsuleHeight;

        if(m_crouchDistance > m_capsuleHeight - 2.f*m_capsuleRadius)
            m_crouchDistance = m_capsuleHeight - 2.f*m_capsuleRadius;

        // Calculate the crouch down final velocity
        m_crouchDownFinalVelocity = 2 * m_crouchDistance / m_crouchTime - m_crouchDownInitVelocity;
        // Check to make sure that the crouching is decelerated, if not then force a constant velocity to meet m_crouchTime
        if(m_crouchDownFinalVelocity > m_crouchDownInitVelocity)
        {
            m_crouchDownInitVelocity = m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = m_crouchDownInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed set to a value that's too slow to reach the crouched position within the crouch time, setting the crouch speed to the crouch distance divide by the crouch time instead (%.3f m/s).", m_crouchDownInitVelocity);
        }
        // Check to make sure that the final crouching velocity isn't negative, and fix it if it is
        else if(m_crouchDownFinalVelocity < 0)
        {
            m_crouchDownInitVelocity = 2.f * m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed set to a value that's too fast to reach the crouching position at crouch time, setting the start crouch speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchDownInitVelocity);
        }

        // Calculate the crouch up (stand) final velocity
        m_crouchUpFinalVelocity = (2 * m_crouchDistance) / m_standTime - m_crouchUpInitVelocity;
        // Check to make sure that the standing is decelerated, if not then force a constant velocity to meet m_standTime
        if(m_crouchUpFinalVelocity > m_crouchUpInitVelocity)
        {
            m_crouchUpInitVelocity = m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = m_crouchUpInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Stand start speed set to a value that's too slow to reach the standing position within the stand time, setting the stand speed to the crouch distance divide by the stand time instead (%.3f m/s).", m_crouchUpInitVelocity);
        }
        // Check to make sure that the final standing velocity isn't negative, and fix it if it is
        else if(m_crouchUpFinalVelocity < 0)
        {
            m_crouchUpInitVelocity = 2.f * m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Stand start speed set to a value that's too fast to reach the standing position at stand time, setting the start stand speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchUpInitVelocity);
        }

        // Set the max grounded angle to be slightly greater than the PhysX Character Controller's
        // maximum slope angle value in the editor
        m_maxGroundedAngleDegrees += 0.01f;

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::SetSlopeLimitDegrees, m_maxGroundedAngleDegrees);

        // If using coyote time with gravity not applied then check if the character isn't grounded when activated
        // and if they're not then set m_ungroundedDueToJump to true to prevent the character from floating when activated
        if(m_coyoteTime > 0.f && !m_applyGravityDuringCoyoteTime)
        {
            CheckGrounded(0.f);
            if(!m_grounded)
                m_ungroundedDueToJump = true;
        }

        // Set the sprint pause time based on whether the cooldown time or the max consecutive sprint time is longer
        // This number can be altered using the RequestBus
        m_sprintPauseTime = (m_sprintTotalCooldownTime > m_sprintMaxTime) ? 0.f : 0.1f * m_sprintTotalCooldownTime;

        //AZ_Printf("First Person Controller Component", "m_capsuleHeight = %.10f", m_capsuleHeight);
        //AZ_Printf("First Person Controller Component", "m_capsuleRadius = %.10f", m_capsuleRadius);
        //AZ_Printf("First Person Controller Component", "m_maxGroundedAngleDegrees = %.10f", m_maxGroundedAngleDegrees);
    }

    void FirstPersonControllerComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        InputChannelEventListener::Disconnect();
        FirstPersonControllerComponentRequestBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
        AZ::EntityBus::Handler::BusDisconnect();

        if(m_addVelocityForTimestepVsTick)
        {
            m_attachedSceneHandle = AzPhysics::InvalidSceneHandle;
            m_sceneSimulationStartHandler.Disconnect();
            m_sceneSimulationFinishHandler.Disconnect();
        }

        m_activeCameraEntity = nullptr;
    }

    void FirstPersonControllerComponent::OnEntityActivated(const AZ::EntityId& entityId)
    {
        if(entityId == m_cameraEntityId)
        {
            AZ::EntityBus::Handler::BusDisconnect();
            m_activeCameraEntity = GetEntityPtr(entityId);
            if(m_activeCameraEntity)
            {
                // Calculate initial eye height based on the difference between the
                // camera and character entities' translations, projected along the pose axis.
                AZ::Vector3 characterWorldTranslation;
                AZ::TransformBus::EventResult(characterWorldTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
                AZ::Vector3 cameraWorldTranslation;
                AZ::TransformBus::EventResult(cameraWorldTranslation, m_cameraEntityId, &AZ::TransformBus::Events::GetWorldTranslation);
                AZ::Vector3 diff = cameraWorldTranslation - characterWorldTranslation;
                m_eyeHeight = diff.Dot(m_sphereCastsAxisDirectionPose.GetNormalized());
                InitializeCameraTranslation();
                Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);
                //AZ_Printf("First Person Controller Component", "Camera entity %s activated and set as active view.",
                //    m_activeCameraEntity->GetName().empty() ? m_cameraEntityId.ToString().c_str() : m_activeCameraEntity->GetName().c_str());
            }
            else
            {
                AZ_Warning("FirstPersonControllerComponent", false,
                    "Camera entity ID %s is invalid.", m_cameraEntityId.ToString().c_str());
                m_cameraEntityId = AZ::EntityId();
            }
        }
    }

    void FirstPersonControllerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        required.push_back(AZ_CRC_CE("PhysicsWorldBodyService"));
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    void FirstPersonControllerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void FirstPersonControllerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void FirstPersonControllerComponent::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if(m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("First Person Controller Component", false, "Number of input IDs not equal to number of input names!");
        }
        else
        {
            for(auto& it_event: m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
    }

    void FirstPersonControllerComponent::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        if(*inputId == m_sprintEventId)
        {
            if(m_grounded)
            {
                m_sprintValue = value;
                m_sprintAccelValue = value * m_sprintAccelScale;
            }
            else
                m_sprintValue = 0.f;
        }

        for(auto& it_event: m_controlMap)
        {
            if(*inputId == *(it_event.first) && !(*(it_event.first) == m_sprintEventId))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                //AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        for(auto& it_event: m_controlMap)
        {
            if(*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                //AZ_Printf("Released", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        if(*inputId == m_rotateYawEventId)
        {
            m_yawValue = value;
        }
        else if(*inputId == m_rotatePitchEventId)
        {
            m_pitchValue = value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if(*inputId == m_sprintEventId)
        {
            if(m_grounded || m_sprintPrevValue != 1.f)
            {
                m_sprintValue = value;
                m_sprintAccelValue = value * m_sprintAccelScale;
            }
            else
                m_sprintValue = 0.f;
        }
    }

    bool FirstPersonControllerComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
    {
        const AzFramework::InputDeviceId& deviceId = inputChannel.GetInputDevice().GetInputDeviceId();

        // TODO: Implement gamepad support
        //AZ_Printf("First Person Controller Component", "OnInputChannelEventFiltered");
        if(AzFramework::InputDeviceGamepad::IsGamepadDevice(deviceId))
            OnGamepadEvent(inputChannel);

        return false;
    }

    void FirstPersonControllerComponent::OnGamepadEvent(const AzFramework::InputChannel& inputChannel)
    {
        // TODO: Implement gamepad support
        const AzFramework::InputChannelId& channelId = inputChannel.GetInputChannelId();

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LR)
        {
            m_rightValue = inputChannel.GetValue();
            m_leftValue = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LL)
        {
            m_rightValue = 0.f;
            m_leftValue = inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LU)
        {
            m_forwardValue = inputChannel.GetValue();
            m_backValue = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LD)
        {
            m_forwardValue = 0.f;
            m_backValue = inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RX)
        {
            m_cameraRotationAngles[2] = -1.f*inputChannel.GetValue() * m_yawSensitivity;
            m_rotatingYawViaScriptGamepad = true;
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RY)
        {
            m_cameraRotationAngles[0] = inputChannel.GetValue() * m_pitchSensitivity;
            m_rotatingPitchViaScriptGamepad = true;
        }
    }

    void FirstPersonControllerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), false);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonControllerComponent::OnSceneSimulationStart(float physicsTimestep)
    {
        FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnPhysicsTimestepStart, physicsTimestep * m_physicsTimestepScaleFactor);
        ProcessInput(((physicsTimestep * m_physicsTimestepScaleFactor + m_prevTimestep) / 2.f), true);
    }

    void FirstPersonControllerComponent::OnSceneSimulationFinish(float physicsTimestep)
    {
        CaptureCharacterEyeTranslation();
        FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnPhysicsTimestepFinish, physicsTimestep * m_physicsTimestepScaleFactor);
        m_prevTimestep = physicsTimestep * m_physicsTimestepScaleFactor;
    }

    void FirstPersonControllerComponent::OnCameraAdded(const AZ::EntityId& cameraId)
    {
        if(!m_cameraEntityId.IsValid())
        {
            m_cameraEntityId = cameraId;
            m_activeCameraEntity = GetEntityPtr(cameraId);
            if(m_activeCameraEntity)
            {
                InitializeCameraTranslation();
                Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);
                //AZ_Printf("First Person Controller Component", "Default camera %s assigned and set as active view.",
                //    m_activeCameraEntity->GetName().empty() ? m_cameraEntityId.ToString().c_str() : m_activeCameraEntity->GetName().c_str());
            }
            else
            {
                AZ_Warning("FirstPersonControllerComponent", false,
                    "Default camera ID %s from CameraNotificationBus is invalid.", cameraId.ToString().c_str());
                m_cameraEntityId = AZ::EntityId();
            }
        }
    }

    AZ::Entity* FirstPersonControllerComponent::GetEntityPtr(AZ::EntityId pointer) const
    {
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca ? ca->FindEntity(pointer) : nullptr;
    }

    void FirstPersonControllerComponent::InitializeCameraTranslation()
    {
        if(!m_activeCameraEntity)
        {
            return;
        }

        // Set target translation for smooth follow
        AZ::Vector3 characterWorldTranslation;
        AZ::TransformBus::EventResult(characterWorldTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
        m_currentCharacterEyeTranslation = characterWorldTranslation + m_sphereCastsAxisDirectionPose * (m_eyeHeight + m_cameraLocalZTravelDistance);
        m_prevCharacterEyeTranslation = m_currentCharacterEyeTranslation ;

        // Set initial world translation for smooth following
        if(m_addVelocityForTimestepVsTick && m_cameraSmoothFollow)
            AZ::TransformBus::Event(m_cameraEntityId, &AZ::TransformBus::Events::SetWorldTranslation, m_currentCharacterEyeTranslation);
    }

    void FirstPersonControllerComponent::LerpCameraToCharacter(float deltaTime)
    {
        if(!m_activeCameraEntity || !m_addVelocityForTimestepVsTick || !m_cameraSmoothFollow)
            return;

        // Update time accumulator
        m_physicsTimeAccumulator += deltaTime;

        // Calculate interpolation factor
        float const alpha = AZ::GetClamp(m_physicsTimeAccumulator / m_prevTimestep, 0.f, 1.f);

        // Interpolate translation
        AZ::Vector3 const interpolatedCameraTranslation = m_prevCharacterEyeTranslation.Lerp(m_currentCharacterEyeTranslation, alpha);
        AZ::TransformBus::Event(m_cameraEntityId, &AZ::TransformBus::Events::SetWorldTranslation, interpolatedCameraTranslation);

        // Reset accumulator if it exceeds physics timestep
        if(m_physicsTimeAccumulator >= m_prevTimestep)
            m_physicsTimeAccumulator -= m_prevTimestep;
    }

    // Helper function to check if camera is a child of the character
    bool FirstPersonControllerComponent::IsCameraChildOfCharacter()
    {
        if(!m_obtainedChildIds)
        {
            ReacquireChildEntityIds();
            m_obtainedChildIds = true;
        }
        for(const AZ::EntityId& childId : m_children)
        {
            if(childId == m_cameraEntityId)
            {
                return true;
            }
        }
        return false;
    }

    void FirstPersonControllerComponent::ResetCameraToCharacter()
    {
        AZ::TransformBus::Event(m_cameraEntityId,
            &AZ::TransformBus::Events::SetWorldTranslation, m_currentCharacterEyeTranslation);
    }

    void FirstPersonControllerComponent::CaptureCharacterEyeTranslation()
    {
        if(m_addVelocityForTimestepVsTick && m_cameraSmoothFollow)
        {
            // Capture character's translation after each physics simulation step. This ensures camera lerp uses
            // the most recent post-simulation transform.
            m_prevCharacterEyeTranslation = m_currentCharacterEyeTranslation;
            AZ::TransformBus::EventResult(m_currentCharacterEyeTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            m_currentCharacterEyeTranslation += m_sphereCastsAxisDirectionPose * (m_eyeHeight + m_cameraLocalZTravelDistance);
            m_physicsTimeAccumulator = 0.f;
        }
    }

    void FirstPersonControllerComponent::SmoothRotation()
    {
        // Multiply by -1 since moving the mouse to the right produces a positive value
        // but a positive rotation about Z is counterclockwise
        if(!m_rotatingYawViaScriptGamepad)
            m_cameraRotationAngles[2] = -1.f * m_yawValue * m_yawSensitivity;
        else
            m_rotatingYawViaScriptGamepad = false;

        // Multiply by -1 since moving the mouse up produces a negative value from the input bus
        if(!m_rotatingPitchViaScriptGamepad)
            m_cameraRotationAngles[0] = -1.f * m_pitchValue * m_pitchSensitivity;
        else
            m_rotatingPitchViaScriptGamepad = false;

        const AZ::Quaternion targetLookRotationDelta = AZ::Quaternion::CreateFromEulerAnglesRadians(
            AZ::Vector3::CreateFromFloat3(m_cameraRotationAngles));

        if(m_rotationDamp*0.01f <= 1.f)
            m_newLookRotationDelta = m_newLookRotationDelta.Slerp(targetLookRotationDelta, m_rotationDamp*0.01f);
        else
            m_newLookRotationDelta = targetLookRotationDelta;
    }

    void FirstPersonControllerComponent::UpdateRotation()
    {
        if(!m_enableCameraCharacterRotation)
            return;

        SmoothRotation();
        const AZ::Vector3 newLookRotationDelta = m_newLookRotationDelta.GetEulerRadians();

        // Apply yaw to player character
        AZ::TransformInterface* characterTransform = GetEntity()->GetTransform();
        characterTransform->RotateAroundLocalZ(newLookRotationDelta.GetZ());

        m_activeCameraEntity = GetActiveCameraEntityPtr();
        if(m_activeCameraEntity)
        {
            m_cameraRotationTransform = m_activeCameraEntity->GetTransform();

            if(IsCameraChildOfCharacter())
            {
                // Apply pitch to camera's local rotation, yaw follows the parent character entity
                m_cameraRotationTransform->SetLocalRotation(AZ::Vector3(
                    AZ::GetClamp(m_cameraRotationTransform->GetLocalRotation().GetX() + newLookRotationDelta.GetX(),
                        m_cameraPitchMinAngle, m_cameraPitchMaxAngle),
                    m_cameraRotationTransform->GetLocalRotation().GetY(),
                    m_cameraRotationTransform->GetLocalRotation().GetZ()));
            }
            else if(m_addVelocityForTimestepVsTick && m_cameraSmoothFollow)
            {
                // Follow the character's rotation and apply a delta to the pitch
                m_cameraYaw += newLookRotationDelta.GetZ();

                const float characterPitch = characterTransform->GetLocalRotation().GetX();
                const float pitchDelta = newLookRotationDelta.GetX() + (characterPitch - m_prevCharacterPitch);
                const float angleFromZ = AZ::Vector3::CreateAxisZ().Angle(m_sphereCastsAxisDirectionPose);
                m_prevCharacterPitch = characterPitch;
                m_cameraPitch = AZ::GetClamp(m_cameraPitch + pitchDelta, m_cameraPitchMinAngle - angleFromZ, m_cameraPitchMaxAngle - angleFromZ);

                const float characterRoll = characterTransform->GetLocalRotation().GetY();
                const float rollDelta = (characterRoll - m_prevCharacterRoll);
                m_prevCharacterRoll = characterRoll;
                m_cameraRoll += rollDelta;

                const AZ::Quaternion yawRotation = AZ::Quaternion::CreateFromAxisAngle(m_sphereCastsAxisDirectionPose, m_cameraYaw);
                const AZ::Quaternion pitchRotation = AZ::Quaternion::CreateRotationX(m_cameraPitch);
                const AZ::Quaternion rollRotation = AZ::Quaternion::CreateRotationY(m_cameraRoll);
                m_cameraRotationTransform->SetLocalRotationQuaternion(yawRotation * pitchRotation * rollRotation);
            }
            else
            {
                // Update yaw and pitch for camera's local rotation
                m_cameraYaw += newLookRotationDelta.GetZ();
                const AZ::Quaternion yawRotation = AZ::Quaternion::CreateRotationZ(m_cameraYaw);
                m_cameraPitch = AZ::GetClamp(m_cameraPitch + newLookRotationDelta.GetX(), m_cameraPitchMinAngle, m_cameraPitchMaxAngle);
                const AZ::Quaternion pitchRotation = AZ::Quaternion::CreateRotationX(m_cameraPitch);
                m_cameraRotationTransform->SetLocalRotationQuaternion(yawRotation * pitchRotation);
            }
        }

        // Update heading and pitch
        if(!m_scriptSetCurrentHeadingTick)
            m_currentHeading = characterTransform->GetWorldRotationQuaternion().GetEulerRadians().GetZ();
        else
            m_scriptSetCurrentHeadingTick = false;

        if(m_activeCameraEntity)
            m_currentPitch = m_activeCameraEntity->GetTransform()->GetWorldRotationQuaternion().GetEulerRadians().GetX();
    }

    // Here target velocity is with respect to the character's frame of reference when m_instantVelocityRotation == true
    // and it's with respect to the world when m_instantVelocityRotation == false
    AZ::Vector2 FirstPersonControllerComponent::LerpVelocityXY(const AZ::Vector2& targetVelocityXY, const float& deltaTime)
    {
        m_totalLerpTime = m_prevApplyVelocityXY.GetDistance(targetVelocityXY)/m_accel;

        if(m_totalLerpTime == 0.f)
        {
            m_accelerating = false;
            m_decelerationFactorApplied = false;
            m_opposingDecelFactorApplied = false;
            return m_prevApplyVelocityXY;
        }

        // Apply the sprint factor to the acceleration (dt) based on the sprint having been (recently) pressed
        const float lastLerpTime = m_lerpTime;

        float lerpDeltaTime = (m_sprintAccumulatedAccel > 0.f || m_sprintVelocityAdjust != 1.f) ? deltaTime * m_sprintAccelAdjust : deltaTime;
        if(m_sprintAccelValue < 1.f && m_sprintAccumulatedAccel > 0.f)
            lerpDeltaTime = deltaTime * m_sprintAccelAdjust;

        lerpDeltaTime *= m_grounded ? 1.f : m_jumpAccelFactor;

        m_lerpTime += lerpDeltaTime * 0.5f;

        if(m_lerpTime >= m_totalLerpTime)
            m_lerpTime = m_totalLerpTime;

        // Lerp the velocity from the last applied velocity to the target velocity
        AZ::Vector2 newVelocityXY = m_prevApplyVelocityXY.Lerp(targetVelocityXY, m_lerpTime / m_totalLerpTime);

        if(m_lerpTime != m_totalLerpTime)
            m_lerpTime += lerpDeltaTime * 0.5f;

        // Decelerate at a different rate than the acceleration
        if(newVelocityXY.GetLength() < m_applyVelocityXY.GetLength())
        {
            m_accelerating = false;
            m_decelerationFactorApplied = true;
            // Get the current velocity vector with respect to the character's local coordinate system
            const AZ::Vector2 applyVelocityHeading = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocityXY)));

            // Compare the direction of the current velocity vector against the desired direction
            // and if it's greater than 90 degrees then decelerate even more
            if(targetVelocityXY.GetLength() != 0.f
                && m_instantVelocityRotation ?
                    (abs(applyVelocityHeading.AngleSafe(targetVelocityXY)) > AZ::Constants::HalfPi)
                    : (abs(m_applyVelocityXY.AngleSafe(targetVelocityXY)) > AZ::Constants::HalfPi))
            {
                m_opposingDecelFactorApplied = true;
                m_decelerationFactorApplied = false;
                // Compute the deceleration factor based on the magnitude of the target velocity
                float greatestScale = m_forwardScale;
                for(float scale: {m_forwardScale, m_backScale, m_leftScale, m_rightScale})
                    if(greatestScale < abs(scale))
                        greatestScale = abs(scale);

                AZ::Vector2 targetVelocityXYLocal = targetVelocityXY;
                if(!m_instantVelocityRotation)
                    targetVelocityXYLocal = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(targetVelocityXY)));

                if(m_standing || m_sprintWhileCrouched)
                    m_decelerationFactor = (m_decel + (m_opposingDecel - m_decel) * targetVelocityXYLocal.GetLength() / (m_speed * (1.f + (m_sprintVelocityAdjust-1.f)) * greatestScale));
                else
                    m_decelerationFactor = (m_decel + (m_opposingDecel - m_decel) * targetVelocityXYLocal.GetLength() / (m_speed * m_crouchScale * greatestScale));
            }
            else
            {
                m_decelerationFactor = m_decel;
                m_opposingDecelFactorApplied = false;
            }

            // Use the deceleration factor to get the lerp time closer to the total lerp time at a faster rate
            m_lerpTime = lastLerpTime + lerpDeltaTime * m_decelerationFactor * 0.5f;

            if(m_lerpTime >= m_totalLerpTime)
                m_lerpTime = m_totalLerpTime;

            AZ::Vector2 newVelocityXYDecel = m_prevApplyVelocityXY.Lerp(targetVelocityXY, m_lerpTime / m_totalLerpTime);
            if(newVelocityXYDecel.GetLength() < m_applyVelocityXY.GetLength())
                newVelocityXY = newVelocityXYDecel;

            if(m_lerpTime != m_totalLerpTime)
                m_lerpTime += lerpDeltaTime * m_decelerationFactor * 0.5f;
        }
        else
        {
            m_accelerating = true;
            m_decelerationFactorApplied = false;
            m_opposingDecelFactorApplied = false;
        }

        if(!AZ::IsClose(m_sprintAccelAdjust, 1.f))
        {
            if(!AZ::IsClose(m_sprintVelocityAdjust, 1.f) || (newVelocityXY.GetLength() < m_applyVelocityXY.GetLength()))
                m_sprintAccumulatedAccel += (newVelocityXY.GetLength() - m_applyVelocityXY.GetLength());
            else
                m_sprintAccumulatedAccel = 0.f;

            if(m_sprintAccumulatedAccel < 0.f)
                m_sprintAccumulatedAccel = 0.f;
        }
        else
            m_sprintAccumulatedAccel = 0.f;

        if(m_applyVelocityXY == AZ::Vector2::CreateZero())
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStartedMoving);

        if(newVelocityXY == targetVelocityXY)
        {
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnTargetVelocityReached);

            const bool vXCrossYPos = (m_velocityXCrossYDirection.GetZ() >= 0.f);
            if(newVelocityXY.GetLength() == 0.f)
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStopped);
            else if(vXCrossYPos && (AZ::IsClose(newVelocityXY.GetLength(), m_speed * CreateEllipseScaledVector(newVelocityXY.GetNormalized(), m_forwardScale, m_backScale, m_leftScale, m_rightScale).GetLength())))
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnTopWalkSpeedReached);
            else if(!vXCrossYPos && (AZ::IsClose(newVelocityXY.GetLength(), m_speed * CreateEllipseScaledVector((-newVelocityXY).GetNormalized(), m_forwardScale, m_backScale, m_leftScale, m_rightScale).GetLength())))
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnTopWalkSpeedReached);
            else if(vXCrossYPos && (AZ::IsClose(newVelocityXY.GetLength(), m_speed * CreateEllipseScaledVector(newVelocityXY.GetNormalized(), m_sprintScaleForward*m_forwardScale, m_sprintScaleBack*m_backScale, m_sprintScaleLeft*m_leftScale, m_sprintScaleRight*m_rightScale).GetLength())))
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnTopSprintSpeedReached);
            else if(!vXCrossYPos && (AZ::IsClose(newVelocityXY.GetLength(), m_speed * CreateEllipseScaledVector((-newVelocityXY).GetNormalized(), m_sprintScaleForward*m_forwardScale, m_sprintScaleBack*m_backScale, m_sprintScaleLeft*m_leftScale, m_sprintScaleRight*m_rightScale).GetLength())))
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnTopSprintSpeedReached);
        }

        return newVelocityXY;
    }

    AZ::Vector2 FirstPersonControllerComponent::CreateEllipseScaledVector(const AZ::Vector2& unscaledVector, float forwardScale, float backScale, float leftScale, float rightScale)
    {
        AZ::Vector2 scaledVector = AZ::Vector2::CreateZero();

        if(unscaledVector.IsZero())
            return AZ::Vector2::CreateZero();

        // If the input vector isn't normalized then scale the scale factors accordingly
        if(!unscaledVector.IsNormalized())
        {
            forwardScale *= unscaledVector.GetLength();
            backScale *= unscaledVector.GetLength();
            leftScale *= unscaledVector.GetLength();
            rightScale *= unscaledVector.GetLength();
        }

        // Quadrant I
        if(unscaledVector.GetY() >= 0.f && unscaledVector.GetX() >= 0.f)
        {
            scaledVector.SetX((forwardScale * rightScale) /
                sqrt(forwardScale*forwardScale + rightScale*rightScale * pow(tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX())), 2.f)));
            scaledVector.SetY(scaledVector.GetX()*tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX())));
        }
        // Quadrant II
        else if(unscaledVector.GetY() >= 0.f && unscaledVector.GetX() < 0.f)
        {
            scaledVector.SetX(-1.f*(forwardScale * leftScale) /
                sqrt(forwardScale*forwardScale + leftScale*leftScale * pow(tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX(-1.f))), 2.f)));
            scaledVector.SetY(-1.f*scaledVector.GetX()*tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX(-1.f))));
        }
        // Quadrant III
        else if(unscaledVector.GetY() < 0.f && unscaledVector.GetX() < 0.f)
        {
            scaledVector.SetX(-1.f*(backScale * leftScale) /
                sqrt(backScale*backScale + leftScale*leftScale * pow(tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX(-1.f))), 2.f)));
            scaledVector.SetY(scaledVector.GetX()*tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX(-1.f))));
        }
        // Quadrant IV
        else
        {
            scaledVector.SetX((backScale * rightScale) /
                sqrt(backScale*backScale + rightScale*rightScale * pow(tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX())), 2.f)));
            scaledVector.SetY(-1.f*scaledVector.GetX()*tan(unscaledVector.AngleSafe(AZ::Vector2::CreateAxisX())));
        }

        return scaledVector;
    }

    void FirstPersonControllerComponent::ApplyMovingUpInclineXYSpeedFactor()
    {
        if(m_grounded)
        {
            // The character is not on an incline, so don't apply an incline factor
            if(m_velocityXCrossYDirection.IsClose(m_velocityZPosDirection))
            {
                m_movingUpInclineFactor = 1.f;
                m_prevGroundCloseSumNormals = m_velocityZPosDirection;
                return;
            }

            if(AZ::Vector3(m_prevTargetVelocity.GetX(), m_prevTargetVelocity.GetY(), 0.f).Angle(m_velocityXCrossYDirection) > AZ::Constants::HalfPi)
            {
                m_prevGroundCloseSumNormals = m_velocityXCrossYDirection;

                // Calculate the steepness of the incline
                const float steepness = m_velocityZPosDirection.Angle(m_velocityXCrossYDirection) / AZ::Constants::HalfPi;

                // Get the component of the velocity vector pointing towards the incline
                const AZ::Vector2 currentVelocityXYTowardsIncline = AZ::Vector2(m_prevTargetVelocity).GetProjected(AZ::Vector2(-m_velocityXCrossYDirection).GetNormalized());

                // Calculate the maximum expected velocity when moving directly towards the incline
                AZ::Vector2 maxVelocityXYTowardsIncline = m_prevTargetVelocity.GetLength()*AZ::Vector2(-m_velocityXCrossYDirection).GetNormalized();
                const AZ::Vector3 tiltedMaxVelocityXYTowardsIncline = TiltVectorXCrossY(maxVelocityXYTowardsIncline, m_velocityXCrossYDirection);
                maxVelocityXYTowardsIncline = AZ::Vector2(tiltedMaxVelocityXYTowardsIncline);

                // Use the steepness and ratio of the velocity towards the incline and the max velocity towards the incline as the factor
                m_movingUpInclineFactor = (1.f - steepness * currentVelocityXYTowardsIncline.GetLength() / maxVelocityXYTowardsIncline.GetLength());

                m_prevTargetVelocity *= m_movingUpInclineFactor;
            }
        }
        else if(!m_instantVelocityRotation)
        {
            const AZ::Vector3 groundCloseSumNormals = GetGroundCloseSumNormalsDirection();

            // An incline isn't below the character, so don't apply an incline factor
            if(groundCloseSumNormals.IsZero() && m_movingUpInclineFactor == 1.f)
            {
                return;
            }
            else if(!groundCloseSumNormals.IsZero())
            {
                m_prevGroundCloseSumNormals = groundCloseSumNormals;
            }
            // Use captured grace normal during coyote time if walking off ledge
            else if(m_airTime < m_coyoteTime && !m_ungroundedDueToJump && m_coyoteTimeTracksLastNormal)
            {
                m_prevGroundCloseSumNormals = m_coyoteVelocityXCrossYDirection;
            }
            else if(!m_ungroundedDueToJump && !m_coyoteTimeTracksLastNormal)
            {
                m_prevGroundCloseSumNormals = m_velocityZPosDirection;
            }

            if(AZ::Vector3(m_prevTargetVelocity.GetX(), m_prevTargetVelocity.GetY(), 0.f).Angle(m_prevGroundCloseSumNormals) > AZ::Constants::HalfPi)
            {
                // Calculate the steepness of the incline
                const float steepness = m_velocityZPosDirection.Angle(m_prevGroundCloseSumNormals) / AZ::Constants::HalfPi;

                // Get the velocity that would be tilted if the character were grounded
                const AZ::Vector2 velocityXYTilted = AZ::Vector2(TiltVectorXCrossY(m_prevTargetVelocityXY, m_prevGroundCloseSumNormals));

                // Get the component of the velocity vector pointing towards the incline
                const AZ::Vector2 currentVelocityXYTowardsIncline = AZ::Vector2(velocityXYTilted).GetProjected(AZ::Vector2(-m_prevGroundCloseSumNormals).GetNormalized());

                // Calculate the maximum expected velocity when moving directly towards the incline
                AZ::Vector2 maxVelocityXYTowardsIncline = m_prevTargetVelocityXY.GetLength()*AZ::Vector2(-m_prevGroundCloseSumNormals).GetNormalized();
                const AZ::Vector3 tiltedMaxVelocityXYTowardsIncline = TiltVectorXCrossY(maxVelocityXYTowardsIncline, m_prevGroundCloseSumNormals);
                maxVelocityXYTowardsIncline = AZ::Vector2(tiltedMaxVelocityXYTowardsIncline);

                // Use the steepness and ratio of the velocity towards the incline and the max velocity towards the incline as the factor
                m_movingUpInclineFactor = (1.f - steepness * currentVelocityXYTowardsIncline.GetLength() / maxVelocityXYTowardsIncline.GetLength());

                if(!(m_airTime < m_coyoteTime && !m_ungroundedDueToJump))
                {
                    m_prevTargetVelocity.SetX(velocityXYTilted.GetX()*m_movingUpInclineFactor);
                    m_prevTargetVelocity.SetY(velocityXYTilted.GetY()*m_movingUpInclineFactor);
                }
                else
                    m_prevTargetVelocity *= m_movingUpInclineFactor;
            }
        }
    }

    // Here target velocity is with respect to the character's frame of reference
    void FirstPersonControllerComponent::SprintManager(const AZ::Vector2& targetVelocityXY, const float& deltaTime)
    {
        // The sprint value should never be 0, it shouldn't be applied if you're trying to moving backwards,
        // and it shouldn't be applied if you're crouching (depending on various settings)
        if(m_sprintValue == 0.f
           || (!m_sprintWhileCrouched && !m_crouchSprintCausesStanding && !m_standing)
           || (!m_applyVelocityXY.GetY() && !m_applyVelocityXY.GetX())
           || (m_forwardValue == -m_backValue && -m_leftValue == m_rightValue)
           || (targetVelocityXY.IsZero())
           || (m_sprintValue != 0.f
               && !m_sprintBackwards
               && ((!m_forwardValue && !m_leftValue && !m_rightValue) ||
                   (!m_forwardValue && -m_leftValue == m_rightValue) ||
                   (targetVelocityXY.GetY() < 0.f)) ))
            m_sprintValue = 0.f;

        if((m_sprintViaScript && m_sprintEnableDisable) && (targetVelocityXY.GetY() > 0.f || m_sprintBackwards))
        {
            m_sprintValue = 1.f;
            m_sprintAccelValue = m_sprintAccelScale;
        }
        else if(m_sprintViaScript && !m_sprintEnableDisable)
            m_sprintValue = 0.f;

        // Reset the counter if there is no movement
        if(m_applyVelocityXY.IsZero())
            m_sprintAccumulatedAccel = 0.f;

        if(m_sprintValue == 0.f || m_sprintCooldownTimer != 0.f)
            m_sprintVelocityAdjust = 1.f;
        else
        {
            if(m_velocityXCrossYDirection.GetZ() >= 0.f)
                m_sprintVelocityAdjust = CreateEllipseScaledVector(targetVelocityXY.GetNormalized(), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
            else
                m_sprintVelocityAdjust = CreateEllipseScaledVector((-targetVelocityXY).GetNormalized(), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
        }

        if(m_sprintPrevValue == 0.f && !AZ::IsClose(m_sprintVelocityAdjust, 1.f) && m_sprintHeldDuration < m_sprintMaxTime && m_sprintCooldownTimer == 0.f)
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnSprintStarted);
        else if(m_sprintPrevValue == 1.f && m_sprintValue == 0.f && AZ::IsClose(m_sprintVelocityAdjust, 1.f))
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnSprintStopped);

        m_sprintPrevValue = m_sprintValue;

        // If sprint is to be applied then increment the sprint counter
        if(!AZ::IsClose(m_sprintVelocityAdjust, 1.f) && m_sprintHeldDuration < m_sprintMaxTime && m_sprintCooldownTimer == 0.f)
        {
            m_staminaIncreasing = false;

            // Cause the character to stand if trying to sprint while crouched and the setting is enabled
            if(m_crouchSprintCausesStanding && m_crouching && m_grounded)
                m_crouching = false;

            // Figure out which of the scaled sprint velocity directions is the greatest
            float greatestSprintScale = 1.f;
            for(const float scale: {m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight})
                if(abs(scale) > abs(greatestSprintScale))
                        greatestSprintScale = scale;

            if(m_sprintAccelValue >= 1.f)
            {
                if(greatestSprintScale >= 1.f)
                    m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale - 1.f) * (m_sprintVelocityAdjust - 1.f) + 1.f;
                else
                    m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale) * (m_sprintVelocityAdjust) + 1.f;
            }
            else
            {
                if(greatestSprintScale >= 1.f)
                    m_sprintAccelAdjust = (m_sprintAccelValue)/(greatestSprintScale - 1.f) * (m_sprintVelocityAdjust - 1.f);
                else
                    m_sprintAccelAdjust = (m_sprintAccelValue)/(greatestSprintScale) * (m_sprintVelocityAdjust);
            }

            if(m_sprintUsesStamina)
            {
                m_staminaDecreasing = true;
                m_sprintHeldDuration += deltaTime * (m_sprintVelocityAdjust-1.f)/(greatestSprintScale-1.f);
            }

            if(m_sprintHeldDuration >= m_sprintMaxTime)
            {
                m_sprintHeldDuration = m_sprintMaxTime;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStaminaReachedZero);
            }

            m_sprintPause = m_sprintPauseTime;

            m_sprintPrevVelocityLength = m_applyVelocityXY.GetLength();
        }
        // Otherwise if the sprint velocity isn't applied then decrement the sprint counter
        else
        {
            m_staminaDecreasing = false;

            m_sprintValue = 0.f;

            // Set the sprint acceleration adjust according to the local direction the character is moving
            if(!m_sprintStopAccelAdjustCaptured && targetVelocityXY.IsZero())
            {
                // Figure out which of the scaled sprint velocity directions is the greatest
                float greatestSprintScale = 0.f;
                for(const float scale: {m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight})
                    if(abs(scale) > abs(greatestSprintScale))
                            greatestSprintScale = scale;

                float lastAdjustScale = 1.f;
                if(m_instantVelocityRotation)
                {
                    if(m_velocityXCrossYDirection.GetZ() >= 0.f)
                        lastAdjustScale = CreateEllipseScaledVector(m_prevTargetVelocityXY.GetNormalized(), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
                    else
                        lastAdjustScale = CreateEllipseScaledVector((-m_prevTargetVelocityXY).GetNormalized(), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
                }
                else
                {
                    if(m_velocityXCrossYDirection.GetZ() >= 0.f)
                        lastAdjustScale = CreateEllipseScaledVector(AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_prevTargetVelocityXY)).GetNormalized()), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
                    else
                        lastAdjustScale = CreateEllipseScaledVector(AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(-m_prevTargetVelocityXY)).GetNormalized()), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
                }

                if(m_sprintAccelValue >= 1.f)
                {
                    if(greatestSprintScale >= 1.f)
                        m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale - 1.f) * (lastAdjustScale - 1.f) + 1.f;
                    else
                        m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale) * (lastAdjustScale) + 1.f;
                }
                else
                {
                    if(greatestSprintScale >= 1.f)
                        m_sprintAccelAdjust = (m_sprintAccelValue)/(greatestSprintScale - 1.f) * (lastAdjustScale - 1.f);
                    else
                        m_sprintAccelAdjust = (m_sprintAccelValue)/(greatestSprintScale) * (lastAdjustScale);
                }

                m_sprintStopAccelAdjustCaptured = true;
            }
            else if(AZ::IsClose(m_sprintAccumulatedAccel, 0.f) && AZ::IsClose(m_sprintVelocityAdjust, 1.f))
                m_sprintAccumulatedAccel = 0.f;

            if(m_sprintAccumulatedAccel <= 0.f)
            {
                m_sprintPrevVelocityLength = 0.f;
                m_sprintStopAccelAdjustCaptured = false;
                m_sprintAccelAdjust = 1.f;
            }

            // When the sprint held duration exceeds the maximum sprint time then initiate the cooldown period
            if(m_sprintHeldDuration >= m_sprintMaxTime && m_sprintCooldownTimer == 0.f)
            {
                m_sprintVelocityAdjust = 1.f;
                m_sprintCooldownTimer = m_sprintTotalCooldownTime;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnCooldownStarted);
            }

            m_sprintPause -= deltaTime;
            if(m_sprintPause < 0.f)
                m_sprintPause = 0.f;

            if(m_sprintPause == 0.f && m_sprintCooldownTimer == 0.f && m_regenerateStaminaAutomatically && m_sprintHeldDuration > 0.f)
            {
                // Decrement the sprint held duration at a rate which makes it so that the stamina
                // will regenerate when nearly depleted at the same time it would take if you were
                // just wait through the cooldown time.
                // Decrement this value by only deltaTime if you wish to instead use m_sprintPause
                // to achieve the same timing but instead through the use of a pause.
                m_sprintHeldDuration -= deltaTime * ((m_sprintMaxTime+m_sprintPauseTime)/m_sprintTotalCooldownTime) * m_sprintRegenRate;
                m_staminaIncreasing = true;

                if(m_sprintHeldDuration <= 0.f)
                {
                    m_sprintHeldDuration = 0.f;
                    FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStaminaCapped);
                }
            }
            else
                m_staminaIncreasing = false;

            if(m_sprintCooldownTimer != 0.f)
            {
                m_sprintCooldownTimer -= deltaTime;
                if(m_sprintCooldownTimer <= 0.f)
                {
                    m_sprintCooldownTimer = 0.f;
                    m_sprintPause = 0.f;
                    FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnCooldownDone);
                    if(m_regenerateStaminaAutomatically)
                    {
                        m_sprintHeldDuration = 0.f;
                        m_staminaIncreasing = true;
                        FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStaminaCapped);
                    }
                }
            }
        }

        if(m_sprintMaxTime != 0.f)
            m_staminaPercentage = 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime;
        else
            m_staminaPercentage = 0.f;
        //AZ_Printf("First Person Controller Component", "Stamina = %.10f\%", m_staminaPercentage);
    }

    void FirstPersonControllerComponent::SmoothCriticallyDampedFloat(float& value, float& valueRate, const float& timeDelta, const float& target, const float& smoothTime)
    {
        if(smoothTime > 0.f)
        {
            const float omega = 2.f / smoothTime;
            const float x = omega * timeDelta;
            const float exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
            const float change = value - target;
            const float temp = (valueRate + change * omega) * timeDelta;
            valueRate = (valueRate - temp * omega) * exp;
            value = target + (change + temp) * exp;
        }
        else if(timeDelta > 0.f)
        {
            valueRate = (target - value) / timeDelta;
            value = target;
        }
        else
        {
            value = target;
            valueRate = float(0); // Zero the rate
        }
    }

    void FirstPersonControllerComponent::CrouchManager(const float& deltaTime)
    {
        if(m_activeCameraEntity == nullptr)
            return;

        AZ::TransformInterface* cameraTransform = m_activeCameraEntity->GetTransform();

        if(m_crouchEnableToggle && (m_grounded || m_crouching || m_crouchWhenNotGrounded)
            && !m_crouchScriptLocked && m_crouchPrevValue == 0.f && m_crouchValue == 1.f)
        {
            m_crouching = !m_crouching;
        }
        else if(!m_crouchEnableToggle && (m_grounded || m_crouching) && !m_crouchScriptLocked)
        {
            if(m_crouchValue != 0.f
                 && ((m_sprintValue == 0.f || !m_crouchSprintCausesStanding)
                  || ((m_crouchPriorityWhenSprintPressed) && (m_standing || m_crouching)))
                 && (m_jumpValue == 0.f || !m_crouchJumpCausesStanding || (m_jumpReqRepress && (m_standing || m_crouching))))
                m_crouching = true;
            else
                m_crouching = false;
        }

        // If the crouch key takes priority when the sprint key is held and we're attempting to crouch
        // while the sprint key is being pressed then stop the sprinting and continue crouching
        if(m_crouchPriorityWhenSprintPressed
                && !m_sprintWhileCrouched
                && m_sprintValue != 0.f
                && m_crouching
                && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
            m_sprintValue = 0.f;
        // Otherwise if the crouch key does not take priority when the sprint key is held,
        // and we are attempting to crouch while the sprint key is held, then do not crouch
        else if(!m_crouchPriorityWhenSprintPressed
            && m_sprintValue != 0.f
            && m_grounded
            && m_crouching
            && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
             m_crouching = false;

        //AZ_Printf("First Person Controller Component", "m_crouching = %s", m_crouching ? "true" : "false");

        // Crouch down
        if(m_crouching && (!m_crouched || m_grounded || m_crouchWhenNotGrounded) && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
        {
            if(m_standing)
            {
                m_crouchCurrentUpDownTime = 0.f;
                m_standing = false;
            }
            else if(m_standingUpMove)
            {
                // Back-calculate m_crouchCurrentUpDownTime based on the current crouching height
                m_crouchCurrentUpDownTime = -1.f * ((sqrt(m_crouchTime * (m_crouchDownInitVelocity - m_crouchDownFinalVelocity) * (2.f * m_capsuleCurrentHeight - 2.f * m_capsuleHeight) + m_crouchTime * m_crouchTime * m_crouchDownInitVelocity * m_crouchDownInitVelocity) - m_crouchTime * m_crouchDownInitVelocity)) / (m_crouchDownInitVelocity - m_crouchDownFinalVelocity);
                // Ensure the resulting calculation is real, otherwise set it to zero
                if(isnan(m_crouchCurrentUpDownTime))
                    m_crouchCurrentUpDownTime = 0.f;
                m_standingUpMove = false;
            }

            m_crouchingDownMove = true;

            if(m_cameraLocalZTravelDistance == 0.f)
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStartedCrouching);

            float cameraTravelDelta = -1.f * m_capsuleCurrentHeight;
            // Calculate the current crouch height
            m_capsuleCurrentHeight = m_capsuleHeight - (m_crouchCurrentUpDownTime * (2 * m_crouchTime * m_crouchDownInitVelocity - m_crouchCurrentUpDownTime * m_crouchDownInitVelocity + m_crouchCurrentUpDownTime * m_crouchDownFinalVelocity)) / (2 * m_crouchTime);
            cameraTravelDelta += m_capsuleCurrentHeight;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            m_crouchCurrentUpDownTime += deltaTime;

            if(m_cameraLocalZTravelDistance <= -1.f * m_crouchDistance || m_crouchCurrentUpDownTime > m_crouchTime)
            {
                cameraTravelDelta += abs(m_cameraLocalZTravelDistance) - m_crouchDistance;
                m_cameraLocalZTravelDistance = -1.f * m_crouchDistance;
                m_crouchCurrentUpDownTime = m_crouchTime;
                m_crouchingDownMove = false;
                m_crouched = true;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnCrouched);
            }

            // Adjust the height of the collider capsule based on the crouching height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleCurrentHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            float stepHeight = 0.f;
            Physics::CharacterRequestBus::EventResult(stepHeight, GetEntityId(),
                &Physics::CharacterRequestBus::Events::GetStepHeight);

            // Subtract the distance to get down to the crouching height
            m_capsuleCurrentHeight += cameraTravelDelta;
            if(m_capsuleCurrentHeight < (2.f*m_capsuleRadius + 0.00001f))
                m_capsuleCurrentHeight = 2.f*m_capsuleRadius + 0.00001f;
            if(m_capsuleCurrentHeight < (stepHeight + 0.00001f))
                m_capsuleCurrentHeight = stepHeight + 0.00001f;
            //AZ_Printf("First Person Controller Component", "Crouching capsule height = %.10f", m_capsuleCurrentHeight);

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleCurrentHeight);

            cameraTransform->SetLocalZ(cameraTransform->GetLocalZ() + cameraTravelDelta);
        }
        // Stand up
        else if(!m_crouching && m_cameraLocalZTravelDistance != 0.f)
        {
            if(m_crouched)
            {
                m_crouchCurrentUpDownTime = m_standTime;
                m_crouched = false;
            }
            else if(m_crouchingDownMove)
            {
                // Back-calculate m_crouchCurrentUpDownTime based on the current crouching height
                m_crouchCurrentUpDownTime = ((sqrt(m_standTime * (m_crouchUpInitVelocity - m_crouchUpFinalVelocity) * (2.f * m_capsuleHeight - 2.f * m_capsuleCurrentHeight) + m_standTime * m_standTime * m_crouchUpFinalVelocity * m_crouchUpFinalVelocity) - sqrt(m_standTime * m_standTime * m_crouchUpFinalVelocity * m_crouchUpFinalVelocity)))/(m_crouchUpInitVelocity - m_crouchUpFinalVelocity);
                // Ensure the resulting calculation is real, otherwise set it to m_standTime
                if(isnan(m_crouchCurrentUpDownTime))
                    m_crouchCurrentUpDownTime = m_standTime;
                m_crouchingDownMove = false;
            }

            m_standingUpMove = true;

            if(m_cameraLocalZTravelDistance == -1.f * m_crouchDistance)
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStartedStanding);

            // Create a shapecast sphere that will be used to detect whether there is an obstruction
            // above the players head, and prevent them from fully standing up if there is
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

            // Move the sphere to the location of the character and apply the Z offset
            sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius));

            AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ();

            // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
            if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
            {
                sphereCastDirection = m_sphereCastsAxisDirectionPose;
                if(m_sphereCastsAxisDirectionPose.GetZ() >= 0.f)
                    sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius)));
                else
                    sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius)));
            }

            AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
                m_capsuleRadius,
                sphereCastPose,
                sphereCastDirection,
                m_uncrouchHeadSphereCastOffset,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                m_standCollisionGroup,
                nullptr);

            request.m_reportMultipleHits = true;

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

            // Disregard intersections with the character's collider and its child entities
            auto selfChildEntityCheck = [this](AzPhysics::SceneQueryHit& hit)
                {
                    if(hit.m_entityId == GetEntityId())
                        return true;

                    // Obtain the child IDs if we don't already have them
                    if(!m_obtainedChildIds)
                    {
                        AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                        m_obtainedChildIds = true;
                    }

                    for(AZ::EntityId id: m_children)
                    {
                        if(hit.m_entityId == id)
                            return true;
                    }

                    if(m_standIgnoreDynamicRigidBodies)
                    {
                        // Check to see if the entity hit is dynamic
                        AzPhysics::RigidBody* bodyHit = NULL;
                        Physics::RigidBodyRequestBus::EventResult(bodyHit, hit.m_entityId,
                            &Physics::RigidBodyRequests::GetRigidBody);

                        // Static Rigid Bodies are not connected to the RigidBodyRequestBus and therefore
                        // do not have a handler for it
                        if(bodyHit != NULL && !bodyHit->IsKinematic())
                            return true;
                    }

                    return false;
                };

            AZStd::erase_if(hits.m_hits, selfChildEntityCheck);

            m_standPreventedEntityIds.clear();
            if(hits)
                for(AzPhysics::SceneQueryHit hit: hits.m_hits)
                    m_standPreventedEntityIds.push_back(hit.m_entityId);

            // Bail if something is detected above the player
            if(hits || m_standPreventedViaScript)
            {
                m_crouchPrevValue = m_crouchValue;
                m_standPrevented = true;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStandPrevented);
                return;
            }
            m_standPrevented = false;

            float cameraTravelDelta = -1.f * m_capsuleCurrentHeight;
            // Calculate the current crouch height
            m_capsuleCurrentHeight = m_capsuleHeight - (m_crouchCurrentUpDownTime * (2 * m_standTime * m_crouchUpFinalVelocity + m_crouchCurrentUpDownTime * m_crouchUpInitVelocity - m_crouchCurrentUpDownTime * m_crouchUpFinalVelocity)) / (2 * m_standTime);
            cameraTravelDelta += m_capsuleCurrentHeight;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            m_crouchCurrentUpDownTime -= deltaTime;

            if(m_cameraLocalZTravelDistance >= 0.f || m_crouchCurrentUpDownTime <= 0.f)
            {
                cameraTravelDelta -= m_cameraLocalZTravelDistance;
                m_cameraLocalZTravelDistance = 0.f;
                m_crouchCurrentUpDownTime = 0.f;
                m_standingUpMove = false;
                m_standing = true;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStoodUp);
            }

            // Adjust the height of the collider capsule based on the standing height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleCurrentHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Add the distance to get back to the standing height
            m_capsuleCurrentHeight += cameraTravelDelta;
            if(m_capsuleCurrentHeight > m_capsuleHeight)
                m_capsuleCurrentHeight = m_capsuleHeight;
            //AZ_Printf("First Person Controller Component", "Standing capsule height = %.10f", m_capsuleCurrentHeight);

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleCurrentHeight);

            cameraTransform->SetLocalZ(cameraTransform->GetLocalZ() + cameraTravelDelta);
        }

        m_crouchPrevValue = m_crouchValue;
    }

    void FirstPersonControllerComponent::UpdateVelocityXY(const float& deltaTime)
    {
        float forwardBack = m_forwardValue * m_forwardScale + -1.f * m_backValue * m_backScale;
        float leftRight = -1.f * m_leftValue * m_leftScale + m_rightValue * m_rightScale;

        // Remove the scale factor since it's going to be applied after the normalization
        if(forwardBack >= 0.f)
            forwardBack /= m_forwardScale;
        else
            forwardBack /= m_backScale;

        // If the character is being flipped upside-down then flip the X&Y movement
        if(m_velocityXCrossYDirection.GetZ() < 0.f)
        {
            forwardBack *= -1.f;
            leftRight *= -1.f;
        }

        if(leftRight >= 0.f)
            leftRight /= m_rightScale;
        else
            leftRight /= m_leftScale;

        AZ::Vector2 targetVelocityXY = AZ::Vector2(leftRight, forwardBack);

        // Normalize the vector if its magnitude is greater than 1 and then scale it
        if((forwardBack || leftRight) && sqrt(forwardBack*forwardBack + leftRight*leftRight) > 1.f)
            targetVelocityXY.Normalize();

        if(m_velocityXCrossYDirection.GetZ() >= 0.f)
            targetVelocityXY = CreateEllipseScaledVector(targetVelocityXY, m_forwardScale, m_backScale, m_leftScale, m_rightScale);
        else
            targetVelocityXY = -CreateEllipseScaledVector((-targetVelocityXY), m_forwardScale, m_backScale, m_leftScale, m_rightScale);

        // Call the sprint manager
        if(!m_scriptSetsTargetVelocityXY)
            SprintManager(targetVelocityXY, deltaTime);

        // Apply the speed, sprint factor, and crouch factor
        if(m_standing)
            targetVelocityXY *= m_speed * m_sprintVelocityAdjust;
        else if(m_sprintWhileCrouched && !m_standing)
            targetVelocityXY *= m_speed * m_sprintVelocityAdjust * m_crouchScale;
        else
            targetVelocityXY *= m_speed * m_crouchScale;

        if(m_scriptSetsTargetVelocityXY)
        {
            targetVelocityXY = m_scriptTargetVelocityXY;
            SprintManager(targetVelocityXY, deltaTime);
        }
        else
            m_scriptTargetVelocityXY = targetVelocityXY;

        // Rotate the target velocity vector so that it can be compared against the applied velocity
        const AZ::Vector2 targetVelocityXYWorld = AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3(targetVelocityXY)));

        // Obtain the last applied velocity if the target velocity changed
        if((m_instantVelocityRotation ? (m_prevTargetVelocityXY != targetVelocityXY)
                                        : (m_prevTargetVelocityXY != targetVelocityXYWorld))
            || (!m_velocityXYIgnoresObstacles && m_velocityFromImpulse.IsZero() && m_linearImpulse.IsZero() && m_velocityXYObstructed)
            || (AZ::GetSign(m_prevVelocityXCrossYDirection.GetZ()) != AZ::GetSign(m_velocityXCrossYDirection.GetZ())))
        {
            if(m_instantVelocityRotation)
            {
                // Set the previous target velocity to the new one
                m_prevTargetVelocityXY = targetVelocityXY;
                // Store the last applied velocity to be used for the lerping
                if(!m_velocityXYIgnoresObstacles && m_velocityFromImpulse.IsZero() && m_linearImpulse.IsZero() && m_velocityXYObstructed)
                {
                    m_applyVelocityXY = AZ::Vector2(m_correctedVelocityXY);
                    m_correctedVelocityXY = AZ::Vector2::CreateZero();
                }
                m_prevApplyVelocityXY = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocityXY)));
            }
            else
            {
                // Set the previous target velocity to the new one
                m_prevTargetVelocityXY = targetVelocityXYWorld;
                // Store the last applied velocity to be used for the lerping
                if(!m_velocityXYIgnoresObstacles && m_velocityFromImpulse.IsZero() && m_linearImpulse.IsZero() && m_velocityXYObstructed)
                    m_applyVelocityXY = AZ::Vector2(m_correctedVelocityXY);

                m_prevApplyVelocityXY = m_applyVelocityXY;
            }

            // Once the character's movement gets flipped on Z, m_prevApplyVelocityXY needs to be flipped,
            // so long as it hasn't occured around the world's X axis
            if(AZ::GetSign(m_prevVelocityXCrossYDirection.GetZ()) != AZ::GetSign(m_velocityXCrossYDirection.GetZ()) && !AZ::IsClose(m_velocityXCrossYDirection.GetY(), 0.f))
                m_prevApplyVelocityXY *= -1.f;

            // Reset the lerp time since the target velocity changed
            m_lerpTime = 0.f;
        }

        m_prevVelocityXCrossYDirection = m_velocityXCrossYDirection;

        // Logic to determine if the lateral velocity on X&Y when jumping on inclines is to be captured
        if(!m_grounded && !m_jumpInclineVelocityXYCaptured && m_velocityXCrossYTracksNormal && !m_instantVelocityRotation)
        {
            const AZ::Vector3 groundCloseSumNormals = GetGroundCloseSumNormalsDirection();
            // If either moving up inclines isn't slowed or the character is jumping down an incline, then capture the velocity on X&Y
            if((groundCloseSumNormals != m_velocityZPosDirection && !groundCloseSumNormals.IsZero()) &&
               ((!m_movingUpInclineSlowed) ||
                (m_movingUpInclineSlowed && AZ::Vector3(m_prevTargetVelocity.GetX(), m_prevTargetVelocity.GetY(), 0.f).Angle(groundCloseSumNormals) < AZ::Constants::HalfPi)))
            {
                // Capture the X&Y components of the velocity vector when jumping on an inclined surface
                m_prevApplyVelocityXY = AZ::Vector2(m_prevTargetVelocity.GetX(), m_prevTargetVelocity.GetY());
                m_applyVelocityXY = m_prevApplyVelocityXY;
                // Set the flag which says that the lateral velocity has been captured
                m_jumpInclineVelocityXYCaptured = true;
                // Set the lerp time to zero since the a new initial velocity is to be used inside LerpVelocityXY()
                m_lerpTime = 0.f;
            }
        }
        else if(m_grounded)
            m_jumpInclineVelocityXYCaptured = false;

        // Lerp to the velocity if we're not already there
        if(m_applyVelocityXY != targetVelocityXYWorld)
        {
            if(m_instantVelocityRotation)
                m_applyVelocityXY = AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3(LerpVelocityXY(targetVelocityXY, deltaTime))));
            else
                m_applyVelocityXY = LerpVelocityXY(targetVelocityXYWorld, deltaTime);
        }
        else
        {
            m_accelerating = false;
            m_decelerationFactorApplied = false;
            m_opposingDecelFactorApplied = false;
        }

        // Debug print statements to observe the velocity, acceleration, and translation
        //AZ_Printf("First Person Controller Component", "m_currentHeading = %.10f", m_currentHeading);
        //AZ_Printf("First Person Controller Component", "m_applyVelocityXY.GetLength() = %.10f", m_applyVelocityXY.GetLength());
        //AZ_Printf("First Person Controller Component", "m_applyVelocityXY.GetX() = %.10f", m_applyVelocityXY.GetX());
        //AZ_Printf("First Person Controller Component", "m_applyVelocityXY.GetY() = %.10f", m_applyVelocityXY.GetY());
        //AZ_Printf("First Person Controller Component", "m_sprintAccumulatedAccel = %.10f", m_sprintAccumulatedAccel);
        //AZ_Printf("First Person Controller Component", "m_sprintValue = %.10f", m_sprintValue);
        //AZ_Printf("First Person Controller Component", "m_sprintAccelValue = %.10f", m_sprintAccelValue);
        //AZ_Printf("First Person Controller Component", "m_sprintAccelAdjust = %.10f", m_sprintAccelAdjust);
        //AZ_Printf("First Person Controller Component", "m_decelerationFactor = %.10f", m_decelerationFactor);
        //AZ_Printf("First Person Controller Component", "m_sprintVelocityAdjust = %.10f", m_sprintVelocityAdjust);
        //AZ_Printf("First Person Controller Component", "m_sprintHeldDuration = %.10f", m_sprintHeldDuration);
        //AZ_Printf("First Person Controller Component", "m_sprintPause = %.10f", m_sprintPause);
        //AZ_Printf("First Person Controller Component", "m_sprintPauseTime = %.10f", m_sprintPauseTime);
        //AZ_Printf("First Person Controller Component", "m_sprintCooldownTimer = %.10f", m_sprintCooldownTimer);
        //static AZ::Vector2 prevVelocity = m_applyVelocityXY;
        //AZ_Printf("First Person Controller Component", "dv/dt = %.10f", prevVelocity.GetDistance(m_applyVelocityXY)/deltaTime);
        //prevVelocity = m_applyVelocityXY;
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("First Person Controller Component", "X Translation = %.10f", pos.GetX());
        //AZ_Printf("First Person Controller Component", "Y Translation = %.10f", pos.GetY());
        //AZ_Printf("First Person Controller Component", "Z Translation = %.10f", pos.GetZ());
        //AZ_Printf("First Person Controller Component","");
    }

    // Update m_velocityXCrossYDirection based on the sum of the normal vectors beneath the character
    void FirstPersonControllerComponent::AcquireSumOfGroundNormals()
    {
        if(m_velocityXCrossYTracksNormal)
        {
            if(m_grounded || m_coyoteTime == 0.f || !m_coyoteTimeTracksLastNormal || m_applyGravityDuringCoyoteTime)
            {
                SetVelocityXCrossYDirection(GetGroundSumNormalsDirection());
                if(m_coyoteTimeTracksLastNormal)
                    m_coyoteVelocityXCrossYDirection = m_velocityXCrossYDirection;
            }
            else if(m_airTime < m_coyoteTime && !m_ungroundedDueToJump && m_coyoteTimeTracksLastNormal)
                SetVelocityXCrossYDirection(m_coyoteVelocityXCrossYDirection);
            else
            {
                SetVelocityXCrossYDirection(GetGroundSumNormalsDirection());
                if(m_coyoteTimeTracksLastNormal)
                    m_coyoteVelocityXCrossYDirection = m_velocityXCrossYDirection;
            }
        }
    }

    void FirstPersonControllerComponent::CheckCharacterMovementObstructed()
    {
        // Get the current velocity to determine if something was hit
        Physics::CharacterRequestBus::EventResult(m_currentVelocity, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetVelocity);

        if(!m_prevPrevTargetVelocity.IsClose(m_currentVelocity, m_velocityCloseTolerance))
        {
            // If enabled, cause the character's applied velocity to match the current velocity from Physics
            m_velocityXYObstructed = true;

            if(m_velocityXCrossYDirection == AZ::Vector3::CreateAxisZ())
                m_correctedVelocityXY = AZ::Vector2(m_currentVelocity);
            else
                m_correctedVelocityXY = AZ::Vector2(m_currentVelocity.Dot(TiltVectorXCrossY(AZ::Vector2::CreateAxisX(), m_velocityXCrossYDirection)), m_currentVelocity.Dot(TiltVectorXCrossY(AZ::Vector2::CreateAxisY(), m_velocityXCrossYDirection)));

            if(m_velocityZPosDirection == AZ::Vector3::CreateAxisZ())
                m_correctedVelocityZ = m_currentVelocity.GetZ();
            else
                m_correctedVelocityZ = m_currentVelocity.Dot(m_velocityZPosDirection);

            if(!m_gravityIgnoresObstacles && !m_prevTargetVelocity.IsClose(m_currentVelocity, m_velocityCloseTolerance) && m_prevTargetVelocity.Dot(m_velocityZPosDirection) < 0.f && AZ::IsClose(m_currentVelocity.Dot(m_velocityZPosDirection), 0.f))
            {
                // Gravity needs to be prevented for two ticks in a row to prevent exploitable behavior
                if(m_gravityPrevented[0])
                {
                    m_gravityPrevented[1] = true;
                    FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnCharacterGravityObstructed);
                }
                else
                    m_gravityPrevented[0] = true;
            }
            else
                m_gravityPrevented[0] = m_gravityPrevented[1] = false;

            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnVelocityXYObstructed);
        }
        else
        {
            m_correctedVelocityXY = m_applyVelocityXY;
            m_correctedVelocityZ = m_applyVelocityZ;
            m_velocityXYObstructed = false;
        }
    }

    void FirstPersonControllerComponent::CheckGrounded(const float& deltaTime)
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Used to determine when event notifications occur
        const bool prevGrounded = m_grounded;
        const bool prevGroundClose = m_groundClose;

        AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

        // Move the sphere to the location of the character and apply the Z offset
        sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ((1.f + m_groundSphereCastsRadiusPercentageIncrease/100.f)*m_capsuleRadius));

        AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ(-1.f);

        // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
        if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
        {
            sphereCastDirection = -m_sphereCastsAxisDirectionPose;
            if(m_sphereCastsAxisDirectionPose.GetZ() > 0.f)
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ((1.f + m_groundSphereCastsRadiusPercentageIncrease/100.f)*m_capsuleRadius)));
            else
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ((1.f + m_groundSphereCastsRadiusPercentageIncrease/100.f)*m_capsuleRadius)));
        }

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            (1.f + m_groundSphereCastsRadiusPercentageIncrease/100.f)*m_capsuleRadius,
            sphereCastPose,
            sphereCastDirection,
            m_groundedSphereCastOffset,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            m_groundedCollisionGroup,
            nullptr);

        request.m_reportMultipleHits = true;

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        AZStd::vector<AzPhysics::SceneQueryHit> steepNormals;

        // Enumerator for filtering the various scene query hit vectors
        enum groundSphereCasts : AZ::u8 {
            grounded = 0,
            groundClose = 1,
            coyoteTimeGroundClose = 2,
        };

        groundSphereCasts groundedGroundCloseOrGroundCloseCoyoteTime = grounded;

        // Disregard intersections with the character's collider, its child entities,
        // and if the slope angle of the thing that's intersecting is greater than the max grounded angle
        auto selfChildSlopeEntityCheck = [this, &steepNormals, &groundedGroundCloseOrGroundCloseCoyoteTime](AzPhysics::SceneQueryHit& hit)
            {
                if(hit.m_entityId == GetEntityId())
                    return true;

                // Obtain the child IDs if we don't already have them
                if(!m_obtainedChildIds)
                {
                    AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                    m_obtainedChildIds = true;
                }

                for(AZ::EntityId id: m_children)
                {
                    if(hit.m_entityId == id)
                        return true;
                }

                if(abs(hit.m_normal.AngleSafeDeg(m_sphereCastsAxisDirectionPose)) > m_maxGroundedAngleDegrees)
                {
                    steepNormals.push_back(hit);
                    //AZ_Printf("First Person Controller Component", "Steep Angle EntityId = %s", hit.m_entityId.ToString().c_str());
                    //AZ_Printf("First Person Controller Component", "Steep Angle = %.10f", hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
                    return true;
                }

                if(groundedGroundCloseOrGroundCloseCoyoteTime == grounded)
                    m_groundHits.push_back(hit);
                else if(groundedGroundCloseOrGroundCloseCoyoteTime == groundClose)
                    m_groundCloseHits.push_back(hit);
                else if(groundedGroundCloseOrGroundCloseCoyoteTime == coyoteTimeGroundClose)
                    m_groundCloseCoyoteTimeHits.push_back(hit);

                return false;
            };

        m_groundHits.clear();
        AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
        m_grounded = hits ? true : false;

        bool normalsSumNotSteep = false;

        // Check to see if the sum of the steep angles is less than or equal to m_maxGroundedAngleDegrees
        if(!m_grounded && steepNormals.size() > 1)
        {
            AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
            for(AzPhysics::SceneQueryHit normal: steepNormals)
                sumNormals += normal.m_normal;

            //AZ_Printf("First Person Controller Component", "Sum of Steep Angles = %.10f", sumNormals.AngleSafeDeg(m_sphereCastsAxisDirectionPose));
            if(abs(sumNormals.AngleSafeDeg(m_sphereCastsAxisDirectionPose)) <= m_maxGroundedAngleDegrees)
            {
                normalsSumNotSteep = true;
                m_grounded = true;
            }
        }

        if(normalsSumNotSteep)
            for(AzPhysics::SceneQueryHit normal: steepNormals)
                m_groundHits.push_back(normal);

        steepNormals.clear();

        if(m_scriptSetGroundTick)
        {
            m_grounded = m_scriptGrounded;
            m_scriptSetGroundTick = false;
        }

        // Accumulate airtime if the character isn't grounded, otherwise set it to zero
        // Set m_ungroundedDueToJump to false when the character is grounded
        if(m_grounded)
        {
            m_ungroundedDueToJump = false;
            m_airTime = 0.f;
        }
        else
            m_airTime += deltaTime;

        request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            (1.f + m_groundSphereCastsRadiusPercentageIncrease/100.f)*m_capsuleRadius,
            sphereCastPose,
            sphereCastDirection,
            m_groundCloseSphereCastOffset,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            m_groundedCollisionGroup,
            nullptr);

        request.m_reportMultipleHits = true;

        // Filter the ground close hits
        groundedGroundCloseOrGroundCloseCoyoteTime = groundClose;

        hits = sceneInterface->QueryScene(sceneHandle, &request);

        m_groundCloseHits.clear();
        AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
        m_groundClose = hits ? true : false;

        if(m_scriptSetGroundCloseTick)
        {
            m_groundClose = m_scriptGroundClose;
            m_scriptSetGroundCloseTick = false;
        }
        //AZ_Printf("First Person Controller Component", "m_groundClose = %s", m_groundClose ? "true" : "false");

        // Logic for handling ground close detection for Coyote Time application (e.g. moving down from a shallow to a steeper inclined surface)
        if(m_coyoteTime > 0.f)
        {
            // When the radius percentage increase is set to less than or equal to -100% then use a raycast instead
            const float noRadiusUseRacast = -100.f;
            if(m_groundCloseCoyoteTimeRadiusPercentageIncrease > noRadiusUseRacast)
            {
                request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
                    (1.f + m_groundCloseCoyoteTimeRadiusPercentageIncrease/100.f)*m_capsuleRadius,
                    sphereCastPose,
                    sphereCastDirection,
                    m_groundCloseCoyoteTimeOffset,
                    AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                    m_groundedCollisionGroup,
                    nullptr);
            }
            else
            {
                AzPhysics::RayCastRequest rayRequest;
                rayRequest.m_start = sphereCastPose.GetTranslation();
                rayRequest.m_direction = sphereCastDirection;
                rayRequest.m_distance = m_groundCloseCoyoteTimeOffset;
                rayRequest.m_queryType = AzPhysics::SceneQuery::QueryType::StaticAndDynamic;
                rayRequest.m_collisionGroup = m_groundedCollisionGroup;
                rayRequest.m_reportMultipleHits = true;
                rayRequest.m_filterCallback = nullptr;
                hits = sceneInterface->QueryScene(sceneHandle, &rayRequest);
            }

            request.m_reportMultipleHits = true;

            // Filter the ground close coyote time hits
            groundedGroundCloseOrGroundCloseCoyoteTime = coyoteTimeGroundClose;

            hits = sceneInterface->QueryScene(sceneHandle, &request);

            m_groundCloseCoyoteTimeHits.clear();
            AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
            m_groundCloseCoyoteTime = hits ? true : false;

            //AZ_Printf("First Person Controller Component", "m_groundCloseCoyoteTime = %s", m_groundCloseCoyoteTime ? "true" : "false");
        }

        // Trigger an event notification if the player hits the ground, is about to hit the ground,
        // or just left the ground (via jumping or otherwise)
        if(!prevGrounded && m_grounded)
        {
            if(m_velocityZPosDirection == AZ::Vector3::CreateAxisZ())
                m_fellDistance = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetZ() - m_fellFromHeight;
            else
                m_fellDistance = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetProjected(m_velocityZPosDirection).GetLength() - m_fellFromHeight;
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnGroundHit, m_fellDistance);
        }
        else if(!prevGroundClose && m_groundClose)
        {
            if(m_velocityZPosDirection == AZ::Vector3::CreateAxisZ())
                m_soonFellDistance = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetZ() - m_fellFromHeight;
            else
                m_soonFellDistance = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetProjected(m_velocityZPosDirection).GetLength() - m_fellFromHeight;
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnGroundSoonHit, m_soonFellDistance);
        }
        else if(prevGrounded && !m_grounded)
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnUngrounded);
    }

    void FirstPersonControllerComponent::UpdateJumpMaxHoldTime()
    {
        // Calculate the amount of time that the jump key can be held based on m_jumpHoldDistance
        // divided by the average of the initial jump velocity and the velocity at the edge of the capsule
        const float jumpVelocityCapsuleEdgeSquared = m_jumpInitialVelocity*m_jumpInitialVelocity
                                                         + 2.f*m_gravity*m_jumpHeldGravityFactor*m_jumpHoldDistance;
        // If the initial velocity is large enough such that the apogee can be reached outside of the capsule
        // then compute how long the jump key is held while still inside the jump hold offset intersection capsule
        if(jumpVelocityCapsuleEdgeSquared >= 0.f)
            m_jumpMaxHoldTime = m_jumpHoldDistance / ((m_jumpInitialVelocity
                                                        + sqrt(jumpVelocityCapsuleEdgeSquared)) / 2.f);
        // Otherwise the apogee will be reached inside m_jumpHoldDistance
        // and the jump time needs to computed accordingly
        else
        {
            AZ_Warning("First Person Controller Component", false, "Jump Hold Distance is higher than the max apogee of the jump.")
            m_jumpMaxHoldTime = abs(m_jumpInitialVelocity / (m_gravity*m_jumpHeldGravityFactor));
        }
    }

    void FirstPersonControllerComponent::UpdateVelocityZ(const float& deltaTime)
    {
        // Create a shapecast sphere that will be used to detect whether there is an obstruction
        // above the players head, and prevent them from fully standing up if there is
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

        // Move the sphere to the location of the character and apply the Z offset
        sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius));

        AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ();

        // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
        if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
        {
            sphereCastDirection = m_sphereCastsAxisDirectionPose;
            if(m_sphereCastsAxisDirectionPose.GetZ() >= 0.f)
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius)));
            else
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ(m_capsuleCurrentHeight - m_capsuleRadius)));
        }

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            m_capsuleRadius,
            sphereCastPose,
            sphereCastDirection,
            m_jumpHeadSphereCastOffset,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            m_headCollisionGroup,
            nullptr);

        request.m_reportMultipleHits = true;

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        // Disregard intersections with the character's collider and its child entities
        auto selfChildEntityCheck = [this](AzPhysics::SceneQueryHit& hit)
            {
                if(hit.m_entityId == GetEntityId())
                    return true;

                // Obtain the child IDs if we don't already have them
                if(!m_obtainedChildIds)
                {
                    AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                    m_obtainedChildIds = true;
                }

                for(AZ::EntityId id: m_children)
                {
                    if(hit.m_entityId == id)
                        return true;
                }

                if(m_jumpHeadIgnoreDynamicRigidBodies)
                {
                    // Check to see if the entity hit is dynamic
                    AzPhysics::RigidBody* bodyHit = NULL;
                    Physics::RigidBodyRequestBus::EventResult(bodyHit, hit.m_entityId,
                        &Physics::RigidBodyRequests::GetRigidBody);

                    // Static Rigid Bodies are not connected to the RigidBodyRequestBus and therefore
                    // do not have a handler for it
                    if(bodyHit != NULL && !bodyHit->IsKinematic())
                        return true;
                }

                return false;
            };

        AZStd::erase_if(hits.m_hits, selfChildEntityCheck);

        m_headHit = hits ? true : false;

        m_headHitEntityIds.clear();
        if(m_headHit)
            for(AzPhysics::SceneQueryHit hit: hits.m_hits)
                m_headHitEntityIds.push_back(hit.m_entityId);

        if(m_headHit && !m_grounded && m_applyVelocityZ >= 0.f)
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnHeadHit);

        if(m_gravityPrevented[0] && m_gravityPrevented[1])
        {
            m_applyVelocityZ = m_correctedVelocityZ;
            m_gravityPrevented[0] = false;
            m_gravityPrevented[1] = false;
            m_grounded = true;
            m_groundClose = true;
            if(m_jumpAllowedWhenGravityPrevented)
                m_jumpHeld = false;
        }

        const float prevApplyVelocityZ = m_applyVelocityZ;

        bool initialJump = false;

        if((m_grounded ||
            (m_airTime < m_coyoteTime && !m_ungroundedDueToJump && !m_applyGravityDuringCoyoteTime && !m_groundCloseCoyoteTime) ||
             m_jumpCoyoteGravityPending) &&
              m_jumpReqRepress && m_applyVelocityZ <= 0.f)
        {
            if((m_jumpValue || m_crouchJumpPending || m_jumpCoyoteGravityPending) && !m_jumpHeld && !m_headHit)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                    {
                        m_crouching = false;
                        if(m_crouchPendJumps && m_crouchEnableToggle)
                            m_crouchJumpPending = true;
                    }
                    if(!m_jumpWhileCrouched)
                        return;
                }
                m_crouchJumpPending = false;
                m_applyVelocityZCurrentDelta = m_jumpInitialVelocity + m_gravity * m_jumpHeldGravityFactor * deltaTime;
                initialJump = true;
                m_jumpHeld = true;
                m_jumpReqRepress = false;
                m_ungroundedDueToJump = true;
                if(m_jumpCoyoteGravityPending)
                {
                    m_applyVelocityZ = 0.f;
                    m_jumpCoyoteGravityPending = false;
                }
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnFirstJump);
            }
            else
            {
                m_applyVelocityZ = 0.f;
                m_applyVelocityZCurrentDelta = 0.f;
                m_applyVelocityZPrevDelta = 0.f;
                m_jumpTimer = 0.f;

                if(m_jumpValue == 0.f && m_jumpHeld)
                    m_jumpHeld = false;

                if(m_doubleJumpEnabled && m_finalJump)
                    m_finalJump = false;
            }
        }
        else if((m_jumpTimer + deltaTime/2.f) < m_jumpMaxHoldTime && m_applyVelocityZ > 0.f && m_jumpHeld && !m_jumpReqRepress)
        {
            m_ungroundedDueToJump = true;
            if(m_jumpValue == 0.f)
            {
                m_jumpHeld = false;
                m_jumpTimer = 0.f;
                m_applyVelocityZCurrentDelta = m_gravity * deltaTime;
            }
            else
            {
                m_jumpTimer += deltaTime;
                m_applyVelocityZCurrentDelta = m_gravity * m_jumpHeldGravityFactor * deltaTime;
            }
        }
        else
        {
            if(!m_jumpReqRepress)
                m_jumpReqRepress = true;

            if(m_jumpTimer != 0.f)
                m_jumpTimer = 0.f;

            if(m_applyVelocityZ <= 0.f)
                m_applyVelocityZCurrentDelta = m_gravity * m_jumpFallingGravityFactor * deltaTime;
            else
                m_applyVelocityZCurrentDelta = m_gravity * deltaTime;

            if(m_jumpHeld && m_jumpValue == 0.f)
                m_jumpHeld = false;

            if(m_doubleJumpEnabled && !m_finalJump && !m_jumpHeld && m_jumpValue != 0.f)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                        m_crouching = false;
                    return;
                }
                m_applyVelocityZ = m_jumpSecondInitialVelocity + m_gravity * m_jumpHeldGravityFactor * deltaTime;
                m_applyVelocityZCurrentDelta = m_gravity * deltaTime;
                m_applyVelocityZCurrentDelta = -m_gravity * deltaTime;
                m_finalJump = true;
                m_jumpHeld = true;
                FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnFinalJump);
            }

            if(m_airTime < m_coyoteTime && !m_ungroundedDueToJump && m_applyGravityDuringCoyoteTime && m_jumpValue)
                m_jumpCoyoteGravityPending = true;
        }

        // Perform an average of the current and previous Z velocity delta
        // as described by Verlet integration, which should reduce accumulated error
        if(!initialJump)
        {
            m_applyVelocityZ += (m_applyVelocityZCurrentDelta + m_applyVelocityZPrevDelta) / 2.f;
            m_applyVelocityZPrevDelta = m_applyVelocityZCurrentDelta;
        }
        else
        {
            m_applyVelocityZ += m_applyVelocityZCurrentDelta;
            m_applyVelocityZCurrentDelta = m_gravity * m_jumpHeldGravityFactor * deltaTime;
        }

        if(m_headHit && m_applyVelocityZ > 0.f && m_headHitSetsApogee)
            m_applyVelocityZ = m_applyVelocityZCurrentDelta = 0.f;

        // Account for the case where the PhysX Character Gameplay component's gravity is used instead
        if(m_gravity == 0.f && m_grounded)
        {
            AZ::Vector3 currentVelocity = AZ::Vector3::CreateZero();
            Physics::CharacterRequestBus::EventResult(currentVelocity, GetEntityId(),
                &Physics::CharacterRequestBus::Events::GetVelocity);

            // Reorient the applied "Z" velocity to the true Z axis
            if(m_velocityZPosDirection != AZ::Vector3::CreateAxisZ())
            {
                if(m_velocityZPosDirection.GetZ() >= 0.f)
                    currentVelocity = AZ::Quaternion::CreateShortestArc(m_velocityZPosDirection, AZ::Vector3::CreateAxisZ()).TransformVector(currentVelocity);
                else
                    currentVelocity = AZ::Quaternion::CreateShortestArc(m_velocityZPosDirection, AZ::Vector3::CreateAxisZ(-1.f)).TransformVector(-currentVelocity);
            }

            if(currentVelocity.GetZ() < 0.f)
                m_applyVelocityZ = m_applyVelocityZCurrentDelta = 0.f;
        }

        if(prevApplyVelocityZ > 0.f && m_applyVelocityZ <= 0.f)
        {
            if(m_velocityZPosDirection == AZ::Vector3::CreateAxisZ())
                m_fellFromHeight = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetZ();
            else
                m_fellFromHeight = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetProjected(m_velocityZPosDirection).GetLength();
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnJumpApogeeReached);
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStartedFalling);
        }
        else if(prevApplyVelocityZ == 0.f && m_applyVelocityZ < 0.f)
        {
            if(m_velocityZPosDirection == AZ::Vector3::CreateAxisZ())
                m_fellFromHeight = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetZ();
            else
                m_fellFromHeight = GetEntity()->GetTransform()->GetWorldTM().GetTranslation().GetProjected(m_velocityZPosDirection).GetLength();
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnStartedFalling);
        }

        // Debug print statements to observe the jump mechanic
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("First Person Controller Component", "Z Translation = %.10f", pos.GetZ());
        //AZ_Printf("First Person Controller Component", "m_applyVelocityZPrevDelta = %.10f", m_applyVelocityZPrevDelta);
        //AZ_Printf("First Person Controller Component", "m_applyVelocityZCurrentDelta = %.10f", m_applyVelocityZCurrentDelta);
        //AZ_Printf("First Person Controller Component", "m_applyVelocityZ = %.10f", m_applyVelocityZ);
        //AZ_Printf("First Person Controller Component", "m_grounded = %s", m_grounded ? "true" : "false");
        //AZ_Printf("First Person Controller Component", "m_jumpTimer = %.10f", m_jumpTimer);
        //AZ_Printf("First Person Controller Component", "deltaTime = %.10f", deltaTime);
        //AZ_Printf("First Person Controller Component", "m_jumpMaxHoldTime = %.10f", m_jumpMaxHoldTime);
        //AZ_Printf("First Person Controller Component", "m_jumpHoldDistance = %.10f", m_jumpHoldDistance);
        //AZ_Printf("First Person Controller Component", "dvz/dt = %.10f", (m_applyVelocityZ - prevApplyVelocityZ)/deltaTime);
        //AZ_Printf("First Person Controller Component","");
    }

    void FirstPersonControllerComponent::ProcessLinearImpulse(const float& deltaTime)
    {
        // Only apply impulses if it's enabled, allow any residual velocity from a previous impulse to decay
        if(!m_enableImpulses)
        {
            m_linearImpulse = AZ::Vector3::CreateZero();
            if(m_velocityFromImpulse.IsZero())
                return;
        }

        if(m_impulseDecelUsesFriction && !m_groundHits.empty())
            m_impulseConstantDecel = -1.f * m_gravity * GetSceneQueryHitDynamicFriction(m_groundHits.front());
        else if(m_groundHits.empty())
            m_impulseConstantDecel = 0.f;

        // Convert the linear impulse to a velocity based on the character's mass and accumulate it
        const AZ::Vector3 impulseVelocity = m_linearImpulse / m_characterMass;
        m_velocityFromImpulse += impulseVelocity;

        // When using a constant deceleration, calculate a new total lerp time when an impulse is applied or when the deceleration changes
        if(!impulseVelocity.IsZero() || m_impulsePrevConstantDecel != m_impulseConstantDecel)
        {
            m_initVelocityFromImpulse = m_velocityFromImpulse;
            m_impulseTotalLerpTime = m_initVelocityFromImpulse.GetLength() / m_impulseConstantDecel;
            m_impulseLerpTime = 0.f;
            m_impulsePrevConstantDecel = m_impulseConstantDecel;
        }

        // Decelerate using the linear damping value
        if(m_impulseLinearDamp != 0.f)
        {
            // This follows a first-order homogeneous linear recurrence relation, similar to Stokes' Law
            m_velocityFromImpulse *= (1 - m_impulseLinearDamp * deltaTime);
            if(m_impulseConstantDecel != 0.f)
            {
                m_initVelocityFromImpulse = m_velocityFromImpulse;
                m_impulseTotalLerpTime = m_initVelocityFromImpulse.GetLength() / m_impulseConstantDecel;
                m_impulseLerpTime = 0.f;
                m_impulsePrevConstantDecel = m_impulseConstantDecel;
            }
        }

        // Apply the velocity from the impulse
        m_applyVelocityXY += AZ::Vector2(m_velocityFromImpulse.GetX(), m_velocityFromImpulse.GetY());
        m_applyVelocityZ += m_velocityFromImpulse.GetZ();

        // Accumulate half of the deltaTime
        m_impulseLerpTime += deltaTime * 0.5f;

        // Decelerate at a constant rate
        // If the total lerp time is zero or the lerp time has reached the total lerp time then do not continue adding velocity
        if(m_impulseConstantDecel != 0.f && (m_impulseTotalLerpTime == 0.f || m_impulseLerpTime >= m_impulseTotalLerpTime))
        {
            m_impulseLerpTime = m_impulseTotalLerpTime;
            m_initVelocityFromImpulse = AZ::Vector3::CreateZero();
            m_velocityFromImpulse = AZ::Vector3::CreateZero();
            m_linearImpulse = AZ::Vector3::CreateZero();
            return;
        }
        // Set the applied velocity based on the time that was calculated for it to reach zero
        else if(m_impulseConstantDecel != 0.f)
            m_velocityFromImpulse = m_initVelocityFromImpulse.Lerp(AZ::Vector3::CreateZero(), m_impulseLerpTime / m_impulseTotalLerpTime);

        // Debug print statements to observe the acceleration and timing
        //static AZ::Vector3 prevVelocity = m_velocityFromImpulse;
        //AZ_Printf("First Person Controller Component", "dv/dt = %.10f", prevVelocity.GetDistance(m_velocityFromImpulse)/deltaTime);
        //prevVelocity = m_velocityFromImpulse;
        //AZ_Printf("First Person Controller Component", "m_impulseTotalLerpTime = %.10f", m_impulseTotalLerpTime);
        //AZ_Printf("First Person Controller Component", "m_impulseLerpTime = %.10f", m_impulseLerpTime);

        // Set the Z component of the velocity from the impulse to zero after it's applied
        m_velocityFromImpulse.SetZ(0.f);

        // Zero the impulse vector since it's been applied for this update
        m_linearImpulse = AZ::Vector3::CreateZero();

        // Accumulate half of the deltaTime if the total lerp time hasn't been reached
        if(m_impulseLerpTime != m_impulseTotalLerpTime)
            m_impulseLerpTime += deltaTime * 0.5f;
    }

    void FirstPersonControllerComponent::ProcessCharacterHits(const float& deltaTime)
    {
        if(!m_enableCharacterHits)
            return;

        // Create a capsule cast that will be used to detect when the character is hit
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Follow the character's transform
        AZ::Transform capsulePose = AZ::Transform::CreateIdentity();

        // Set the pose rotation based on the angle between the X axis and the m_sphereCastsAxisDirectionPose,
        // this was experimentally found to be necessary to get the capsule orientation correct
        capsulePose.SetRotation(AZ::Quaternion::CreateFromEulerAnglesRadians(GetVectorAnglesBetweenVectorsRadians(AZ::Vector3::CreateAxisX(), m_sphereCastsAxisDirectionPose)));

        // Set the translation and shift the capsule based on the character's capsule height
        capsulePose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + m_sphereCastsAxisDirectionPose * (m_capsuleCurrentHeight / 2.f));

        AzPhysics::ShapeCastRequest request = !m_applyVelocityXY.IsZero() || m_applyVelocityZ != 0.f || !m_currentVelocity.IsZero() ?
            AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsuleRadius * (1.f + m_hitRadiusPercentageIncrease / 100.f),
                m_capsuleCurrentHeight * (1.f + m_hitHeightPercentageIncrease / 100.f),
                capsulePose,
                AZ::Vector3(m_applyVelocityXY.GetX(), m_applyVelocityXY.GetY(), m_applyVelocityZ) + m_currentVelocity,
                ((AZ::Vector3(m_applyVelocityXY.GetX(), m_applyVelocityXY.GetY(), m_applyVelocityZ) + m_currentVelocity) * deltaTime).GetLength() + m_capsuleRadius * m_hitExtraProjectionPercentage / 100.f,
                m_characterHitBy,
                m_characterHitCollisionGroup,
                nullptr)
            :
            AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsuleRadius * (1.f + m_hitRadiusPercentageIncrease / 100.f),
                m_capsuleCurrentHeight * (1.f + m_hitHeightPercentageIncrease / 100.f),
                capsulePose,
                AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3::CreateAxisY()),
                0.001f,
                m_characterHitBy,
                m_characterHitCollisionGroup,
                nullptr);

        request.m_reportMultipleHits = true;

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        // Disregard intersections with the character's collider and its child entities
        auto selfChildEntityCheck = [this](AzPhysics::SceneQueryHit& hit)
            {
                if(hit.m_entityId == GetEntityId())
                    return true;

                // Obtain the child IDs if we don't already have them
                if(!m_obtainedChildIds)
                {
                    AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                    m_obtainedChildIds = true;
                }

                for(AZ::EntityId id: m_children)
                {
                    if(hit.m_entityId == id)
                        return true;
                }

                return false;
            };

        AZStd::erase_if(hits.m_hits, selfChildEntityCheck);

        m_characterHits = hits.m_hits;

        if(!m_characterHits.empty())
            FirstPersonControllerComponentNotificationBus::Broadcast(&FirstPersonControllerComponentNotificationBus::Events::OnCharacterShapecastHitSomething, m_characterHits);
    }

    // TiltVectorXCrossY will rotate any vector2 such that the cross product of its components becomes aligned
    // with the vector 3 that's provided. This is intentionally done without any rotation about the Z axis.
    AZ::Vector3 FirstPersonControllerComponent::TiltVectorXCrossY(const AZ::Vector2 vXY, const AZ::Vector3& newXCrossYDirection)
    {
        AZ::Vector3 tiltedXY = AZ::Vector3(vXY);

        if(!newXCrossYDirection.IsZero() && newXCrossYDirection != AZ::Vector3::CreateAxisZ())
        {
            if(newXCrossYDirection.GetZ() > 0.f)
            {
                AZ::Vector3 tiltedX = AZ::Vector3::CreateZero();
                if(newXCrossYDirection.GetX() >= 0.f)
                    tiltedX = AZ::Quaternion::CreateRotationY(AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisX(vXY.GetX()));
                else
                    tiltedX = AZ::Quaternion::CreateRotationY(-AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisX(vXY.GetX()));

                AZ::Vector3 tiltedY = AZ::Vector3::CreateZero();
                if(newXCrossYDirection.GetY() >= 0.f)
                    tiltedY = AZ::Quaternion::CreateRotationX(-AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisY(vXY.GetY()));
                else
                   tiltedY = AZ::Quaternion::CreateRotationX(AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisY(vXY.GetY()));

                tiltedXY = tiltedX + tiltedY;
            }
            else if(newXCrossYDirection.GetZ() < 0.f)
            {
                AZ::Vector3 tiltedX = AZ::Vector3::CreateZero();
                if(newXCrossYDirection.GetX() >= 0.f)
                    tiltedX = AZ::Quaternion::CreateRotationY(-AZ::Vector3::CreateAxisZ(-1.f).AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisX(-vXY.GetX()));
                else
                    tiltedX = AZ::Quaternion::CreateRotationY(AZ::Vector3::CreateAxisZ(-1.f).AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisX(-vXY.GetX()));

                AZ::Vector3 tiltedY = AZ::Vector3::CreateZero();
                if(newXCrossYDirection.GetY() >= 0.f)
                    tiltedY = AZ::Quaternion::CreateRotationX(AZ::Vector3::CreateAxisZ(-1.f).AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisY(vXY.GetY()));
                else
                   tiltedY = AZ::Quaternion::CreateRotationX(-AZ::Vector3::CreateAxisZ(-1.f).AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), newXCrossYDirection.GetZ()))).TransformVector(AZ::Vector3::CreateAxisY(vXY.GetY()));

                tiltedXY = tiltedX + tiltedY;
            }
            else
            {
                AZ::Vector3 tiltedX = AZ::Vector3::CreateAxisX(vXY.GetX());
                if(!AZ::IsClose(newXCrossYDirection.GetX(), 0.f))
                {
                    if(newXCrossYDirection.GetX() > 0.f)
                        tiltedX = AZ::Quaternion::CreateRotationY(AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, 0.f))).TransformVector(AZ::Vector3::CreateAxisX(vXY.GetX()));
                    else
                        tiltedX = AZ::Quaternion::CreateRotationY(AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(newXCrossYDirection.GetX(), 0.f, 0.f))).TransformVector(AZ::Vector3::CreateAxisX(-vXY.GetX()));
                }

                AZ::Vector3 tiltedY = AZ::Vector3::CreateAxisY(vXY.GetY());
                if(!AZ::IsClose(newXCrossYDirection.GetY(), 0.f))
                {
                    if(newXCrossYDirection.GetY() > 0.f)
                        tiltedY = AZ::Quaternion::CreateRotationX(-AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), 0.f))).TransformVector(AZ::Vector3::CreateAxisY(vXY.GetY()));
                    else
                        tiltedY = AZ::Quaternion::CreateRotationX(-AZ::Vector3::CreateAxisZ().AngleSafe(AZ::Vector3(0.f, newXCrossYDirection.GetY(), 0.f))).TransformVector(AZ::Vector3::CreateAxisY(-vXY.GetY()));
                }

                tiltedXY = tiltedX + tiltedY;
            }
        }

        return tiltedXY;
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime, const bool& timestepElseTick)
    {
        // Only update the rotation on each tick
        if(!timestepElseTick)
        {
            UpdateRotation();
            LerpCameraToCharacter(deltaTime);
        }

        // Keep track of the last two target velocity values for the obstruction check logic
        m_prevPrevTargetVelocity = m_prevTargetVelocity;

        // Handle motion on either the physics fixed timestep or the frame tick, depending on which is selected and which is currently executing
        if(timestepElseTick == m_addVelocityForTimestepVsTick)
        {
            CheckCharacterMovementObstructed();

            if(m_addVelocityForTimestepVsTick && m_cameraSmoothFollow && m_activeCameraEntity)
            {
                // Reset camera to latest physics translation before physics update
                ResetCameraToCharacter();
            }

            CheckGrounded(deltaTime);

            CrouchManager(deltaTime);

            // So long as the character is grounded or depending on how the update X&Y velocity while jumping
            // boolean values are set, and based on the state of jumping/falling, update the X&Y velocity accordingly
            if(m_grounded || (m_updateXYAscending && m_updateXYDescending && !m_updateXYOnlyNearGround)
               || ((m_updateXYAscending && m_applyVelocityZ >= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose))
               || ((m_updateXYDescending && m_applyVelocityZ <= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose)) )
            {
                UpdateVelocityXY(deltaTime);
            }

            UpdateVelocityZ(deltaTime);

            // Apply any linear impulses to the character that have been set via the EBus
            ProcessLinearImpulse(deltaTime);

            // Acquire the sum of the normal vectors for the velocity's XY plane if it's set
            AcquireSumOfGroundNormals();

            AZ::Vector3 addVelocityHeading = m_addVelocityHeading;
            // Rotate addVelocityHeading so it's with respect to the character's heading
            if(!addVelocityHeading.IsZero())
                addVelocityHeading = AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(m_addVelocityHeading);
            // Tilt the XY velocity plane based on m_velocityXCrossYDirection
            m_prevTargetVelocity = TiltVectorXCrossY((m_applyVelocityXY + AZ::Vector2(m_addVelocityWorld) + AZ::Vector2(addVelocityHeading)), m_velocityXCrossYDirection);

            // Calculate the walking up incline factor if it's enabled, otherwise set it to one
            if(m_velocityXCrossYTracksNormal && m_movingUpInclineSlowed && !m_prevTargetVelocityXY.IsZero())
                ApplyMovingUpInclineXYSpeedFactor();

            ProcessCharacterHits(deltaTime);

            // Change the +Z direction based on m_velocityZPosDirection
            m_prevTargetVelocity += (m_applyVelocityZ + m_addVelocityWorld.GetZ() + m_addVelocityHeading.GetZ()) * m_velocityZPosDirection;

            if(m_addVelocityForTimestepVsTick)
                Physics::CharacterRequestBus::Event(GetEntityId(),
                    &Physics::CharacterRequestBus::Events::AddVelocityForPhysicsTimestep,
                    m_prevTargetVelocity);
            else
                Physics::CharacterRequestBus::Event(GetEntityId(),
                    &Physics::CharacterRequestBus::Events::AddVelocityForTick,
                    m_prevTargetVelocity);
        }
    }

    // Event Notification methods for use in scripts
    void FirstPersonControllerComponent::OnPhysicsTimestepStart([[maybe_unused]] const float& timeStep){}
    void FirstPersonControllerComponent::OnPhysicsTimestepFinish([[maybe_unused]] const float& timeStep){}
    void FirstPersonControllerComponent::OnGroundHit([[maybe_unused]] const float& fellDistance){}
    void FirstPersonControllerComponent::OnGroundSoonHit([[maybe_unused]] const float& soonFellDistance){}
    void FirstPersonControllerComponent::OnUngrounded(){}
    void FirstPersonControllerComponent::OnStartedFalling(){}
    void FirstPersonControllerComponent::OnJumpApogeeReached(){}
    void FirstPersonControllerComponent::OnStartedMoving(){}
    void FirstPersonControllerComponent::OnTargetVelocityReached(){}
    void FirstPersonControllerComponent::OnStopped(){}
    void FirstPersonControllerComponent::OnTopWalkSpeedReached(){}
    void FirstPersonControllerComponent::OnTopSprintSpeedReached(){}
    void FirstPersonControllerComponent::OnHeadHit(){}
    void FirstPersonControllerComponent::OnCharacterShapecastHitSomething([[maybe_unused]] const AZStd::vector<AzPhysics::SceneQueryHit> characterHits){}
    void FirstPersonControllerComponent::OnVelocityXYObstructed(){}
    void FirstPersonControllerComponent::OnCharacterGravityObstructed(){}
    void FirstPersonControllerComponent::OnCrouched(){}
    void FirstPersonControllerComponent::OnStoodUp(){}
    void FirstPersonControllerComponent::OnStandPrevented(){}
    void FirstPersonControllerComponent::OnStartedCrouching(){}
    void FirstPersonControllerComponent::OnStartedStanding(){}
    void FirstPersonControllerComponent::OnFirstJump(){}
    void FirstPersonControllerComponent::OnFinalJump(){}
    void FirstPersonControllerComponent::OnStaminaCapped(){}
    void FirstPersonControllerComponent::OnStaminaReachedZero(){}
    void FirstPersonControllerComponent::OnSprintStarted(){}
    void FirstPersonControllerComponent::OnSprintStopped(){}
    void FirstPersonControllerComponent::OnCooldownStarted(){}
    void FirstPersonControllerComponent::OnCooldownDone(){}

    // Request Bus getter and setter methods for use in scripts
    AZ::EntityId FirstPersonControllerComponent::GetCharacterEntityId() const
    {
        return GetEntityId();
    }
    AZ::EntityId FirstPersonControllerComponent::GetActiveCameraEntityId() const
    {
        return m_activeCameraEntity->GetId();
    }
    AZ::Entity* FirstPersonControllerComponent::GetActiveCameraEntityPtr() const
    {
        if(m_activeCameraEntity)
        {
            return m_activeCameraEntity;
        }
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }
    void FirstPersonControllerComponent::SetCameraEntity(const AZ::EntityId new_cameraEntityId)
    {
        if(new_cameraEntityId != m_cameraEntityId)
        {
            AZ::EntityBus::Handler::BusDisconnect();
            m_cameraEntityId = new_cameraEntityId;
            m_activeCameraEntity = nullptr;
            m_obtainedChildIds = false;

            if(m_cameraEntityId.IsValid())
            {
                AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
                m_activeCameraEntity = GetEntityPtr(m_cameraEntityId);
                if(m_activeCameraEntity)
                {
                    InitializeCameraTranslation();
                    Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);
                    //AZ_Printf("First Person Controller Component", "Camera entity %s set and activated.",
                    //    m_activeCameraEntity->GetName().empty() ? m_cameraEntityId.ToString().c_str() : m_activeCameraEntity->GetName().c_str());
                }
                else
                {
                    AZ_Warning("First Person Controller Component", false,
                        "New camera entity ID %s is invalid.", m_cameraEntityId.ToString().c_str());
                    m_cameraEntityId = AZ::EntityId();
                }
            }
            // Update m_cameraParentEntityId to reflect the new camera's current parent
            if(m_activeCameraEntity)
            {
                AZ::EntityId parentId;
                AZ::TransformBus::EventResult(parentId, m_cameraEntityId, &AZ::TransformBus::Events::GetParentId);
                m_cameraParentEntityId = parentId;
            }
            else
            {
                m_cameraParentEntityId = AZ::EntityId();
            }
        }
    }
    bool FirstPersonControllerComponent::GetCameraSmoothFollow() const
    {
        return m_cameraSmoothFollow;
    }
    void FirstPersonControllerComponent::SetCameraSmoothFollow(const bool& new_cameraSmoothFollow)
    {
        if(m_cameraSmoothFollow != new_cameraSmoothFollow)
        {
            m_cameraSmoothFollow = new_cameraSmoothFollow;

            if(m_activeCameraEntity)
            {
                InitializeCameraTranslation();
            }
        }
    }
    // GetCameraNotSmoothFollow() is not exposed to the request bus, it's used for the visibility attribute in the editor
    bool FirstPersonControllerComponent::GetCameraNotSmoothFollow() const
    {
        return !m_cameraSmoothFollow;
    }
    void FirstPersonControllerComponent::SetParentChangeDoNotUpdate(const AZ::EntityId& entityId)
    {
        if(entityId.IsValid())
        {
            AZ::Entity* entity = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationBus::Events::FindEntity, entityId);
            if(entity)
            {
                AZ::TransformBus::Event(entityId, &AZ::TransformBus::Events::SetOnParentChangedBehavior,
                    AZ::OnParentChangedBehavior::DoNotUpdate);
                //AZ_Printf("First Person Controller Component", "Entity %s set to OnParentChangedBehavior::DoNotUpdate.",
                //    entity->GetName().empty() ? entityId.ToString().c_str() : entity->GetName().c_str());
            }
            else
            {
                AZ_Warning("First Person Controller Component", false,
                    "Entity ID %s is invalid for SetParentChangeDoNotUpdate.", entityId.ToString().c_str());
            }
        }
    }
    void FirstPersonControllerComponent::SetParentChangeUpdate(const AZ::EntityId& entityId)
    {
        if(entityId.IsValid())
        {
            AZ::Entity* entity = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationBus::Events::FindEntity, entityId);
            if(entity)
            {
                AZ::TransformBus::Event(entityId, &AZ::TransformBus::Events::SetOnParentChangedBehavior,
                    AZ::OnParentChangedBehavior::Update);
                //AZ_Printf("First Person Controller Component", "Entity %s reset to OnParentChangedBehavior::Update.",
                //    entity->GetName().empty() ? entityId.ToString().c_str() : entity->GetName().c_str());
            }
            else
            {
                AZ_Warning("First Person Controller Component", false,
                    "Entity ID %s is invalid for SetParentChangeUpdate.", entityId.ToString().c_str());
            }
        }
    }
    AZ::OnParentChangedBehavior FirstPersonControllerComponent::GetParentChangeBehavior(const AZ::EntityId& entityId) const
    {
        if(entityId.IsValid())
        {
            AZ::OnParentChangedBehavior behavior = AZ::OnParentChangedBehavior::Update;
            AZ::TransformBus::EventResult(behavior, entityId, &AZ::TransformBus::Events::GetOnParentChangedBehavior);
            return behavior;
        }
        AZ_Warning("First Person Controller Component", false,
            "Entity ID %s is invalid for GetParentChangeBehavior.", entityId.ToString().c_str());
        return AZ::OnParentChangedBehavior::Update;
    }
    float FirstPersonControllerComponent::GetEyeHeight() const
    {
        return m_eyeHeight;
    }
    void FirstPersonControllerComponent::SetEyeHeight(const float& new_eyeHeight)
    {
        m_eyeHeight = new_eyeHeight;
    }
    float FirstPersonControllerComponent::GetCameraLocalZTravelDistance() const
    {
        return m_cameraLocalZTravelDistance;
    }
    AZ::TransformInterface* FirstPersonControllerComponent::GetCameraRotationTransform() const
    {
        return m_cameraRotationTransform;
    }
    void FirstPersonControllerComponent::ReacquireChildEntityIds()
    {
        AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
    }
    AZStd::vector<AZ::EntityId> FirstPersonControllerComponent::GetChildEntityIds() const
    {
        return m_children;
    }
    void FirstPersonControllerComponent::ReacquireCapsuleDimensions()
    {
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetHeight);
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleRadius, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetRadius);

        if(m_crouchDistance > m_capsuleHeight - 2.f*m_capsuleRadius)
            SetCrouchDistance(m_capsuleHeight - 2.f*m_capsuleRadius);

        m_capsuleCurrentHeight = m_capsuleHeight;
    }
    void FirstPersonControllerComponent::ReacquireMaxSlopeAngle()
    {
        Physics::CharacterRequestBus::EventResult(m_maxGroundedAngleDegrees, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetSlopeLimitDegrees);

        // Set the max grounded angle to be slightly greater than the PhysX Character Controller's
        // maximum slope angle value in the editor
        m_maxGroundedAngleDegrees += 0.01f;

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::SetSlopeLimitDegrees, m_maxGroundedAngleDegrees);
    }
    AZStd::string FirstPersonControllerComponent::GetForwardEventName() const
    {
        return m_strForward;
    }
    void FirstPersonControllerComponent::SetForwardEventName(const AZStd::string& new_strForward)
    {
        m_strForward = new_strForward;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetForwardScale() const
    {
        return m_forwardScale;
    }
    void FirstPersonControllerComponent::SetForwardScale(const float& new_forwardScale)
    {
        m_forwardScale = new_forwardScale;
    }
    float FirstPersonControllerComponent::GetForwardInputValue() const
    {
        return m_forwardValue;
    }
    void FirstPersonControllerComponent::SetForwardInputValue(const float& new_forwardValue)
    {
        m_forwardValue = new_forwardValue;
    }
    AZStd::string FirstPersonControllerComponent::GetBackEventName() const
    {
        return m_strBack;
    }
    void FirstPersonControllerComponent::SetBackEventName(const AZStd::string& new_strBack)
    {
        m_strBack = new_strBack;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetBackScale() const
    {
        return m_backScale;
    }
    void FirstPersonControllerComponent::SetBackScale(const float& new_backScale)
    {
        m_backScale = new_backScale;
    }
    float FirstPersonControllerComponent::GetBackInputValue() const
    {
        return m_backValue;
    }
    void FirstPersonControllerComponent::SetBackInputValue(const float& new_backValue)
    {
        m_backValue = new_backValue;
    }
    AZStd::string FirstPersonControllerComponent::GetLeftEventName() const
    {
        return m_strLeft;
    }
    void FirstPersonControllerComponent::SetLeftEventName(const AZStd::string& new_strLeft)
    {
        m_strLeft = new_strLeft;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetLeftScale() const
    {
        return m_leftScale;
    }
    void FirstPersonControllerComponent::SetLeftScale(const float& new_leftScale)
    {
        m_leftScale = new_leftScale;
    }
    float FirstPersonControllerComponent::GetLeftInputValue() const
    {
        return m_leftValue;
    }
    void FirstPersonControllerComponent::SetLeftInputValue(const float& new_leftValue)
    {
        m_leftValue = new_leftValue;
    }
    AZStd::string FirstPersonControllerComponent::GetRightEventName() const
    {
        return m_strRight;
    }
    void FirstPersonControllerComponent::SetRightEventName(const AZStd::string& new_strRight)
    {
        m_strRight = new_strRight;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetRightScale() const
    {
        return m_rightScale;
    }
    void FirstPersonControllerComponent::SetRightScale(const float& new_rightScale)
    {
        m_rightScale = new_rightScale;
    }
    float FirstPersonControllerComponent::GetRightInputValue() const
    {
        return m_rightValue;
    }
    void FirstPersonControllerComponent::SetRightInputValue(const float& new_rightValue)
    {
        m_rightValue = new_rightValue;
    }
    AZStd::string FirstPersonControllerComponent::GetYawEventName() const
    {
        return m_strYaw;
    }
    void FirstPersonControllerComponent::SetYawEventName(const AZStd::string& new_strYaw)
    {
        m_strYaw = new_strYaw;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetYawInputValue() const
    {
        return m_yawValue;
    }
    void FirstPersonControllerComponent::SetYawInputValue(const float& new_yawValue)
    {
        m_yawValue = new_yawValue;
    }
    AZStd::string FirstPersonControllerComponent::GetPitchEventName() const
    {
        return m_strPitch;
    }
    void FirstPersonControllerComponent::SetPitchEventName(const AZStd::string& new_strPitch)
    {
        m_strPitch = new_strPitch;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetPitchInputValue() const
    {
        return m_pitchValue;
    }
    void FirstPersonControllerComponent::SetPitchInputValue(const float& new_pitchValue)
    {
        m_pitchValue = new_pitchValue;
    }
    AZStd::string FirstPersonControllerComponent::GetSprintEventName() const
    {
        return m_strSprint;
    }
    void FirstPersonControllerComponent::SetSprintEventName(const AZStd::string& new_strSprint)
    {
        m_strSprint = new_strSprint;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetSprintInputValue() const
    {
        return m_sprintValue;
    }
    void FirstPersonControllerComponent::SetSprintInputValue(const float& new_sprintValue)
    {
        m_sprintValue = new_sprintValue;
    }
    AZStd::string FirstPersonControllerComponent::GetCrouchEventName() const
    {
        return m_strCrouch;
    }
    void FirstPersonControllerComponent::SetCrouchEventName(const AZStd::string& new_strCrouch)
    {
        m_strCrouch = new_strCrouch;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetCrouchInputValue() const
    {
        return m_crouchValue;
    }
    void FirstPersonControllerComponent::SetCrouchInputValue(const float& new_crouchValue)
    {
        m_crouchValue = new_crouchValue;
    }
    AZStd::string FirstPersonControllerComponent::GetJumpEventName() const
    {
        return m_strJump;
    }
    void FirstPersonControllerComponent::SetJumpEventName(const AZStd::string& new_strJump)
    {
        m_strJump = new_strJump;
        AssignConnectInputEvents();
    }
    float FirstPersonControllerComponent::GetJumpInputValue() const
    {
        return m_jumpValue;
    }
    void FirstPersonControllerComponent::SetJumpInputValue(const float& new_jumpValue)
    {
        m_jumpValue = new_jumpValue;
    }
    bool FirstPersonControllerComponent::GetGrounded() const
    {
        return m_grounded;
    }
    void FirstPersonControllerComponent::SetGroundedForTick(const bool& new_grounded)
    {
        m_scriptGrounded = new_grounded;
        m_scriptSetGroundTick = true;
    }
    AzPhysics::SceneQueryHits FirstPersonControllerComponent::GetGroundSceneQueryHits() const
    {
        AzPhysics::SceneQueryHits groundHits;
        groundHits.m_hits = m_groundHits;
        return groundHits;
    }
    AzPhysics::SceneQueryHits FirstPersonControllerComponent::GetGroundCloseSceneQueryHits() const
    {
        AzPhysics::SceneQueryHits groundCloseHits;
        groundCloseHits.m_hits = m_groundCloseHits;
        return groundCloseHits;
    }
    AzPhysics::SceneQueryHits FirstPersonControllerComponent::GetGroundCloseCoyoteTimeSceneQueryHits() const
    {
        AzPhysics::SceneQueryHits groundCloseCoyoteTimeHits;
        groundCloseCoyoteTimeHits.m_hits = m_groundCloseCoyoteTimeHits;
        return groundCloseCoyoteTimeHits;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetGroundSumNormalsDirection() const
    {
        if(m_groundHits.empty())
            return AZ::Vector3::CreateAxisZ();
        AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
        for(AzPhysics::SceneQueryHit hit: m_groundHits)
            sumNormals += hit.m_normal;
        return sumNormals.GetNormalized();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetGroundCloseSumNormalsDirection() const
    {
        if(m_groundCloseHits.empty())
            return AZ::Vector3::CreateZero();
        AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
        for(AzPhysics::SceneQueryHit hit: m_groundCloseHits)
            sumNormals += hit.m_normal;
        return sumNormals.GetNormalized();
    }
    AzPhysics::SceneQuery::ResultFlags FirstPersonControllerComponent::GetSceneQueryHitResultFlags(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_resultFlags;
    }
    AZ::EntityId FirstPersonControllerComponent::GetSceneQueryHitEntityId(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_entityId;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetSceneQueryHitNormal(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_normal;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetSceneQueryHitPosition(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_position;
    }
    float FirstPersonControllerComponent::GetSceneQueryHitDistance(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_distance;
    }
    Physics::MaterialId FirstPersonControllerComponent::GetSceneQueryHitMaterialId(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_physicsMaterialId;
    }
    AZStd::shared_ptr<Physics::Material> FirstPersonControllerComponent::GetSceneQueryHitMaterialPtr(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_shape->GetMaterial();
    }
    AZ::Data::Asset<Physics::MaterialAsset> FirstPersonControllerComponent::GetSceneQueryHitMaterialAsset(AzPhysics::SceneQueryHit hit) const
    {
        AZStd::shared_ptr<Physics::Material> material =
            AZStd::rtti_pointer_cast<Physics::Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(hit.m_physicsMaterialId));

        return material->GetMaterialAsset();
    }
    AZ::Data::AssetId FirstPersonControllerComponent::GetSceneQueryHitMaterialAssetId(AzPhysics::SceneQueryHit hit) const
    {
        AZStd::shared_ptr<Physics::Material> material =
            AZStd::rtti_pointer_cast<Physics::Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(hit.m_physicsMaterialId));

        return material->GetMaterialAsset().GetId();
    }
    float FirstPersonControllerComponent::GetSceneQueryHitDynamicFriction(AzPhysics::SceneQueryHit hit) const
    {
        AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset =
            AZStd::rtti_pointer_cast<Physics::Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(hit.m_physicsMaterialId))->GetMaterialAsset();

        AZStd::shared_ptr<PhysX::Material> physxMaterial = AZStd::rtti_pointer_cast<PhysX::Material>(
            AZ::Interface<Physics::MaterialManager>::Get()->FindOrCreateMaterial(
                hit.m_physicsMaterialId,
                physicsMaterialAsset));

        return physxMaterial->GetProperty("DynamicFriction").GetValue<float>();
    }
    float FirstPersonControllerComponent::GetSceneQueryHitStaticFriction(AzPhysics::SceneQueryHit hit) const
    {
        AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset =
            AZStd::rtti_pointer_cast<Physics::Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(hit.m_physicsMaterialId))->GetMaterialAsset();

        AZStd::shared_ptr<PhysX::Material> physxMaterial = AZStd::rtti_pointer_cast<PhysX::Material>(
            AZ::Interface<Physics::MaterialManager>::Get()->FindOrCreateMaterial(
                hit.m_physicsMaterialId,
                physicsMaterialAsset));

        return physxMaterial->GetProperty("StaticFriction").GetValue<float>();
    }
    float FirstPersonControllerComponent::GetSceneQueryHitRestitution(AzPhysics::SceneQueryHit hit) const
    {
        AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset =
            AZStd::rtti_pointer_cast<Physics::Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(hit.m_physicsMaterialId))->GetMaterialAsset();

        AZStd::shared_ptr<PhysX::Material> physxMaterial = AZStd::rtti_pointer_cast<PhysX::Material>(
            AZ::Interface<Physics::MaterialManager>::Get()->FindOrCreateMaterial(
                hit.m_physicsMaterialId,
                physicsMaterialAsset));

        return physxMaterial->GetProperty("Restitution").GetValue<float>();
    }
    Physics::Shape* FirstPersonControllerComponent::GetSceneQueryHitShapePtr(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_shape;
    }
    bool FirstPersonControllerComponent::GetSceneQueryHitIsInGroupName(AzPhysics::SceneQueryHit hit, AZStd::string groupName) const
    {
        bool success = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(success, &Physics::CollisionRequests::TryGetCollisionGroupByName, groupName, collisionGroup);
        if(success)
            return collisionGroup.IsSet(hit.m_shape->GetCollisionLayer());
        else
            return false;
    }
    AzPhysics::SimulatedBodyHandle FirstPersonControllerComponent::GetSceneQueryHitSimulatedBodyHandle(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_bodyHandle;
    }
    bool FirstPersonControllerComponent::GetLayerNameIsInGroupName(AZStd::string layerName, AZStd::string groupName) const
    {
        bool groupSuccess = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(groupSuccess, &Physics::CollisionRequests::TryGetCollisionGroupByName, groupName, collisionGroup);
        if(groupSuccess)
        {
            bool layerSuccess = false;
            AzPhysics::CollisionLayer collisionLayer;
            Physics::CollisionRequestBus::BroadcastResult(
                layerSuccess, &Physics::CollisionRequests::TryGetCollisionLayerByName, layerName, collisionLayer);
            if(layerSuccess)
                return collisionGroup.IsSet(collisionLayer);
            else return false;
        }
        else
            return false;
    }
    bool FirstPersonControllerComponent::GetGroundClose() const
    {
        return m_groundClose;
    }
    void FirstPersonControllerComponent::SetGroundCloseForTick(const bool& new_groundClose)
    {
        m_scriptGroundClose = new_groundClose;
        m_scriptSetGroundCloseTick = true;
    }
    AZStd::string FirstPersonControllerComponent::GetGroundedCollisionGroupName() const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, m_groundedCollisionGroup);
        return groupName;
    }
    void FirstPersonControllerComponent::SetGroundedCollisionGroup(const AZStd::string& new_groundedCollisionGroupName)
    {
        bool success = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(success, &Physics::CollisionRequests::TryGetCollisionGroupByName, new_groundedCollisionGroupName, collisionGroup);
        if(success)
        {
            m_groundedCollisionGroup = collisionGroup;
            const AzPhysics::CollisionConfiguration& configuration = AZ::Interface<AzPhysics::SystemInterface>::Get()->GetConfiguration()->m_collisionConfig;
            m_groundedCollisionGroupId = configuration.m_collisionGroups.FindGroupIdByName(new_groundedCollisionGroupName);
        }
    }
    float FirstPersonControllerComponent::GetAirTime() const
    {
        return m_airTime;
    }
    float FirstPersonControllerComponent::GetGravity() const
    {
        return m_gravity;
    }
    void FirstPersonControllerComponent::SetGravity(const float& new_gravity)
    {
        m_gravity = new_gravity;
        UpdateJumpMaxHoldTime();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetPrevTargetVelocityWorld() const
    {
        return m_prevTargetVelocity;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetPrevTargetVelocityHeading() const
    {
        return AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(m_prevTargetVelocity);
    }
    float FirstPersonControllerComponent::GetVelocityCloseTolerance() const
    {
        return m_velocityCloseTolerance;
    }
    void FirstPersonControllerComponent::SetVelocityCloseTolerance(const float& new_velocityCloseTolerance)
    {
        m_velocityCloseTolerance = new_velocityCloseTolerance;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVelocityXCrossYDirection() const
    {
        return m_velocityXCrossYDirection;
    }
    void FirstPersonControllerComponent::SetVelocityXCrossYDirection(const AZ::Vector3& new_velocityXCrossYDirection)
    {
        m_velocityXCrossYDirection = new_velocityXCrossYDirection.GetNormalized();
        if(m_velocityXCrossYDirection.IsZero())
            m_velocityXCrossYDirection = AZ::Vector3::CreateAxisZ();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVelocityZPosDirection() const
    {
        return m_velocityZPosDirection;
    }
    void FirstPersonControllerComponent::SetVelocityZPosDirection(const AZ::Vector3& new_velocityZPosDirection)
    {
        m_velocityZPosDirection = new_velocityZPosDirection.GetNormalized();
        if(m_velocityZPosDirection.IsZero())
            m_velocityZPosDirection = AZ::Vector3::CreateAxisZ();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetSphereCastsAxisDirectionPose() const
    {
        return m_sphereCastsAxisDirectionPose;
    }
    void FirstPersonControllerComponent::SetSphereCastsAxisDirectionPose(const AZ::Vector3& new_sphereCastsAxisDirectionPose)
    {
        m_sphereCastsAxisDirectionPose = new_sphereCastsAxisDirectionPose;
        if(m_sphereCastsAxisDirectionPose.IsZero())
            m_sphereCastsAxisDirectionPose = AZ::Vector3::CreateAxisZ();

        // Set the character up direction using the new spherecast direction
        Physics::CharacterRequestBus::Event(GetEntityId(),
             &Physics::CharacterRequestBus::Events::SetUpDirection, m_sphereCastsAxisDirectionPose);
    }
    bool FirstPersonControllerComponent::GetVelocityXCrossYTracksNormal() const
    {
        return m_velocityXCrossYTracksNormal;
    }
    void FirstPersonControllerComponent::SetVelocityXCrossYTracksNormal(const bool& new_velocityXCrossYTracksNormal)
    {
        m_velocityXCrossYTracksNormal = new_velocityXCrossYTracksNormal;
    }
    bool FirstPersonControllerComponent::GetSpeedReducedWhenMovingUpInclines() const
    {
        return m_movingUpInclineSlowed;
    }
    void FirstPersonControllerComponent::SetSpeedReducedWhenMovingUpInclines(const bool& new_movingUpInclineSlowed)
    {
        m_movingUpInclineSlowed = new_movingUpInclineSlowed;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVectorAnglesBetweenVectorsRadians(const AZ::Vector3& v1, const AZ::Vector3& v2)
    {
        if(v1 == v2)
            return AZ::Vector3::CreateZero();
        if(v1 == -v2)
            return AZ::Vector3(AZ::Constants::Pi, AZ::Constants::Pi, AZ::Constants::Pi);
        AZ::Vector3 angle = AZ::Vector3::CreateZero();
        if(v1.AngleSafe(v2) <= AZ::Constants::HalfPi)
            angle = AZ::Quaternion::CreateShortestArc(v1, v2).GetEulerRadians();
        else
            angle = AZ::Quaternion::CreateShortestArc(v1, -v2).GetEulerRadians();
        return angle;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVectorAnglesBetweenVectorsDegrees(const AZ::Vector3& v1, const AZ::Vector3& v2)
    {
        AZ::Vector3 angle = GetVectorAnglesBetweenVectorsRadians(v1, v2);
        if(angle.IsZero())
            return angle;
        angle.SetX(angle.GetX() * 360.f/AZ::Constants::TwoPi);
        angle.SetY(angle.GetY() * 360.f/AZ::Constants::TwoPi);
        angle.SetZ(angle.GetZ() * 360.f/AZ::Constants::TwoPi);
        return angle;
    }
    float FirstPersonControllerComponent::GetJumpHeldGravityFactor() const
    {
        return m_jumpHeldGravityFactor;
    }
    void FirstPersonControllerComponent::SetJumpHeldGravityFactor(const float& new_jumpHeldGravityFactor)
    {
        m_jumpHeldGravityFactor = new_jumpHeldGravityFactor;
        UpdateJumpMaxHoldTime();
    }
    float FirstPersonControllerComponent::GetJumpFallingGravityFactor() const
    {
        return m_jumpFallingGravityFactor;
    }
    void FirstPersonControllerComponent::SetJumpFallingGravityFactor(const float& new_jumpFallingGravityFactor)
    {
        m_jumpFallingGravityFactor = new_jumpFallingGravityFactor;
    }
    float FirstPersonControllerComponent::GetJumpAccelFactor() const
    {
        return m_jumpAccelFactor;
    }
    void FirstPersonControllerComponent::SetJumpAccelFactor(const float& new_jumpAccelFactor)
    {
        m_jumpAccelFactor = new_jumpAccelFactor;
    }
    bool FirstPersonControllerComponent::GetUpdateXYAscending() const
    {
        return m_updateXYAscending;
    }
    void FirstPersonControllerComponent::SetUpdateXYAscending(const bool& new_updateXYAscending)
    {
        m_updateXYAscending = new_updateXYAscending;
    }
    bool FirstPersonControllerComponent::GetUpdateXYDescending() const
    {
        return m_updateXYDescending;
    }
    void FirstPersonControllerComponent::SetUpdateXYDescending(const bool& new_updateXYDescending)
    {
        m_updateXYDescending = new_updateXYDescending;
    }
    bool FirstPersonControllerComponent::GetUpdateXYOnlyNearGround() const
    {
        return m_updateXYOnlyNearGround;
    }
    void FirstPersonControllerComponent::SetUpdateXYOnlyNearGround(const bool& new_updateXYOnlyNearGround)
    {
        m_updateXYOnlyNearGround = new_updateXYOnlyNearGround;
    }
    bool FirstPersonControllerComponent::GetAddVelocityForTimestepVsTick() const
    {
        return m_addVelocityForTimestepVsTick;
    }
    void FirstPersonControllerComponent::SetAddVelocityForTimestepVsTick(const bool& new_addVelocityForTimestepVsTick)
    {
        m_addVelocityForTimestepVsTick = new_addVelocityForTimestepVsTick;

        if(m_addVelocityForTimestepVsTick)
        {
            Physics::DefaultWorldBus::BroadcastResult(m_attachedSceneHandle, &Physics::DefaultWorldRequests::GetDefaultSceneHandle);
            if(m_attachedSceneHandle == AzPhysics::InvalidSceneHandle)
            {
                AZ_Error("First Person Controller Component", false, "Failed to retrieve default scene.");
                return;
            }

            m_sceneSimulationStartHandler = AzPhysics::SceneEvents::OnSceneSimulationStartHandler(
                [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, float fixedDeltaTime)
                {
                    OnSceneSimulationStart(fixedDeltaTime);
                }, aznumeric_cast<int32_t>(AzPhysics::SceneEvents::PhysicsStartFinishSimulationPriority::Physics));

            m_sceneSimulationFinishHandler = AzPhysics::SceneEvents::OnSceneSimulationFinishHandler(
                [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, [[maybe_unused]] float fixedDeltaTime)
                {
                    OnSceneSimulationFinish(fixedDeltaTime);
                }, aznumeric_cast<int32_t>(AzPhysics::SceneEvents::PhysicsStartFinishSimulationPriority::Physics));

            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            if(sceneInterface != nullptr)
            {
                sceneInterface->RegisterSceneSimulationStartHandler(m_attachedSceneHandle, m_sceneSimulationStartHandler);
            }
        }
        else
        {
            m_attachedSceneHandle = AzPhysics::InvalidSceneHandle;
            m_sceneSimulationStartHandler.Disconnect();
        }
    }
    float FirstPersonControllerComponent::GetPhysicsTimestepScaleFactor() const
    {
        return m_physicsTimestepScaleFactor;
    }
    void FirstPersonControllerComponent::SetPhysicsTimestepScaleFactor(const float& new_physicsTimestepScaleFactor)
    {
        m_physicsTimestepScaleFactor = new_physicsTimestepScaleFactor;
    }
    bool FirstPersonControllerComponent::GetScriptSetsTargetVelocityXY() const
    {
        return m_scriptSetsTargetVelocityXY;
    }
    void FirstPersonControllerComponent::SetScriptSetsTargetVelocityXY(const bool& new_scriptSetsTargetVelocityXY)
    {
        m_scriptSetsTargetVelocityXY = new_scriptSetsTargetVelocityXY;
    }
    AZ::Vector2 FirstPersonControllerComponent::GetTargetVelocityXY() const
    {
        return m_scriptTargetVelocityXY;
    }
    void FirstPersonControllerComponent::SetTargetVelocityXY(const AZ::Vector2& new_scriptTargetVelocityXY)
    {
        m_scriptTargetVelocityXY = new_scriptTargetVelocityXY;
    }
    AZ::Vector2 FirstPersonControllerComponent::GetTargetVelocityXYWorld() const
    {
        return AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3(m_scriptTargetVelocityXY)));
    }
    AZ::Vector2 FirstPersonControllerComponent::GetCorrectedVelocityXY() const
    {
        return m_correctedVelocityXY;
    }
    void FirstPersonControllerComponent::SetCorrectedVelocityXY(const AZ::Vector2& new_correctedVelocityXY)
    {
        m_velocityXYObstructed = true;
        m_correctedVelocityXY = new_correctedVelocityXY;
    }
    float FirstPersonControllerComponent::GetCorrectedVelocityZ() const
    {
        return m_correctedVelocityZ;
    }
    void FirstPersonControllerComponent::SetCorrectedVelocityZ(const float& new_correctedVelocityZ)
    {
        m_velocityXYObstructed = true;
        m_correctedVelocityZ = new_correctedVelocityZ;
    }
    AZ::Vector2 FirstPersonControllerComponent::GetApplyVelocityXY() const
    {
        return m_applyVelocityXY;
    }
    void FirstPersonControllerComponent::SetApplyVelocityXY(const AZ::Vector2& new_applyVelocityXY)
    {
        m_applyVelocityXY = new_applyVelocityXY;
        if(m_instantVelocityRotation)
            m_prevApplyVelocityXY = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocityXY)));
        else
            m_prevApplyVelocityXY = m_applyVelocityXY;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetAddVelocityWorld() const
    {
        return m_addVelocityWorld;
    }
    void FirstPersonControllerComponent::SetAddVelocityWorld(const AZ::Vector3& new_addVelocityWorld)
    {
        m_addVelocityWorld = new_addVelocityWorld;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetAddVelocityHeading() const
    {
        return m_addVelocityHeading;
    }
    void FirstPersonControllerComponent::SetAddVelocityHeading(const AZ::Vector3& new_addVelocityHeading)
    {
        m_addVelocityHeading = new_addVelocityHeading;
    }
    float FirstPersonControllerComponent::GetApplyVelocityZ() const
    {
        return m_applyVelocityZ;
    }
    void FirstPersonControllerComponent::SetApplyVelocityZ(const float& new_applyVelocityZ)
    {
        SetGroundedForTick(false);
        m_applyVelocityZ = new_applyVelocityZ;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetLinearImpulse() const
    {
        return m_linearImpulse;
    }
    void FirstPersonControllerComponent::SetLinearImpulse(const AZ::Vector3& new_linearImpulse)
    {
        m_linearImpulse = new_linearImpulse;
    }
    void FirstPersonControllerComponent::ApplyLinearImpulse(const AZ::Vector3& new_addLinearImpulse)
    {
        m_linearImpulse += new_addLinearImpulse;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetInitVelocityFromImpulse() const
    {
        return m_initVelocityFromImpulse;
    }
    void FirstPersonControllerComponent::SetInitVelocityFromImpulse(const AZ::Vector3& new_initVelocityFromImpulse)
    {
        m_initVelocityFromImpulse = new_initVelocityFromImpulse;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVelocityFromImpulse() const
    {
        return m_velocityFromImpulse;
    }
    void FirstPersonControllerComponent::SetVelocityFromImpulse(const AZ::Vector3& new_velocityFromImpulse)
    {
        m_velocityFromImpulse = new_velocityFromImpulse;
    }
    bool FirstPersonControllerComponent::GetEnableImpulses() const
    {
        return m_enableImpulses;
    }
    void FirstPersonControllerComponent::SetEnableImpulses(const bool& new_enableImpulses)
    {
        m_enableImpulses = new_enableImpulses;
        if(!m_enableImpulses)
            m_linearImpulse = AZ::Vector3::CreateZero();
    }
    // GetEnableImpulsesAndNotDecelUsesFriction() is not exposed to the request bus, it's used for the visibility attribute in the editor
    bool FirstPersonControllerComponent::GetEnableImpulsesAndNotDecelUsesFriction() const
    {
        return GetEnableImpulses() && !m_impulseDecelUsesFriction;
    }
    bool FirstPersonControllerComponent::GetImpulseDecelUsesFriction() const
    {
        return m_impulseDecelUsesFriction;
    }
    void FirstPersonControllerComponent::SetImpulseDecelUsesFriction(const bool& new_impulseDecelUsesFriction)
    {
        m_impulseDecelUsesFriction = new_impulseDecelUsesFriction;
    }
    float FirstPersonControllerComponent::GetImpulseLinearDamp() const
    {
        return m_impulseLinearDamp;
    }
    void FirstPersonControllerComponent::SetImpulseLinearDamp(const float& new_impulseLinearDamp)
    {
        m_impulseLinearDamp = AZ::GetMax(new_impulseLinearDamp, 0.f);
    }
    float FirstPersonControllerComponent::GetImpulseConstantDecel() const
    {
        return m_impulseConstantDecel;
    }
    void FirstPersonControllerComponent::SetImpulseConstantDecel(const float& new_impulseConstantDecel)
    {
        m_impulseConstantDecel = AZ::GetMax(new_impulseConstantDecel, 0.f);
    }
    float FirstPersonControllerComponent::GetImpulseTotalLerpTime() const
    {
        return m_impulseTotalLerpTime;
    }
    void FirstPersonControllerComponent::SetImpulseTotalLerpTime(const float& new_impulseTotalLerpTime)
    {
        m_impulseTotalLerpTime = AZ::GetMax(new_impulseTotalLerpTime, 0.f);
    }
    float FirstPersonControllerComponent::GetImpulseLerpTime() const
    {
        return m_impulseLerpTime;
    }
    void FirstPersonControllerComponent::SetImpulseLerpTime(const float& new_impulseLerpTime)
    {
        m_impulseLerpTime = AZ::GetMax(new_impulseLerpTime, 0.f);
    }
    float FirstPersonControllerComponent::GetCharacterMass() const
    {
        return m_characterMass;
    }
    void FirstPersonControllerComponent::SetCharacterMass(const float& new_characterMass)
    {
        m_characterMass = AZ::GetMax(new_characterMass, 0.f);
    }
    bool FirstPersonControllerComponent::GetEnableCharacterHits() const
    {
        return m_enableCharacterHits;
    }
    void FirstPersonControllerComponent::SetEnableCharacterHits(const bool& new_enableCharacterHits)
    {
        m_enableCharacterHits = new_enableCharacterHits;
        if(!m_enableCharacterHits)
            m_characterHits.clear();
    }
    float FirstPersonControllerComponent::GetHitRadiusPercentageIncrease() const
    {
        return m_hitRadiusPercentageIncrease;
    }
    void FirstPersonControllerComponent::SetHitRadiusPercentageIncrease(const float& new_hitRadiusPercentageIncrease)
    {
        m_hitRadiusPercentageIncrease = new_hitRadiusPercentageIncrease;
    }
    float FirstPersonControllerComponent::GetHitHeightPercentageIncrease() const
    {
        return m_hitHeightPercentageIncrease;
    }
    void FirstPersonControllerComponent::SetHitHeightPercentageIncrease(const float& new_hitHeightPercentageIncrease)
    {
        m_hitHeightPercentageIncrease = new_hitHeightPercentageIncrease;
    }
    float FirstPersonControllerComponent::GetHitExtraProjectionPercentage() const
    {
        return m_hitExtraProjectionPercentage;
    }
    void FirstPersonControllerComponent::SetHitExtraProjectionPercentage(const float& new_hitExtraProjectionPercentage)
    {
        m_hitExtraProjectionPercentage = new_hitExtraProjectionPercentage;
    }
    AZStd::string FirstPersonControllerComponent::GetCharacterHitCollisionGroupName() const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, m_characterHitCollisionGroup);
        return groupName;
    }
    void FirstPersonControllerComponent::SetCharacterHitCollisionGroupByName(const AZStd::string& new_characterHitCollisionGroupName)
    {
        bool success = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(success, &Physics::CollisionRequests::TryGetCollisionGroupByName, new_characterHitCollisionGroupName, collisionGroup);
        if(success)
        {
            m_characterHitCollisionGroup = collisionGroup;
            const AzPhysics::CollisionConfiguration& configuration = AZ::Interface<AzPhysics::SystemInterface>::Get()->GetConfiguration()->m_collisionConfig;
            m_characterHitCollisionGroupId = configuration.m_collisionGroups.FindGroupIdByName(new_characterHitCollisionGroupName);
        }
    }
    AzPhysics::SceneQuery::QueryType FirstPersonControllerComponent::GetCharacterHitBy() const
    {
        // 0 = Static
        // 1 = Dynamic
        // 2 = StaticAndDynamic
        return m_characterHitBy;
    }
    void FirstPersonControllerComponent::SetCharacterHitBy(const AzPhysics::SceneQuery::QueryType& new_characterHitBy)
    {
        // 0 = Static
        // 1 = Dynamic
        // 2 = StaticAndDynamic
        m_characterHitBy = new_characterHitBy;
    }
    AzPhysics::SceneQueryHits FirstPersonControllerComponent::GetCharacterSceneQueryHits() const
    {
        AzPhysics::SceneQueryHits characterHits;
        characterHits.m_hits = m_characterHits;
        return characterHits;
    }
    float FirstPersonControllerComponent::GetJumpInitialVelocity() const
    {
        return m_jumpInitialVelocity;
    }
    void FirstPersonControllerComponent::SetJumpInitialVelocity(const float& new_jumpInitialVelocity)
    {
        m_jumpInitialVelocity = new_jumpInitialVelocity;
        UpdateJumpMaxHoldTime();
    }
    float FirstPersonControllerComponent::GetJumpSecondInitialVelocity() const
    {
        return m_jumpSecondInitialVelocity;
    }
    void FirstPersonControllerComponent::SetJumpSecondInitialVelocity(const float& new_jumpSecondInitialVelocity)
    {
        m_jumpSecondInitialVelocity = new_jumpSecondInitialVelocity;
    }
    bool FirstPersonControllerComponent::GetJumpReqRepress() const
    {
        return m_jumpReqRepress;
    }
    void FirstPersonControllerComponent::SetJumpReqRepress(const bool& new_jumpReqRepress)
    {
        m_jumpReqRepress = new_jumpReqRepress;
    }
    bool FirstPersonControllerComponent::GetJumpHeld() const
    {
        return m_jumpHeld;
    }
    void FirstPersonControllerComponent::SetJumpHeld(const bool& new_jumpHeld)
    {
        m_jumpHeld = new_jumpHeld;
    }
    bool FirstPersonControllerComponent::GetDoubleJump() const
    {
        return m_doubleJumpEnabled;
    }
    void FirstPersonControllerComponent::SetDoubleJump(const bool& new_doubleJumpEnabled)
    {
        m_doubleJumpEnabled = new_doubleJumpEnabled;
    }
    bool FirstPersonControllerComponent::GetFinalJumpPerformed() const
    {
        return m_doubleJumpEnabled;
    }
    void FirstPersonControllerComponent::SetFinalJumpPerformed(const bool& new_finalJump)
    {
        m_finalJump = new_finalJump;
    }
    float FirstPersonControllerComponent::GetGroundedOffset() const
    {
        return m_groundedSphereCastOffset;
    }
    void FirstPersonControllerComponent::SetGroundedOffset(const float& new_groundedSphereCastOffset)
    {
        m_groundedSphereCastOffset = new_groundedSphereCastOffset;
    }
    float FirstPersonControllerComponent::GetGroundCloseOffset() const
    {
        return m_groundCloseSphereCastOffset;
    }
    void FirstPersonControllerComponent::SetGroundCloseOffset(const float& new_groundCloseSphereCastOffset)
    {
        m_groundCloseSphereCastOffset = new_groundCloseSphereCastOffset;
    }
    float FirstPersonControllerComponent::GetGroundCloseCoyoteTimeOffset() const
    {
        return m_groundCloseCoyoteTimeOffset;
    }
    void FirstPersonControllerComponent::SetGroundCloseCoyoteTimeOffset(const float& new_groundCloseCoyoteTimeOffset)
    {
        m_groundCloseCoyoteTimeOffset = new_groundCloseCoyoteTimeOffset;
    }
    float FirstPersonControllerComponent::GetJumpHoldDistance() const
    {
        return m_jumpHoldDistance;
    }
    void FirstPersonControllerComponent::SetJumpHoldDistance(const float& new_jumpHoldDistance)
    {
        m_jumpHoldDistance = new_jumpHoldDistance;
        UpdateJumpMaxHoldTime();
    }
    float FirstPersonControllerComponent::GetJumpHeadSphereCastOffset() const
    {
        return m_jumpHeadSphereCastOffset;
    }
    void FirstPersonControllerComponent::SetJumpHeadSphereCastOffset(const float& new_jumpHeadSphereCastOffset)
    {
        m_jumpHeadSphereCastOffset = new_jumpHeadSphereCastOffset;
    }
    bool FirstPersonControllerComponent::GetHeadHitSetsApogee() const
    {
        return m_headHitSetsApogee;
    }
    void FirstPersonControllerComponent::SetHeadHitSetsApogee(const bool& new_headHitSetsApogee)
    {
        m_headHitSetsApogee = new_headHitSetsApogee;
    }
    float FirstPersonControllerComponent::GetFellFromHeight() const
    {
        return m_fellFromHeight;
    }
    void FirstPersonControllerComponent::SetFellFromHeight(const float& new_fellFromHeight)
    {
        m_fellFromHeight = new_fellFromHeight;
    }
    bool FirstPersonControllerComponent::GetHeadHit() const
    {
        return m_headHit;
    }
    void FirstPersonControllerComponent::SetHeadHit(const bool& new_headHit)
    {
        m_headHit = new_headHit;
    }
    bool FirstPersonControllerComponent::GetJumpHeadIgnoreDynamicRigidBodies() const
    {
        return m_jumpHeadIgnoreDynamicRigidBodies;
    }
    void FirstPersonControllerComponent::SetJumpHeadIgnoreDynamicRigidBodies(const bool& new_jumpHeadIgnoreDynamicRigidBodies)
    {
        m_jumpHeadIgnoreDynamicRigidBodies = new_jumpHeadIgnoreDynamicRigidBodies;
    }
    AZStd::string FirstPersonControllerComponent::GetHeadCollisionGroupName() const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, m_headCollisionGroup);
        return groupName;
    }
    void FirstPersonControllerComponent::SetHeadCollisionGroupByName(const AZStd::string& new_headCollisionGroupName)
    {
        bool success = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(success, &Physics::CollisionRequests::TryGetCollisionGroupByName, new_headCollisionGroupName, collisionGroup);
        if(success)
        {
            m_headCollisionGroup = collisionGroup;
            const AzPhysics::CollisionConfiguration& configuration = AZ::Interface<AzPhysics::SystemInterface>::Get()->GetConfiguration()->m_collisionConfig;
            m_headCollisionGroupId = configuration.m_collisionGroups.FindGroupIdByName(new_headCollisionGroupName);
        }
    }
    AZStd::vector<AZ::EntityId> FirstPersonControllerComponent::GetHeadHitEntityIds() const
    {
        return m_headHitEntityIds;
    }
    bool FirstPersonControllerComponent::GetJumpWhileCrouched() const
    {
        return m_jumpWhileCrouched;
    }
    void FirstPersonControllerComponent::SetJumpWhileCrouched(const bool& new_jumpWhileCrouched)
    {
        m_jumpWhileCrouched = new_jumpWhileCrouched;
    }
    float FirstPersonControllerComponent::GetCoyoteTime() const
    {
        return m_coyoteTime;
    }
    void FirstPersonControllerComponent::SetCoyoteTime(const float& new_coyoteTime)
    {
        m_coyoteTime = new_coyoteTime;
    }
    // GetCoyoteTimeNotZero() is not exposed to the request bus, it's used for the visibility attribute in the editor
    bool FirstPersonControllerComponent::GetCoyoteTimeGreaterThanZero() const
    {
        return m_coyoteTime > 0.f;
    }
    bool FirstPersonControllerComponent::GetUngroundedDueToJump() const
    {
        return m_ungroundedDueToJump;
    }
    void FirstPersonControllerComponent::SetUngroundedDueToJump(const bool& new_ungroundedDueToJump)
    {
        m_ungroundedDueToJump = new_ungroundedDueToJump;
    }
    bool FirstPersonControllerComponent::GetApplyGravityDuringCoyoteTime() const
    {
        return m_applyGravityDuringCoyoteTime;
    }
    void FirstPersonControllerComponent::SetApplyGravityDuringCoyoteTime(const bool& new_applyGravityDuringCoyoteTime)
    {
        m_applyGravityDuringCoyoteTime = new_applyGravityDuringCoyoteTime;
    }
    bool FirstPersonControllerComponent::GetCoyoteTimeTracksLastNormal() const
    {
        return m_coyoteTimeTracksLastNormal;
    }
    // GetCoyoteTimeGreaterThanZeroAndNoGravityDuring() is not exposed to the request bus, it's used for the visibility attribute in the editor
    bool FirstPersonControllerComponent::GetCoyoteTimeGreaterThanZeroAndNoGravityDuring() const
    {
        return !m_applyGravityDuringCoyoteTime && GetCoyoteTimeGreaterThanZero();
    }
    void FirstPersonControllerComponent::SetCoyoteTimeTracksLastNormal(const bool& new_coyoteTimeTracksLastNormal)
    {
        m_coyoteTimeTracksLastNormal = new_coyoteTimeTracksLastNormal;
    }
    bool FirstPersonControllerComponent::GetStandPrevented() const
    {
        return m_standPrevented;
    }
    void FirstPersonControllerComponent::SetStandPrevented(const bool& new_standPrevented)
    {
        m_standPrevented = new_standPrevented;
        if(m_standPrevented)
            m_standPreventedViaScript = true;
        else
            m_standPreventedViaScript = false;
    }
    bool FirstPersonControllerComponent::GetStandIgnoreDynamicRigidBodies() const
    {
        return m_standIgnoreDynamicRigidBodies;
    }
    void FirstPersonControllerComponent::SetStandIgnoreDynamicRigidBodies(const bool& new_standIgnoreDynamicRigidBodies)
    {
        m_standIgnoreDynamicRigidBodies = new_standIgnoreDynamicRigidBodies;
    }
    AZStd::string FirstPersonControllerComponent::GetStandCollisionGroupName() const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, m_standCollisionGroup);
        return groupName;
    }
    void FirstPersonControllerComponent::SetStandCollisionGroupByName(const AZStd::string& new_standCollisionGroupName)
    {
        bool success = false;
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(success, &Physics::CollisionRequests::TryGetCollisionGroupByName, new_standCollisionGroupName, collisionGroup);
        if(success)
        {
            m_standCollisionGroup = collisionGroup;
            const AzPhysics::CollisionConfiguration& configuration = AZ::Interface<AzPhysics::SystemInterface>::Get()->GetConfiguration()->m_collisionConfig;
            m_standCollisionGroupId = configuration.m_collisionGroups.FindGroupIdByName(new_standCollisionGroupName);
        }
    }
    AZStd::string FirstPersonControllerComponent::GetCollisionGroupName(AzPhysics::CollisionGroup& collisionGroup) const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, collisionGroup);
        return groupName;
    }
    AZStd::vector<AZ::EntityId> FirstPersonControllerComponent::GetStandPreventedEntityIds() const
    {
        return m_standPreventedEntityIds;
    }
    float FirstPersonControllerComponent::GetGroundSphereCastsRadiusPercentageIncrease() const
    {
        return m_groundSphereCastsRadiusPercentageIncrease;
    }
    void FirstPersonControllerComponent::SetGroundSphereCastsRadiusPercentageIncrease(const float& new_groundSphereCastsRadiusPercentageIncrease)
    {
        m_groundSphereCastsRadiusPercentageIncrease = new_groundSphereCastsRadiusPercentageIncrease;
    }
    float FirstPersonControllerComponent::GetGroundCloseCoyoteTimeRadiusPercentageIncrease() const
    {
        return m_groundCloseCoyoteTimeRadiusPercentageIncrease;
    }
    void FirstPersonControllerComponent::SetGroundCloseCoyoteTimeRadiusPercentageIncrease(const float& new_groundCloseCoyoteTimeRadiusPercentageIncrease)
    {
        m_groundCloseCoyoteTimeRadiusPercentageIncrease = new_groundCloseCoyoteTimeRadiusPercentageIncrease;
    }
    float FirstPersonControllerComponent::GetMaxGroundedAngleDegrees() const
    {
        return m_maxGroundedAngleDegrees;
    }
    void FirstPersonControllerComponent::SetMaxGroundedAngleDegrees(const float& new_maxGroundedAngleDegrees)
    {
        m_maxGroundedAngleDegrees = new_maxGroundedAngleDegrees;
    }
    float FirstPersonControllerComponent::GetTopWalkSpeed() const
    {
        return m_speed;
    }
    void FirstPersonControllerComponent::SetTopWalkSpeed(const float& new_speed)
    {
        m_speed = new_speed;
    }
    float FirstPersonControllerComponent::GetWalkAcceleration() const
    {
        return m_accel;
    }
    void FirstPersonControllerComponent::SetWalkAcceleration(const float& new_accel)
    {
        m_accel = new_accel;
    }
    float FirstPersonControllerComponent::GetTotalLerpTime() const
    {
        return m_totalLerpTime;
    }
    void FirstPersonControllerComponent::SetTotalLerpTime(const float& new_totalLerpTime)
    {
        m_totalLerpTime = new_totalLerpTime;
    }
    float FirstPersonControllerComponent::GetLerpTime() const
    {
        return m_lerpTime;
    }
    void FirstPersonControllerComponent::SetLerpTime(const float& new_lerpTime)
    {
        m_lerpTime = new_lerpTime;
    }
    float FirstPersonControllerComponent::GetDecelerationFactor() const
    {
        return m_decel;
    }
    void FirstPersonControllerComponent::SetDecelerationFactor(const float& new_decel)
    {
        m_decel = new_decel;
    }
    float FirstPersonControllerComponent::GetOpposingDecel() const
    {
        return m_opposingDecel;
    }
    void FirstPersonControllerComponent::SetOpposingDecel(const float& new_opposingDecel)
    {
        m_opposingDecel = new_opposingDecel;
    }
    bool FirstPersonControllerComponent::GetAccelerating() const
    {
        return m_accelerating;
    }
    bool FirstPersonControllerComponent::GetDecelerationFactorApplied() const
    {
        return m_decelerationFactorApplied;
    }
    bool FirstPersonControllerComponent::GetOpposingDecelFactorApplied() const
    {
        return m_opposingDecelFactorApplied;
    }
    bool FirstPersonControllerComponent::GetInstantVelocityRotation() const
    {
        return m_instantVelocityRotation;
    }
    void FirstPersonControllerComponent::SetInstantVelocityRotation(const bool& new_instantVelocityRotation)
    {
        m_instantVelocityRotation = new_instantVelocityRotation;
    }
    bool FirstPersonControllerComponent::GetVelocityXYIgnoresObstacles() const
    {
        return m_velocityXYIgnoresObstacles;
    }
    void FirstPersonControllerComponent::SetVelocityXYIgnoresObstacles(const bool& new_velocityXYIgnoresObstacles)
    {
        m_velocityXYIgnoresObstacles = new_velocityXYIgnoresObstacles;
    }
    bool FirstPersonControllerComponent::GetGravityIgnoresObstacles() const
    {
        return m_gravityIgnoresObstacles;
    }
    void FirstPersonControllerComponent::SetGravityIgnoresObstacles(const bool& new_gravityIgnoresObstacles)
    {
        m_gravityIgnoresObstacles = new_gravityIgnoresObstacles;
    }
    bool FirstPersonControllerComponent::GetPosZIgnoresObstacles() const
    {
        return m_posZIgnoresObstacles;
    }
    void FirstPersonControllerComponent::SetPosZIgnoresObstacles(const bool& new_posZIgnoresObstacles)
    {
        m_posZIgnoresObstacles = new_posZIgnoresObstacles;
    }
    bool FirstPersonControllerComponent::GetJumpAllowedWhenGravityPrevented() const
    {
        return m_jumpAllowedWhenGravityPrevented;
    }
    void FirstPersonControllerComponent::SetJumpAllowedWhenGravityPrevented(const bool& new_jumpAllowedWhenGravityPrevented)
    {
        m_jumpAllowedWhenGravityPrevented = new_jumpAllowedWhenGravityPrevented;
    }
    bool FirstPersonControllerComponent::GetVelocityXYObstructed() const
    {
        return m_velocityXYObstructed;
    }
    void FirstPersonControllerComponent::SetVelocityXYObstructed(const bool& new_velocityXYObstructed)
    {
        m_velocityXYObstructed = new_velocityXYObstructed;
    }
    bool FirstPersonControllerComponent::GetGravityPrevented() const
    {
        return m_gravityPrevented[1];
    }
    void FirstPersonControllerComponent::SetGravityPrevented(const bool& new_gravityPrevented)
    {
        m_gravityPrevented[0] = m_gravityPrevented[1] = new_gravityPrevented;
    }
    float FirstPersonControllerComponent::GetSprintScaleForward() const
    {
        return m_sprintScaleForward;
    }
    void FirstPersonControllerComponent::SetSprintScaleForward(const float& new_sprintScaleForward)
    {
        m_sprintScaleForward = new_sprintScaleForward;
    }
    float FirstPersonControllerComponent::GetSprintScaleBack() const
    {
        return m_sprintScaleBack;
    }
    void FirstPersonControllerComponent::SetSprintScaleBack(const float& new_sprintScaleBack)
    {
        m_sprintScaleBack = new_sprintScaleBack;
    }
    float FirstPersonControllerComponent::GetSprintScaleLeft() const
    {
        return m_sprintScaleLeft;
    }
    void FirstPersonControllerComponent::SetSprintScaleLeft(const float& new_sprintScaleLeft)
    {
        m_sprintScaleLeft = new_sprintScaleLeft;
    }
    float FirstPersonControllerComponent::GetSprintScaleRight() const
    {
        return m_sprintScaleRight;
    }
    void FirstPersonControllerComponent::SetSprintScaleRight(const float& new_sprintScaleRight)
    {
        m_sprintScaleRight = new_sprintScaleRight;
    }
    float FirstPersonControllerComponent::GetSprintAccelScale() const
    {
        return m_sprintAccelScale;
    }
    void FirstPersonControllerComponent::SetSprintAccelScale(const float& new_sprintAccelScale)
    {
        m_sprintAccelScale = new_sprintAccelScale;
    }
    float FirstPersonControllerComponent::GetSprintAccumulatedAccel() const
    {
        return m_sprintAccumulatedAccel;
    }
    void FirstPersonControllerComponent::SetSprintAccumulatedAccel(const float& new_sprintAccumulatedAccel)
    {
        m_sprintAccumulatedAccel = new_sprintAccumulatedAccel;
    }
    float FirstPersonControllerComponent::GetSprintMaxTime() const
    {
        return m_sprintMaxTime;
    }
    void FirstPersonControllerComponent::SetSprintMaxTime(const float& new_sprintMaxTime)
    {
        m_sprintMaxTime = new_sprintMaxTime;
        m_staminaPercentage = (m_sprintCooldownTimer == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;
    }
    float FirstPersonControllerComponent::GetSprintHeldTime() const
    {
        return m_sprintHeldDuration;
    }
    void FirstPersonControllerComponent::SetSprintHeldTime(const float& new_sprintHeldDuration)
    {
        const float prevSprintHeldDuration = m_sprintHeldDuration;
        if(new_sprintHeldDuration <= m_sprintMaxTime)
            m_sprintHeldDuration = new_sprintHeldDuration;
        else
            m_sprintHeldDuration = m_sprintMaxTime;
        m_staminaPercentage = (m_sprintCooldownTimer == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;
        if(m_sprintHeldDuration > prevSprintHeldDuration)
        {
            m_staminaDecreasing = true;
            m_staminaIncreasing = false;
        }
        else if(m_sprintHeldDuration < prevSprintHeldDuration)
        {
            m_staminaDecreasing = false;
            m_staminaIncreasing = true;
        }
    }
    float FirstPersonControllerComponent::GetSprintRegenRate() const
    {
        return m_sprintRegenRate;
    }
    void FirstPersonControllerComponent::SetSprintRegenRate(const float& new_sprintRegenRate)
    {
        m_sprintRegenRate = new_sprintRegenRate;
    }
    float FirstPersonControllerComponent::GetStaminaPercentage() const
    {
        return m_staminaPercentage;
    }
    void FirstPersonControllerComponent::SetStaminaPercentage(const float& new_staminaPercentage)
    {
        const float prevStaminaPercentage = m_staminaPercentage;
        if(new_staminaPercentage >= 0.f && new_staminaPercentage <= 100.f)
            m_staminaPercentage = new_staminaPercentage;
        else if(new_staminaPercentage < 0.f)
            m_staminaPercentage = 0.f;
        else
            m_staminaPercentage = 100.f;
        m_sprintHeldDuration = m_sprintMaxTime - m_sprintMaxTime * m_staminaPercentage / 100.f;
        if(m_staminaPercentage < prevStaminaPercentage)
        {
            m_staminaDecreasing = true;
            m_staminaIncreasing = false;
        }
        else if(m_staminaPercentage > prevStaminaPercentage)
        {
            m_staminaDecreasing = false;
            m_staminaIncreasing = true;
        }
    }
    bool FirstPersonControllerComponent::GetStaminaIncreasing() const
    {
        return m_staminaIncreasing;
    }
    bool FirstPersonControllerComponent::GetStaminaDecreasing() const
    {
        return m_staminaDecreasing;
    }
    bool FirstPersonControllerComponent::GetSprintUsesStamina() const
    {
        return m_sprintUsesStamina;
    }
    void FirstPersonControllerComponent::SetSprintUsesStamina(const bool& new_sprintUsesStamina)
    {
        m_sprintUsesStamina = new_sprintUsesStamina;
    }
    bool FirstPersonControllerComponent::GetRegenerateStaminaAutomatically() const
    {
        return m_regenerateStaminaAutomatically;
    }
    void FirstPersonControllerComponent::SetRegenerateStaminaAutomatically(const bool& new_regenerateStaminaAutomatically)
    {
        m_regenerateStaminaAutomatically = new_regenerateStaminaAutomatically;
    }
    bool FirstPersonControllerComponent::GetSprinting() const
    {
        if(m_sprintVelocityAdjust != 1.f && (m_standing || m_sprintWhileCrouched))
            return true;
        return false;
    }
    float FirstPersonControllerComponent::GetSprintTotalCooldownTime() const
    {
        return m_sprintTotalCooldownTime;
    }
    void FirstPersonControllerComponent::SetSprintTotalCooldownTime(const float& new_sprintTotalCooldownTime)
    {
        m_sprintTotalCooldownTime = new_sprintTotalCooldownTime;
    }
    float FirstPersonControllerComponent::GetSprintCooldownTimer() const
    {
        return m_sprintCooldownTimer;
    }
    void FirstPersonControllerComponent::SetSprintCooldownTimer(const float& new_sprintCooldownTimer)
    {
        m_sprintCooldownTimer = new_sprintCooldownTimer;
    }
    float FirstPersonControllerComponent::GetSprintPauseTime() const
    {
        return m_sprintPauseTime;
    }
    void FirstPersonControllerComponent::SetSprintPauseTime(const float& new_sprintPauseTime)
    {
        m_sprintPauseTime = new_sprintPauseTime;
    }
    float FirstPersonControllerComponent::GetSprintPause() const
    {
        return m_sprintPause;
    }
    void FirstPersonControllerComponent::SetSprintPause(const float& new_sprintPause)
    {
        m_sprintPause = new_sprintPause;
    }
    bool FirstPersonControllerComponent::GetSprintBackwards() const
    {
        return m_sprintBackwards;
    }
    void FirstPersonControllerComponent::SetSprintBackwards(const bool& new_sprintBackwards)
    {
        m_sprintBackwards = new_sprintBackwards;
    }
    bool FirstPersonControllerComponent::GetSprintWhileCrouched() const
    {
        return m_sprintWhileCrouched;
    }
    void FirstPersonControllerComponent::SetSprintWhileCrouched(const bool& new_sprintWhileCrouched)
    {
        m_sprintWhileCrouched = new_sprintWhileCrouched;
    }
    bool FirstPersonControllerComponent::GetSprintViaScript() const
    {
        return m_sprintViaScript;
    }
    void FirstPersonControllerComponent::SetSprintViaScript(const bool& new_sprintViaScript)
    {
        m_sprintViaScript = new_sprintViaScript;
    }
    bool FirstPersonControllerComponent::GetSprintEnableDisable() const
    {
        return m_sprintEnableDisable;
    }
    void FirstPersonControllerComponent::SetSprintEnableDisable(const bool& new_sprintEnableDisable)
    {
        m_sprintEnableDisable = new_sprintEnableDisable;
    }
    bool FirstPersonControllerComponent::GetCrouching() const
    {
        return m_crouching;
    }
    void FirstPersonControllerComponent::SetCrouching(const bool& new_crouching)
    {
        m_crouching = new_crouching;
    }
    bool FirstPersonControllerComponent::GetCrouched() const
    {
        return m_crouched;
    }
    bool FirstPersonControllerComponent::GetStanding() const
    {
        return m_standing;
    }
    float FirstPersonControllerComponent::GetCrouchedPercentage() const
    {
        return abs(m_cameraLocalZTravelDistance) / m_crouchDistance * 100.f;
    }
    bool FirstPersonControllerComponent::GetCrouchScriptLocked() const
    {
        return m_crouchScriptLocked;
    }
    void FirstPersonControllerComponent::SetCrouchScriptLocked(const bool& new_crouchScriptLocked)
    {
        m_crouchScriptLocked = new_crouchScriptLocked;
    }
    float FirstPersonControllerComponent::GetCrouchScale() const
    {
        return m_crouchScale;
    }
    void FirstPersonControllerComponent::SetCrouchScale(const float& new_crouchScale)
    {
        m_crouchScale = new_crouchScale;
    }
    float FirstPersonControllerComponent::GetCrouchDistance() const
    {
        return m_crouchDistance;
    }
    void FirstPersonControllerComponent::SetCrouchDistance(const float& new_crouchDistance)
    {
        m_crouchDistance = new_crouchDistance;

        // Calculate the crouch down final velocity
        m_crouchDownFinalVelocity = 2 * m_crouchDistance / m_crouchTime - m_crouchDownInitVelocity;
        // Check to make sure that the crouching is decelerated, if not then force a constant velocity to meet m_crouchTime
        if(m_crouchDownFinalVelocity > m_crouchDownInitVelocity)
        {
            m_crouchDownInitVelocity = m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = m_crouchDownInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed set to a value that's too slow to reach the crouched position within the crouch time, setting the crouch speed to the crouch distance divide by the crouch time instead (%.3f m/s).", m_crouchDownInitVelocity);
        }
        // Check to make sure that the final crouching velocity isn't negative, and fix it if it is
        else if(m_crouchDownFinalVelocity < 0)
        {
            m_crouchDownInitVelocity = 2.f * m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed set to a value that's too fast to reach the crouching position at crouch time, setting the start crouch speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchDownInitVelocity);
        }

        // Calculate the crouch up (stand) final velocity
        m_crouchUpFinalVelocity = (2 * m_crouchDistance) / m_standTime - m_crouchUpInitVelocity;
        // Check to make sure that the standing is decelerated, if not then force a constant velocity to meet m_standTime
        if(m_crouchUpFinalVelocity > m_crouchUpInitVelocity)
        {
            m_crouchUpInitVelocity = m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = m_crouchUpInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Stand start speed set to a value that's too slow to reach the standing position within the stand time, setting the stand speed to the crouch distance divide by the stand time instead (%.3f m/s).", m_crouchUpInitVelocity);
        }
        // Check to make sure that the final standing velocity isn't negative, and fix it if it is
        else if(m_crouchUpFinalVelocity < 0)
        {
            m_crouchUpInitVelocity = 2.f * m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Stand start speed set to a value that's too fast to reach the standing position at stand time, setting the start stand speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchUpInitVelocity);
        }
    }
    float FirstPersonControllerComponent::GetCrouchTime() const
    {
        return m_crouchTime;
    }
    void FirstPersonControllerComponent::SetCrouchTime(const float& new_crouchTime)
    {
        m_crouchTime = new_crouchTime;
    }
    float FirstPersonControllerComponent::GetCrouchStartSpeed() const
    {
        return m_crouchDownInitVelocity;
    }
    void FirstPersonControllerComponent::SetCrouchStartSpeed(const float& new_crouchDownInitVelocity)
    {
        m_crouchDownInitVelocity = new_crouchDownInitVelocity;
        // Recalculate the crouch down final velocity
        m_crouchDownFinalVelocity = 2 * m_crouchDistance / m_crouchTime - m_crouchDownInitVelocity;
        // Check to make sure that the crouching is decelerated, if not then force a constant velocity to meet m_crouchTime
        if(m_crouchDownFinalVelocity > m_crouchDownInitVelocity)
        {
            m_crouchDownInitVelocity = m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = m_crouchDownInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed changed to a value that's too slow to reach the crouched position within the crouch time, setting the crouch speed to the crouch distance divide by the crouch time instead.");
        }
        // Check to make sure that the final crouching velocity isn't negative, and fix it if it is
        else if(m_crouchDownFinalVelocity < 0)
        {
            m_crouchDownInitVelocity = 2.f * m_crouchDistance / m_crouchTime;
            m_crouchDownFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Crouch start speed changed to a value that's too fast to reach the crouching position at crouch time, setting the start crouch speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchDownInitVelocity);
        }
    }
    float FirstPersonControllerComponent::GetCrouchEndSpeed() const
    {
        return m_crouchDownFinalVelocity;
    }
    float FirstPersonControllerComponent::GetStandTime() const
    {
        return m_standTime;
    }
    void FirstPersonControllerComponent::SetStandTime(const float& new_standTime)
    {
        m_standTime = new_standTime;
    }
    float FirstPersonControllerComponent::GetStandStartSpeed() const
    {
        return m_crouchUpInitVelocity;
    }
    void FirstPersonControllerComponent::SetStandStartSpeed(const float& new_crouchUpInitVelocity)
    {
        m_crouchUpInitVelocity = new_crouchUpInitVelocity;
        // Recalculate the crouch up (stand) final velocity
        m_crouchUpFinalVelocity = (2 * m_crouchDistance) / m_standTime - m_crouchUpInitVelocity;
        // Check to make sure that the standing is decelerated, if not then force a constant velocity to meet m_standTime
        if(m_crouchUpFinalVelocity > m_crouchUpInitVelocity)
        {
            m_crouchUpInitVelocity = m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = m_crouchUpInitVelocity;
            AZ_Warning("First Person Controller Component", false, "Stand start speed changed to a value that's too slow to reach the standing position within the stand time, setting the stand speed to the crouch distance divide by the stand time instead.");
        }
        // Check to make sure that the final standing velocity isn't negative, and fix it if it is
        else if(m_crouchUpFinalVelocity < 0)
        {
            m_crouchUpInitVelocity = 2.f * m_crouchDistance / m_standTime;
            m_crouchUpFinalVelocity = 0.f;
            AZ_Warning("First Person Controller Component", false, "Stand start speed changed to a value that's too fast to reach the standing position at stand time, setting the start stand speed to something slower (%.3f m/s) that ends at a speed of zero.", m_crouchUpInitVelocity);
        }
    }
    float FirstPersonControllerComponent::GetStandEndSpeed() const
    {
        return m_crouchUpFinalVelocity;
    }
    float FirstPersonControllerComponent::GetUncrouchHeadSphereCastOffset() const
    {
        return m_uncrouchHeadSphereCastOffset;
    }
    bool FirstPersonControllerComponent::GetCrouchingDownMove() const
    {
        return m_crouchingDownMove;
    }
    bool FirstPersonControllerComponent::GetStandingUpMove() const
    {
        return m_standingUpMove;
    }
    void FirstPersonControllerComponent::SetUncrouchHeadSphereCastOffset(const float& new_uncrouchHeadSphereCastOffset)
    {
        m_uncrouchHeadSphereCastOffset = new_uncrouchHeadSphereCastOffset;
    }
    bool FirstPersonControllerComponent::GetCrouchEnableToggle() const
    {
        return m_crouchEnableToggle;
    }
    void FirstPersonControllerComponent::SetCrouchEnableToggle(const bool& new_crouchEnableToggle)
    {
        m_crouchEnableToggle = new_crouchEnableToggle;
    }
    bool FirstPersonControllerComponent::GetCrouchJumpCausesStanding() const
    {
        return m_crouchJumpCausesStanding;
    }
    void FirstPersonControllerComponent::SetCrouchJumpCausesStanding(const bool& new_crouchJumpCausesStanding)
    {
        m_crouchJumpCausesStanding = new_crouchJumpCausesStanding;
    }
    bool FirstPersonControllerComponent::GetCrouchPendJumps() const
    {
        return m_crouchPendJumps;
    }
    void FirstPersonControllerComponent::SetCrouchPendJumps(const bool& new_crouchPendJumps)
    {
        m_crouchPendJumps = new_crouchPendJumps;
    }
    bool FirstPersonControllerComponent::GetCrouchSprintCausesStanding() const
    {
        return m_crouchSprintCausesStanding;
    }
    void FirstPersonControllerComponent::SetCrouchSprintCausesStanding(const bool& new_crouchSprintCausesStanding)
    {
        m_crouchSprintCausesStanding = new_crouchSprintCausesStanding;
    }
    bool FirstPersonControllerComponent::GetCrouchPriorityWhenSprintPressed() const
    {
        return m_crouchPriorityWhenSprintPressed;
    }
    void FirstPersonControllerComponent::SetCrouchPriorityWhenSprintPressed(const bool& new_crouchPriorityWhenSprintPressed)
    {
        m_crouchPriorityWhenSprintPressed = new_crouchPriorityWhenSprintPressed;
    }
    bool FirstPersonControllerComponent::GetCrouchWhenNotGrounded() const
    {
        return m_crouchWhenNotGrounded;
    }
    void FirstPersonControllerComponent::SetCrouchWhenNotGrounded(const bool& new_crouchWhenNotGrounded)
    {
        m_crouchWhenNotGrounded = new_crouchWhenNotGrounded;
    }
    bool FirstPersonControllerComponent::GetEnableCameraCharacterRotation() const
    {
        return m_enableCameraCharacterRotation;
    }
    void FirstPersonControllerComponent::SetEnableCameraCharacterRotation(const bool& new_enableCameraCharacterRotation)
    {
        m_enableCameraCharacterRotation = new_enableCameraCharacterRotation;
    }
    float FirstPersonControllerComponent::GetCharacterAndCameraYawSensitivity() const
    {
        return m_yawSensitivity;
    }
    void FirstPersonControllerComponent::SetCharacterAndCameraYawSensitivity(const float& new_yawSensitivity)
    {
        m_yawSensitivity = new_yawSensitivity;
    }
    float FirstPersonControllerComponent::GetCameraPitchSensitivity() const
    {
        return m_pitchSensitivity;
    }
    void FirstPersonControllerComponent::SetCameraPitchSensitivity(const float& new_pitchSensitivity)
    {
        m_pitchSensitivity = new_pitchSensitivity;
    }
    float FirstPersonControllerComponent::GetCameraPitchMaxAngleRadians() const
    {
        return m_cameraPitchMaxAngle;
    }
    void FirstPersonControllerComponent::SetCameraPitchMaxAngleRadians(const float& new_cameraPitchMaxAngleRadians)
    {
        m_cameraPitchMaxAngle = new_cameraPitchMaxAngleRadians;
    }
    float FirstPersonControllerComponent::GetCameraPitchMaxAngleDegrees() const
    {
        return (m_cameraPitchMaxAngle * 360.f/AZ::Constants::TwoPi);
    }
    void FirstPersonControllerComponent::SetCameraPitchMaxAngleDegrees(const float& new_cameraPitchMaxAngleDegrees)
    {
        m_cameraPitchMaxAngle = (new_cameraPitchMaxAngleDegrees * AZ::Constants::TwoPi/360.f);
    }
    float FirstPersonControllerComponent::GetCameraPitchMinAngleRadians() const
    {
        return m_cameraPitchMinAngle;
    }
    void FirstPersonControllerComponent::SetCameraPitchMinAngleRadians(const float& new_cameraPitchMinAngleRadians)
    {
        m_cameraPitchMinAngle = new_cameraPitchMinAngleRadians;
    }
    float FirstPersonControllerComponent::GetCameraPitchMinAngleDegrees() const
    {
        return (m_cameraPitchMinAngle * 360.f/AZ::Constants::TwoPi);
    }
    void FirstPersonControllerComponent::SetCameraPitchMinAngleDegrees(const float& new_cameraPitchMinAngleDegrees)
    {
        m_cameraPitchMinAngle = (new_cameraPitchMinAngleDegrees * AZ::Constants::TwoPi/360.f);
    }
    float FirstPersonControllerComponent::GetCameraRotationDampFactor() const
    {
        return m_rotationDamp;
    }
    void FirstPersonControllerComponent::SetCameraRotationDampFactor(const float& new_rotationDamp)
    {
        m_rotationDamp = new_rotationDamp;
    }
    AZ::TransformInterface* FirstPersonControllerComponent::GetCharacterTransformInterfacePtr() const
    {
        return GetEntity()->GetTransform();
    }
    AZ::Transform FirstPersonControllerComponent::GetCharacterTransform() const
    {
        return GetEntity()->GetTransform()->GetWorldTM();
    }
    void FirstPersonControllerComponent::SetCharacterTransform(const AZ::Transform& new_characterTransform)
    {
        GetEntity()->GetTransform()->SetWorldTM(new_characterTransform);
    }
    AZ::Vector3 FirstPersonControllerComponent::GetCharacterWorldTranslation() const
    {
        return GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
    }
    void FirstPersonControllerComponent::SetCharacterWorldTranslation(const AZ::Vector3& new_characterWorldTranslation)
    {
        GetEntity()->GetTransform()->SetWorldTranslation(new_characterWorldTranslation);
    }
    void FirstPersonControllerComponent::UpdateCharacterAndCameraYaw(const float& new_characterAndCameraYawAngle, const bool& updateCharacterAndCameraYawConsidersInput)
    {
        if(updateCharacterAndCameraYawConsidersInput)
            m_cameraRotationAngles[2] = new_characterAndCameraYawAngle - m_yawValue * m_yawSensitivity;
        else
            m_cameraRotationAngles[2] = new_characterAndCameraYawAngle;
        m_rotatingYawViaScriptGamepad = true;
    }
    void FirstPersonControllerComponent::UpdateCameraPitch(const float& new_cameraPitchAngle, const bool& updateCameraPitchConsidersInput)
    {
        if(updateCameraPitchConsidersInput)
            m_cameraRotationAngles[0] = new_cameraPitchAngle - m_pitchValue * m_pitchSensitivity;
        else
            m_cameraRotationAngles[0] = new_cameraPitchAngle;
        m_rotatingPitchViaScriptGamepad = true;
    }
    float FirstPersonControllerComponent::GetHeading() const
    {
        return m_currentHeading;
    }
    void FirstPersonControllerComponent::SetHeadingForTick(const float& new_currentHeading)
    {
        m_currentHeading = new_currentHeading;
        m_scriptSetCurrentHeadingTick = true;
    }
    float FirstPersonControllerComponent::GetPitch() const
    {
        return m_currentPitch;
    }
}
