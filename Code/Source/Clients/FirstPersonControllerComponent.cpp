#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/InputDeviceId.h>

#include <PhysX/CharacterControllerBus.h>

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

              // Camera Rotation group
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yawSensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitchSensitivity)
              ->Field("Camera Rotation Damp Factor", &FirstPersonControllerComponent::m_rotationDamp)
              ->Field("Camera Slerp Instead of Lerp Rotation", &FirstPersonControllerComponent::m_cameraSlerpInsteadOfLerpRotation)

              // Direction Scale Factors group
              ->Field("Forward Scale", &FirstPersonControllerComponent::m_forwardScale)
              ->Field("Back Scale", &FirstPersonControllerComponent::m_backScale)
              ->Field("Left Scale", &FirstPersonControllerComponent::m_leftScale)
              ->Field("Right Scale", &FirstPersonControllerComponent::m_rightScale)

              // X&Y Movement group
              ->Field("Top Walking Speed (m/s)", &FirstPersonControllerComponent::m_speed)
              ->Field("Walking Acceleration (m/s²)", &FirstPersonControllerComponent::m_accel)
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Opposing Direction Deceleration Factor", &FirstPersonControllerComponent::m_opposingDecel)
              ->Field("X&Y Movement Tracks Surface Inclines", &FirstPersonControllerComponent::m_velocityXCrossYTracksNormal)
              ->Field("Instant Velocity Rotation", &FirstPersonControllerComponent::m_instantVelocityRotation)

              // Sprinting group
              ->Field("Sprint Forward Scale", &FirstPersonControllerComponent::m_sprintScaleForward)
              ->Field("Sprint Back Scale", &FirstPersonControllerComponent::m_sprintScaleBack)
              ->Field("Sprint Left Scale", &FirstPersonControllerComponent::m_sprintScaleLeft)
              ->Field("Sprint Right Scale", &FirstPersonControllerComponent::m_sprintScaleRight)
              ->Field("Sprint Acceleration Scale", &FirstPersonControllerComponent::m_sprintAccelScale)
              ->Field("Sprint Max Time (sec)", &FirstPersonControllerComponent::m_sprintMaxTime)
              ->Field("Sprint Cooldown Time (sec)", &FirstPersonControllerComponent::m_sprintCooldownTime)
              ->Field("Sprint Backwards", &FirstPersonControllerComponent::m_sprintBackwards)
              ->Field("Sprint While Crouched", &FirstPersonControllerComponent::m_sprintWhileCrouched)

              // Crouching group
              ->Field("Crouch Scale", &FirstPersonControllerComponent::m_crouchScale)
              ->Field("Crouch Distance", &FirstPersonControllerComponent::m_crouchDistance)
              ->Field("Crouch Time", &FirstPersonControllerComponent::m_crouchTime)
              ->Field("Crouch Standing Head Clearance", &FirstPersonControllerComponent::m_uncrouchHeadSphereCastOffset)
              ->Field("Crouch Enable Toggle", &FirstPersonControllerComponent::m_crouchEnableToggle)
              ->Field("Crouch Jump Causes Standing", &FirstPersonControllerComponent::m_crouchJumpCausesStanding)
              ->Field("Crouch Sprint Causes Standing", &FirstPersonControllerComponent::m_crouchSprintCausesStanding)
              ->Field("Crouch Priority When Sprint Pressed", &FirstPersonControllerComponent::m_crouchPriorityWhenSprintPressed)

              // Jumping group
              ->Field("Grounded Collision Group", &FirstPersonControllerComponent::m_groundedCollisionGroupId)
              ->Field("Jump Head Hit Collision Group", &FirstPersonControllerComponent::m_headCollisionGroupId)
              ->Field("Gravity (m/s²)", &FirstPersonControllerComponent::m_gravity)
              ->Field("Jump Initial Velocity (m/s)", &FirstPersonControllerComponent::m_jumpInitialVelocity)
              ->Field("Jump Held Gravity Factor", &FirstPersonControllerComponent::m_jumpHeldGravityFactor)
              ->Field("Jump Falling Gravity Factor", &FirstPersonControllerComponent::m_jumpFallingGravityFactor)
              ->Field("X&Y Acceleration Jump Factor (m/s²)", &FirstPersonControllerComponent::m_jumpAccelFactor)
              ->Field("Grounded Sphere Cast Radius Percentage Increase (%)", &FirstPersonControllerComponent::m_groundedSphereCastRadiusPercentageIncrease)
              ->Field("Grounded Offset (m)", &FirstPersonControllerComponent::m_groundedSphereCastOffset)
              ->Field("Ground Close Offset (m)", &FirstPersonControllerComponent::m_groundCloseSphereCastOffset)
              ->Field("Jump Hold Distance (m)", &FirstPersonControllerComponent::m_jumpHoldDistance)
              ->Field("Jump Head Hit Detection Distance (m)", &FirstPersonControllerComponent::m_jumpHeadSphereCastOffset)
              ->Field("Jump Head Hit Ignore Non-Kinematic Rigid Bodies", &FirstPersonControllerComponent::m_jumpHeadIgnoreNonKinematicRigidBodies)
              ->Field("Enable Double Jump", &FirstPersonControllerComponent::m_doubleJumpEnabled)
              ->Field("Update X&Y Velocity When Ascending", &FirstPersonControllerComponent::m_updateXYAscending)
              ->Field("Update X&Y Velocity When Decending", &FirstPersonControllerComponent::m_updateXYDecending)
              ->Field("Update X&Y Velocity Only When Ground Close", &FirstPersonControllerComponent::m_updateXYOnlyNearGround)

              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonControllerComponent>("First Person Controller",
                    "First person character controller")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")

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
                        "Camera Yaw Rotate Input", "Camera left/right rotation control. Must match an Event Name in the .inputbindings file.")
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

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Camera Rotation")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yawSensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitchSensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rotationDamp,
                        "Camera Rotation Damp Factor", "The ‘smoothness’ of the camera rotation. Applies a damp factor to the camera rotation. Setting this to anything greater than the framerate will essentially disable this effect.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_cameraSlerpInsteadOfLerpRotation,
                        "Camera Slerp Instead of Lerp Rotation", "Determines whether Camera Rotation Damp Factor uses Slerp or Lerp function. Enable for Slerp, and disable for Lerp.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "X&Y Movement")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_speed,
                        "Top Walking Speed (m/s)", "Determines maximum walking speed of the character.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_accel,
                        "Walking Acceleration (m/s²)", "Determines how quickly the character will reach maximum walking speed.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_decel,
                        "Deceleration Factor", "Determines how quickly the character will stop. The product of this number and Walking Acceleration determines the resulting deceleration.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_opposingDecel,
                        "Opposing Direction Deceleration Factor", "Determines the deceleration when opposing the current direction of motion. The product of this number and Walking Acceleration creates the deceleration that's used.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_velocityXCrossYTracksNormal,
                        "X&Y Movement Tracks Surface Inclines", "Determines whether the character's X&Y movement will be tilted in order to follow inclines. This will apply up to the max angle that is specified in the PhysX Character Controller component.")
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
                        "Sprint Forward Scale", "Determines the sprint factor applied in the forward direction.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleBack,
                        "Sprint Back Scale", "Determines the sprint factor applied in the back direction.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleLeft,
                        "Sprint Left Scale", "Determines the sprint factor applied in the left direction.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScaleRight,
                        "Sprint Right Scale", "Determines the sprint factor applied in the right direction.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintAccelScale,
                        "Sprint Acceleration Scale", "Determines how quickly the character will reach maximum sprinting speed.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintMaxTime,
                        "Sprint Max Time (sec)", "The maximum consecutive sprinting time before beginning Sprint Cooldown. The underlying quantity of Stamina is set by this number.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintCooldownTime,
                        "Sprint Cooldown Time (sec)", "The time required to wait before sprinting or using Stamina once Sprint Max Time has been reached or Stamina hits 0%.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintBackwards,
                        "Sprint Backwards", "Determines whether the character can sprint backwards.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintWhileCrouched,
                        "Sprint While Crouched", "Determines whether the character can sprint while crouched.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouching")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchScale,
                        "Crouch Scale", "Determines how much slow the character will move when crouched. The product of this number and the top walk speed is the top crouch walk speed.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchDistance,
                        "Crouch Distance (m)", "Determines the distance the camera will move on the Z axis and the reduction in the PhysX Character Controller's capsule collider height. This number cannot be greater than the capsule's height minus two times its radius.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchTime,
                        "Crouch Time (sec)", "Determines the time it takes to complete the crouch.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_uncrouchHeadSphereCastOffset,
                        "Crouch Standing Head Clearance (m)", "Determines the distance above the player's head to detect whether there is an obstruction and prevent them from fully standing up if there is.")
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
                        &FirstPersonControllerComponent::m_gravity,
                        "Gravity (m/s²)", "Z Acceleration due to gravity, set this to zero if using the PhysX Character Gameplay component's gravity instead.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpInitialVelocity,
                        "Jump Initial Velocity (m/s)", "The velocity used when initiating the jump.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeldGravityFactor,
                        "Jump Held Gravity Factor", "The factor applied to the character's gravity for the beginning of the jump.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpFallingGravityFactor,
                        "Jump Falling Gravity Factor", "The factor applied to the character's gravity when the character is falling.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpAccelFactor,
                        "X&Y Acceleration Jump Factor (m/s²)", "X&Y acceleration factor while in the air. This depends on whether Update X&Y Velocity When Ascending is enabled, Update X&Y Velocity When Descending is enabled, and Update X&Y Velocity Only When Ground Close is enabled.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHoldDistance,
                        "Jump Hold Offset (m)", "Effectively determines the time that jump may be held.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundedSphereCastOffset,
                        "Grounded Offset (m)", "Determines the offset distance between the bottom of the character and ground.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundCloseSphereCastOffset,
                        "Ground Close Offset (m)", "Determines the offset distance between the bottom of the character and ground.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_groundedSphereCastRadiusPercentageIncrease,
                        "Grounded Sphere Cast Radius Percentage Increase (%)", "The percentage increase in the ground detection sphere cast over the PhysX Character Controller's capsule radius.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeadSphereCastOffset,
                        "Jump Head Hit Detection Distance (m)", "The distance above the character's head where an obstruction will be detected for jumping. The apogee of the jump occurs when there is a collision.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeadIgnoreNonKinematicRigidBodies,
                        "Jump Head Hit Ignore Non-Kinematic Rigid Bodies", "Determines whether or not non-kinematic (dynamic) rigid bodies are ignored by the jump head collision detection system.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_doubleJumpEnabled,
                        "Enable Double Jump", "Turn this on to enable double jumping.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYAscending,
                        "Update X&Y Velocity When Ascending", "Allows movement in X&Y during a jump’s ascent.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYDecending,
                        "Update X&Y Velocity When Descending", "Allows movement in X&Y during a jump’s descent.")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYOnlyNearGround,
                        "Update X&Y Velocity Only When Ground Close", "Allows movement in X&Y only if close to an acceptable ground entity. According to the distance set in Jump Hold Offset. If the ascending and descending options are disabled, then this will effectively do nothing.");
            }
        }

        if(auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonControllerNotificationBus>("FirstPersonNotificationBus")
                ->Handler<FirstPersonControllerNotificationHandler>();

            bc->EBus<FirstPersonControllerComponentRequestBus>("FirstPersonControllerComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Controller")
                ->Event("Get Active Camera Id", &FirstPersonControllerComponentRequests::GetActiveCameraId)
                ->Event("Reacquire Child Entity Ids", &FirstPersonControllerComponentRequests::ReacquireChildEntityIds)
                ->Event("Get Forward Event Name", &FirstPersonControllerComponentRequests::GetForwardEventName)
                ->Event("Set Forward Event Name", &FirstPersonControllerComponentRequests::SetForwardEventName)
                ->Event("Get Forward Scale", &FirstPersonControllerComponentRequests::GetForwardScale)
                ->Event("Set Forward Scale", &FirstPersonControllerComponentRequests::SetForwardScale)
                ->Event("Get Forward Input Value", &FirstPersonControllerComponentRequests::GetForwardInputValue)
                ->Event("Get Back Event Name", &FirstPersonControllerComponentRequests::GetBackEventName)
                ->Event("Set Back Event Name", &FirstPersonControllerComponentRequests::SetBackEventName)
                ->Event("Get Back Scale", &FirstPersonControllerComponentRequests::GetBackScale)
                ->Event("Set Back Scale", &FirstPersonControllerComponentRequests::SetBackScale)
                ->Event("Get Back Input Value", &FirstPersonControllerComponentRequests::GetBackInputValue)
                ->Event("Get Left Event Name", &FirstPersonControllerComponentRequests::GetLeftEventName)
                ->Event("Set Left Event Name", &FirstPersonControllerComponentRequests::SetLeftEventName)
                ->Event("Get Left Scale", &FirstPersonControllerComponentRequests::GetLeftScale)
                ->Event("Set Left Scale", &FirstPersonControllerComponentRequests::SetLeftScale)
                ->Event("Get Left Input Value", &FirstPersonControllerComponentRequests::GetLeftInputValue)
                ->Event("Get Right Event Name", &FirstPersonControllerComponentRequests::GetRightEventName)
                ->Event("Set Right Event Name", &FirstPersonControllerComponentRequests::SetRightEventName)
                ->Event("Get Right Scale", &FirstPersonControllerComponentRequests::GetRightScale)
                ->Event("Set Right Scale", &FirstPersonControllerComponentRequests::SetRightScale)
                ->Event("Get Right Input Value", &FirstPersonControllerComponentRequests::GetRightInputValue)
                ->Event("Get Yaw Event Name", &FirstPersonControllerComponentRequests::GetYawEventName)
                ->Event("Set Yaw Event Name", &FirstPersonControllerComponentRequests::SetYawEventName)
                ->Event("Get Yaw Input Value", &FirstPersonControllerComponentRequests::GetYawInputValue)
                ->Event("Get Pitch Event Name", &FirstPersonControllerComponentRequests::GetPitchEventName)
                ->Event("Set Pitch Event Name", &FirstPersonControllerComponentRequests::SetPitchEventName)
                ->Event("Get Pitch Input Value", &FirstPersonControllerComponentRequests::GetPitchInputValue)
                ->Event("Get Sprint Event Name", &FirstPersonControllerComponentRequests::GetSprintEventName)
                ->Event("Set Sprint Event Name", &FirstPersonControllerComponentRequests::SetSprintEventName)
                ->Event("Get Sprint Input Value", &FirstPersonControllerComponentRequests::GetSprintInputValue)
                ->Event("Get Crouch Event Name", &FirstPersonControllerComponentRequests::GetCrouchEventName)
                ->Event("Set Crouch Event Name", &FirstPersonControllerComponentRequests::SetCrouchEventName)
                ->Event("Get Crouch Input Value", &FirstPersonControllerComponentRequests::GetCrouchInputValue)
                ->Event("Get Jump Event Name", &FirstPersonControllerComponentRequests::GetJumpEventName)
                ->Event("Set Jump Event Name", &FirstPersonControllerComponentRequests::SetJumpEventName)
                ->Event("Get Jump Input Value", &FirstPersonControllerComponentRequests::GetJumpInputValue)
                ->Event("Get Grounded", &FirstPersonControllerComponentRequests::GetGrounded)
                ->Event("Set Grounded For Tick", &FirstPersonControllerComponentRequests::SetGroundedForTick)
                ->Event("Get Ground Scene Query Hits", &FirstPersonControllerComponentRequests::GetGroundSceneQueryHits)
                ->Event("Get Ground Close Scene Query Hits", &FirstPersonControllerComponentRequests::GetGroundCloseSceneQueryHits)
                ->Event("Get Ground Sum Normals Direction", &FirstPersonControllerComponentRequests::GetGroundSumNormalsDirection)
                ->Event("Get Ground Close Sum Normals Direction", &FirstPersonControllerComponentRequests::GetGroundCloseSumNormalsDirection)
                ->Event("Get Scene Query Hit Result Flags", &FirstPersonControllerComponentRequests::GetSceneQueryHitResultFlags)
                ->Event("Get Scene Query Hit EntityId", &FirstPersonControllerComponentRequests::GetSceneQueryHitEntityId)
                ->Event("Get Scene Query Hit Normal", &FirstPersonControllerComponentRequests::GetSceneQueryHitNormal)
                ->Event("Get Scene Query Hit Position", &FirstPersonControllerComponentRequests::GetSceneQueryHitPosition)
                ->Event("Get Scene Query Hit Distance", &FirstPersonControllerComponentRequests::GetSceneQueryHitDistance)
                ->Event("Get Scene Query Hit MaterialId", &FirstPersonControllerComponentRequests::GetSceneQueryHitMaterialId)
                ->Event("Get Scene Query Hit Shape Pointer", &FirstPersonControllerComponentRequests::GetSceneQueryHitShapePtr)
                ->Event("Get Scene Query Hit Simulated Body Handle", &FirstPersonControllerComponentRequests::GetSceneQueryHitSimulatedBodyHandle)
                ->Event("Get Ground Close", &FirstPersonControllerComponentRequests::GetGroundClose)
                ->Event("Set Ground Close For Tick", &FirstPersonControllerComponentRequests::SetGroundCloseForTick)
                ->Event("Get Grounded Collision Group Name", &FirstPersonControllerComponentRequests::GetGroundedCollisionGroupName)
                ->Event("Set Grounded Collision Group Name", &FirstPersonControllerComponentRequests::SetGroundedCollisionGroup)
                ->Event("Get Air Time", &FirstPersonControllerComponentRequests::GetAirTime)
                ->Event("Get Gravity", &FirstPersonControllerComponentRequests::GetGravity)
                ->Event("Set Gravity", &FirstPersonControllerComponentRequests::SetGravity)
                ->Event("Get Velocity X×Y Direction", &FirstPersonControllerComponentRequests::GetVelocityXCrossYDirection)
                ->Event("Set Velocity X×Y Direction", &FirstPersonControllerComponentRequests::SetVelocityXCrossYDirection)
                ->Event("Get Velocity X×Y Tracks Normal", &FirstPersonControllerComponentRequests::GetVelocityXCrossYTracksNormal)
                ->Event("Set Velocity X×Y Tracks Normal", &FirstPersonControllerComponentRequests::SetVelocityXCrossYTracksNormal)
                ->Event("Get Velocity Z Positive Direction", &FirstPersonControllerComponentRequests::GetVelocityZPosDirection)
                ->Event("Set Velocity Z Positive Direction", &FirstPersonControllerComponentRequests::SetVelocityZPosDirection)
                ->Event("Get Sphere Casts' Axis Direction", &FirstPersonControllerComponentRequests::GetSphereCastsAxisDirectionPose)
                ->Event("Set Sphere Casts' Axis Direction", &FirstPersonControllerComponentRequests::SetSphereCastsAxisDirectionPose)
                ->Event("Get Vector Angles Between Vectors (Radians)", &FirstPersonControllerComponentRequests::GetVectorAnglesBetweenVectorsRadians)
                ->Event("Get Vector Angles Between Vectors (Degrees)", &FirstPersonControllerComponentRequests::GetVectorAnglesBetweenVectorsDegrees)
                ->Event("Create Ellipse Scaled Vector2", &FirstPersonControllerComponentRequests::CreateEllipseScaledVector)
                ->Event("Get Jump Held Gravity Factor", &FirstPersonControllerComponentRequests::GetJumpHeldGravityFactor)
                ->Event("Set Jump Held Gravity Factor", &FirstPersonControllerComponentRequests::SetJumpHeldGravityFactor)
                ->Event("Get Jump Falling Gravity Factor", &FirstPersonControllerComponentRequests::GetJumpFallingGravityFactor)
                ->Event("Set Jump Falling Gravity Factor", &FirstPersonControllerComponentRequests::SetJumpFallingGravityFactor)
                ->Event("Get X&Y Acceleration Jump Factor", &FirstPersonControllerComponentRequests::GetJumpAccelFactor)
                ->Event("Set X&Y Acceleration Jump Factor", &FirstPersonControllerComponentRequests::SetJumpAccelFactor)
                ->Event("Get Update X&Y Velocity When Ascending", &FirstPersonControllerComponentRequests::GetUpdateXYAscending)
                ->Event("Set Update X&Y Velocity When Ascending", &FirstPersonControllerComponentRequests::SetUpdateXYAscending)
                ->Event("Get Update X&Y Velocity When Descending", &FirstPersonControllerComponentRequests::GetUpdateXYDescending)
                ->Event("Set Update X&Y Velocity When Descending", &FirstPersonControllerComponentRequests::SetUpdateXYDescending)
                ->Event("Get Update X&Y Velocity Only Near Ground", &FirstPersonControllerComponentRequests::GetUpdateXYOnlyNearGround)
                ->Event("Set Update X&Y Velocity Only Near Ground", &FirstPersonControllerComponentRequests::SetUpdateXYOnlyNearGround)
                ->Event("Get Script Sets X&Y Target Velocity", &FirstPersonControllerComponentRequests::GetScriptSetsXYTargetVelocity)
                ->Event("Set Script Sets X&Y Target Velocity", &FirstPersonControllerComponentRequests::SetScriptSetsXYTargetVelocity)
                ->Event("Get Target X&Y Velocity", &FirstPersonControllerComponentRequests::GetTargetXYVelocity)
                ->Event("Set Target X&Y Velocity", &FirstPersonControllerComponentRequests::SetTargetXYVelocity)
                ->Event("Get Z Velocity", &FirstPersonControllerComponentRequests::GetZVelocity)
                ->Event("Set Z Velocity", &FirstPersonControllerComponentRequests::SetZVelocity)
                ->Event("Get Initial Jump Velocity", &FirstPersonControllerComponentRequests::GetJumpInitialVelocity)
                ->Event("Set Initial Jump Velocity", &FirstPersonControllerComponentRequests::SetJumpInitialVelocity)
                ->Event("Get Initial Second Jump Velocity", &FirstPersonControllerComponentRequests::GetJumpSecondInitialVelocity)
                ->Event("Set Initial Second Jump Velocity", &FirstPersonControllerComponentRequests::SetJumpSecondInitialVelocity)
                ->Event("Get Double Jump", &FirstPersonControllerComponentRequests::GetDoubleJump)
                ->Event("Set Double Jump", &FirstPersonControllerComponentRequests::SetDoubleJump)
                ->Event("Get Grounded Offset", &FirstPersonControllerComponentRequests::GetGroundedOffset)
                ->Event("Set Grounded Offset", &FirstPersonControllerComponentRequests::SetGroundedOffset)
                ->Event("Get Ground Close Offset", &FirstPersonControllerComponentRequests::GetGroundCloseOffset)
                ->Event("Set Ground Close Offset", &FirstPersonControllerComponentRequests::SetGroundCloseOffset)
                ->Event("Get Jump Hold Offset", &FirstPersonControllerComponentRequests::GetJumpHoldDistance)
                ->Event("Set Jump Hold Offset", &FirstPersonControllerComponentRequests::SetJumpHoldDistance)
                ->Event("Get Jump Head Sphere Cast Offset", &FirstPersonControllerComponentRequests::GetJumpHeadSphereCastOffset)
                ->Event("Set Jump Head Sphere Cast Offset", &FirstPersonControllerComponentRequests::SetJumpHeadSphereCastOffset)
                ->Event("Get Head Hit", &FirstPersonControllerComponentRequests::GetHeadHit)
                ->Event("Set Head Hit", &FirstPersonControllerComponentRequests::SetHeadHit)
                ->Event("Get Jump Head Ignore Non-Kinematic Rigid Bodies", &FirstPersonControllerComponentRequests::GetJumpHeadIgnoreNonKinematicRigidBodies)
                ->Event("Set Jump Head Ignore Non-Kinematic Rigid Bodies", &FirstPersonControllerComponentRequests::SetJumpHeadIgnoreNonKinematicRigidBodies)
                ->Event("Get Head Collision Group Name", &FirstPersonControllerComponentRequests::GetHeadCollisionGroupName)
                ->Event("Set Head Collision Group Name", &FirstPersonControllerComponentRequests::SetHeadCollisionGroup)
                ->Event("Get Head Hit EntityIds", &FirstPersonControllerComponentRequests::GetHeadHitEntityIds)
                ->Event("Get Sphere Cast Radius Percentage Increase", &FirstPersonControllerComponentRequests::GetGroundedSphereCastRadiusPercentageIncrease)
                ->Event("Set Sphere Cast Radius Percentage Increase", &FirstPersonControllerComponentRequests::SetGroundedSphereCastRadiusPercentageIncrease)
                ->Event("Get Max Grounded Angle (Degrees)", &FirstPersonControllerComponentRequests::GetMaxGroundedAngleDegrees)
                ->Event("Set Max Grounded Angle (Degrees)", &FirstPersonControllerComponentRequests::SetMaxGroundedAngleDegrees)
                ->Event("Get Top Walk Speed", &FirstPersonControllerComponentRequests::GetTopWalkSpeed)
                ->Event("Set Top Walk Speed", &FirstPersonControllerComponentRequests::SetTopWalkSpeed)
                ->Event("Get Walk Acceleration", &FirstPersonControllerComponentRequests::GetWalkAcceleration)
                ->Event("Set Walk Acceleration", &FirstPersonControllerComponentRequests::SetWalkAcceleration)
                ->Event("Get Walk Deceleration", &FirstPersonControllerComponentRequests::GetWalkDeceleration)
                ->Event("Set Walk Deceleration", &FirstPersonControllerComponentRequests::SetWalkDeceleration)
                ->Event("Get Opposing Direction Deceleration Factor", &FirstPersonControllerComponentRequests::GetOpposingDecel)
                ->Event("Set Opposing Direction Deceleration Factor", &FirstPersonControllerComponentRequests::SetOpposingDecel)
                ->Event("Get Instant Velocity Rotation", &FirstPersonControllerComponentRequests::GetInstantVelocityRotation)
                ->Event("Set Instant Velocity Rotation", &FirstPersonControllerComponentRequests::SetInstantVelocityRotation)
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
                ->Event("Get Sprint Max Time", &FirstPersonControllerComponentRequests::GetSprintMaxTime)
                ->Event("Set Sprint Max Time", &FirstPersonControllerComponentRequests::SetSprintMaxTime)
                ->Event("Get Sprint Held Time", &FirstPersonControllerComponentRequests::GetSprintHeldTime)
                ->Event("Set Sprint Held Time", &FirstPersonControllerComponentRequests::SetSprintHeldTime)
                ->Event("Get Sprint Regeneration Rate", &FirstPersonControllerComponentRequests::GetSprintRegenRate)
                ->Event("Set Sprint Regeneration Rate", &FirstPersonControllerComponentRequests::SetSprintRegenRate)
                ->Event("Get Stamina Percentage", &FirstPersonControllerComponentRequests::GetStaminaPercentage)
                ->Event("Set Stamina Percentage", &FirstPersonControllerComponentRequests::SetStaminaPercentage)
                ->Event("Get Sprint Cooldown Time", &FirstPersonControllerComponentRequests::GetSprintCooldownTime)
                ->Event("Set Sprint Cooldown Time", &FirstPersonControllerComponentRequests::SetSprintCooldownTime)
                ->Event("Get Sprint Cooldown", &FirstPersonControllerComponentRequests::GetSprintCooldown)
                ->Event("Set Sprint Cooldown", &FirstPersonControllerComponentRequests::SetSprintCooldown)
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
                ->Event("Get Enable Disable Sprint", &FirstPersonControllerComponentRequests::GetSprintEnableDisableScript)
                ->Event("Set Enable Disable Sprint", &FirstPersonControllerComponentRequests::SetSprintEnableDisableScript)
                ->Event("Get Crouching", &FirstPersonControllerComponentRequests::GetCrouching)
                ->Event("Set Crouching", &FirstPersonControllerComponentRequests::SetCrouching)
                ->Event("Get Crouch Script Locked", &FirstPersonControllerComponentRequests::GetCrouchScriptLocked)
                ->Event("Set Crouch Script Locked", &FirstPersonControllerComponentRequests::SetCrouchScriptLocked)
                ->Event("Get Crouch Scale", &FirstPersonControllerComponentRequests::GetCrouchScale)
                ->Event("Set Crouch Scale", &FirstPersonControllerComponentRequests::SetCrouchScale)
                ->Event("Get Crouch Distance", &FirstPersonControllerComponentRequests::GetCrouchDistance)
                ->Event("Set Crouch Distance", &FirstPersonControllerComponentRequests::SetCrouchDistance)
                ->Event("Get Crouch Time", &FirstPersonControllerComponentRequests::GetCrouchTime)
                ->Event("Set Crouch Time", &FirstPersonControllerComponentRequests::SetCrouchTime)
                ->Event("Get Crouch Enable Toggle", &FirstPersonControllerComponentRequests::GetCrouchEnableToggle)
                ->Event("Set Crouch Enable Toggle", &FirstPersonControllerComponentRequests::SetCrouchEnableToggle)
                ->Event("Get Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchJumpCausesStanding)
                ->Event("Set Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchJumpCausesStanding)
                ->Event("Get Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchSprintCausesStanding)
                ->Event("Set Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchSprintCausesStanding)
                ->Event("Get Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::GetCrouchPriorityWhenSprintPressed)
                ->Event("Set Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::SetCrouchPriorityWhenSprintPressed)
                ->Event("Get Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::GetCameraYawSensitivity)
                ->Event("Set Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::SetCameraYawSensitivity)
                ->Event("Get Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::GetCameraPitchSensitivity)
                ->Event("Set Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::SetCameraPitchSensitivity)
                ->Event("Get Camera Pitch Max Angle (Radians)", &FirstPersonControllerComponentRequests::GetCameraPitchMaxAngleRadians)
                ->Event("Set Camera Pitch Max Angle (Radians)", &FirstPersonControllerComponentRequests::SetCameraPitchMaxAngleRadians)
                ->Event("Get Camera Pitch Max Angle (Degrees)", &FirstPersonControllerComponentRequests::GetCameraPitchMaxAngleDegrees)
                ->Event("Set Camera Pitch Max Angle (Degrees)", &FirstPersonControllerComponentRequests::SetCameraPitchMaxAngleDegrees)
                ->Event("Get Camera Pitch Min Angle (Radians)", &FirstPersonControllerComponentRequests::GetCameraPitchMinAngleRadians)
                ->Event("Set Camera Pitch Min Angle (Radians)", &FirstPersonControllerComponentRequests::SetCameraPitchMinAngleRadians)
                ->Event("Get Camera Pitch Min Angle (Degrees)", &FirstPersonControllerComponentRequests::GetCameraPitchMinAngleDegrees)
                ->Event("Set Camera Pitch Min Angle (Degrees)", &FirstPersonControllerComponentRequests::SetCameraPitchMinAngleDegrees)
                ->Event("Get Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::GetCameraRotationDampFactor)
                ->Event("Set Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::SetCameraRotationDampFactor)
                ->Event("Get Camera Slerp Instead of Lerp Rotation", &FirstPersonControllerComponentRequests::GetCameraSlerpInsteadOfLerpRotation)
                ->Event("Set Camera Slerp Instead of Lerp Rotation", &FirstPersonControllerComponentRequests::SetCameraSlerpInsteadOfLerpRotation)
                ->Event("Get Update Camera Yaw Ignores Input", &FirstPersonControllerComponentRequests::GetUpdateCameraYawIgnoresInput)
                ->Event("Set Update Camera Yaw Ignores Input", &FirstPersonControllerComponentRequests::SetUpdateCameraYawIgnoresInput)
                ->Event("Get Update Camera Pitch Ignores Input", &FirstPersonControllerComponentRequests::GetUpdateCameraPitchIgnoresInput)
                ->Event("Set Update Camera Pitch Ignores Input", &FirstPersonControllerComponentRequests::SetUpdateCameraPitchIgnoresInput)
                ->Event("Update Camera Yaw", &FirstPersonControllerComponentRequests::UpdateCameraYaw)
                ->Event("Update Camera Pitch", &FirstPersonControllerComponentRequests::UpdateCameraPitch)
                ->Event("Get Character Heading", &FirstPersonControllerComponentRequests::GetHeading)
                ->Event("Get Camera Pitch", &FirstPersonControllerComponentRequests::GetPitch);

            bc->Class<FirstPersonControllerComponent>()->RequestBus("FirstPersonControllerComponentRequestBus");
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
        // Obtain the PhysX Character Controller's capsule height and radius
        // and use those dimensions for the ground detection shapecast capsule
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetHeight);
        PhysX::CharacterControllerRequestBus::EventResult(m_capsuleRadius, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetRadius);
        Physics::CharacterRequestBus::EventResult(m_maxGroundedAngleDegrees, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetSlopeLimitDegrees);

        // Set the collision group based on the group Id that is selected
        Physics::CollisionRequestBus::BroadcastResult(
            m_groundedCollisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, m_groundedCollisionGroupId);

        if(m_crouchDistance > m_capsuleHeight - 2.f*m_capsuleRadius)
            m_crouchDistance = m_capsuleHeight - 2.f*m_capsuleRadius;

        // Set the max grounded angle to be slightly greater than the PhysX Character Controller's
        // maximum slope angle value
        m_maxGroundedAngleDegrees += 0.01f;

        // Set the sprint pause time based on whether the cooldown time or the max consecutive sprint time is longer
        // This number can be altered using the RequestBus
        m_sprintPauseTime = (m_sprintCooldownTime > m_sprintMaxTime) ? 0.f : 0.1f * m_sprintCooldownTime;

        //AZ_Printf("", "m_capsuleHeight = %.10f", m_capsuleHeight);
        //AZ_Printf("", "m_capsuleRadius = %.10f", m_capsuleRadius);
        //AZ_Printf("", "m_maxGroundedAngleDegrees = %.10f", m_maxGroundedAngleDegrees);

        UpdateJumpMaxHoldTime();

        AssignConnectInputEvents();

        AZ::TickBus::Handler::BusConnect();

        InputChannelEventListener::Connect();
        // Attempting to allow all possible input events through without filtering anything out
        // This may not be necessary
        AZStd::shared_ptr<AzFramework::InputChannelEventFilterInclusionList> filter;
        AzFramework::InputChannelEventListener::SetFilter(filter);

        FirstPersonControllerComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void FirstPersonControllerComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        InputChannelEventListener::Disconnect();
        FirstPersonControllerComponentRequestBus::Handler::BusDisconnect();
    }

    void FirstPersonControllerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
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
            AZ_Printf("FirstPersonControllerComponent",
                      "ERROR: Number of input IDs not equal to number of input names!");
        }
        else
        {
            for(auto& it_event : m_controlMap)
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

        if(*inputId == m_SprintEventId)
        {
            if(m_grounded)
            {
                m_sprintValue = value;
                m_sprintAccelValue = value * m_sprintAccelScale;
            }
            else
                m_sprintValue = 0.f;
        }

        for(auto& it_event : m_controlMap)
        {
            if(*inputId == *(it_event.first) && !(*(it_event.first) == m_SprintEventId))
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

        for(auto& it_event : m_controlMap)
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
        {
            return;
        }

        if(*inputId == m_RotateYawEventId)
        {
            m_yawValue = value;
        }
        else if(*inputId == m_RotatePitchEventId)
        {
            m_pitchValue = value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if(*inputId == m_SprintEventId)
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
        //AZ_Printf("", "OnInputChannelEventFiltered");
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
        ProcessInput(deltaTime);
    }

    AZ::Entity* FirstPersonControllerComponent::GetActiveCamera() const
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId,
            &Camera::CameraSystemRequestBus::Events::GetActiveCamera);

        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void FirstPersonControllerComponent::SmoothRotation(const float& deltaTime)
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

        if(m_rotationDamp*deltaTime <= 1.f)
        {
            if(m_cameraSlerpInsteadOfLerpRotation)
                m_newLookRotationDelta = m_newLookRotationDelta.Slerp(targetLookRotationDelta, m_rotationDamp*deltaTime);
            else
                m_newLookRotationDelta = m_newLookRotationDelta.Lerp(targetLookRotationDelta, m_rotationDamp*deltaTime);
        }
        else
            m_newLookRotationDelta = targetLookRotationDelta;
    }

    void FirstPersonControllerComponent::UpdateRotation(const float& deltaTime)
    {
        SmoothRotation(deltaTime);
        const AZ::Vector3 newLookRotationDelta = m_newLookRotationDelta.GetEulerRadians();

        AZ::TransformInterface* t = GetEntity()->GetTransform();

        t->RotateAroundLocalZ(newLookRotationDelta.GetZ());

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        float currentPitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(currentPitch <= m_cameraPitchMaxAngle && currentPitch >= m_cameraPitchMinAngle ||
           currentPitch >= HalfPi && newLookRotationDelta.GetX() < 0.f ||
           currentPitch <= -HalfPi && newLookRotationDelta.GetX() > 0.f)
        {
            t->RotateAroundLocalX(newLookRotationDelta.GetX());
            currentPitch = t->GetLocalRotation().GetX();
        }
        if(currentPitch > m_cameraPitchMaxAngle)
            t->RotateAroundLocalX(m_cameraPitchMaxAngle - currentPitch);
        else if(currentPitch < m_cameraPitchMinAngle)
            t->RotateAroundLocalX(m_cameraPitchMinAngle - currentPitch);

        m_currentHeading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();
        m_currentPitch = GetActiveCamera()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetX();
    }

    // Here target velocity is with respect to the character's frame of reference when m_instantVelocityRotation == true
    // and it's with respect to the world when m_instantVelocityRotation == true
    AZ::Vector2 FirstPersonControllerComponent::LerpVelocity(const AZ::Vector2& targetVelocity, const float& deltaTime)
    {
        float totalLerpTime = m_lastAppliedVelocity.GetDistance(targetVelocity)/m_accel;

        // Apply the sprint factor to the acceleration (dt) based on the sprint having been (recently) pressed
        const float lastLerpTime = m_lerpTime;

        float lerpDeltaTime = m_sprintAccumulateAccelTime > 0.f ? deltaTime * m_sprintAccelAdjust : deltaTime;
        if(m_sprintAccelValue < 1.f && m_sprintAccumulateAccelTime > 0.f)
            lerpDeltaTime = deltaTime *  m_sprintAccelAdjust;

        lerpDeltaTime *= m_grounded ? 1.f : m_jumpAccelFactor;

        m_lerpTime += lerpDeltaTime;

        if(m_lerpTime >= totalLerpTime)
            m_lerpTime = totalLerpTime;

        // Lerp the velocity from the last applied velocity to the target velocity
        AZ::Vector2 newVelocity = m_lastAppliedVelocity.Lerp(targetVelocity, m_lerpTime / totalLerpTime);

        // Decelerate at a different rate than the acceleration
        if(newVelocity.GetLength() < m_applyVelocity.GetLength())
        {
            // Get the current velocity vector with respect to the character's local coordinate system
            const AZ::Vector2 applyVelocityLocal = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocity)));

            // Compare the direction of the current velocity vector against the desired direction
            // and if it's greater than 90 degrees then decelerate even more
            if(targetVelocity.GetLength() != 0.f
                && m_instantVelocityRotation ?
                    (abs(applyVelocityLocal.AngleSafe(targetVelocity)) > AZ::Constants::HalfPi)
                    : (abs(m_applyVelocity.AngleSafe(targetVelocity)) > AZ::Constants::HalfPi))
            {
                // Compute the deceleration factor based on the magnitude of the target velocity
                float greatestScale = m_forwardScale;
                for(float scale: {m_forwardScale, m_backScale, m_leftScale, m_rightScale})
                    if(greatestScale < abs(scale))
                        greatestScale = abs(scale);

                AZ::Vector2 targetVelocityLocal = targetVelocity;
                if(!m_instantVelocityRotation)
                    targetVelocityLocal = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(targetVelocity)));

                if(m_standing || m_sprintWhileCrouched)
                    m_decelerationFactor = (m_decel + (m_opposingDecel - m_decel) * targetVelocityLocal.GetLength() / (m_speed * (1.f + (m_sprintVelocityAdjust-1.f)) * greatestScale)) * m_sprintAccelAdjust;
                else
                    m_decelerationFactor = (m_decel + (m_opposingDecel - m_decel) * targetVelocityLocal.GetLength() / (m_speed * m_crouchScale * greatestScale)) * m_sprintAccelAdjust;
            }
            else
                m_decelerationFactor = m_decel;

            // Use the deceleration factor to get the lerp time closer to the total lerp time at a faster rate
            m_lerpTime = lastLerpTime + lerpDeltaTime * m_decelerationFactor;

            if(m_lerpTime >= totalLerpTime)
                m_lerpTime = totalLerpTime;

            AZ::Vector2 newVelocityDecel =  m_lastAppliedVelocity.Lerp(targetVelocity, m_lerpTime / totalLerpTime);
            if(newVelocityDecel.GetLength() < m_applyVelocity.GetLength())
                newVelocity = newVelocityDecel;
        }

        return newVelocity;
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

    // Here target velocity is with respect to the character's frame of reference
    void FirstPersonControllerComponent::SprintManager(const AZ::Vector2& targetVelocity, const float& deltaTime)
    {
        // The sprint value should never be 0, it shouldn't be applied if you're trying to moving backwards,
        // and it shouldn't be applied if you're crouching
        if(m_sprintValue == 0.f
           || (!m_sprintWhileCrouched && !m_standing)
           || (!m_applyVelocity.GetY() && !m_applyVelocity.GetX())
           || (m_forwardValue == -m_backValue && -m_leftValue == m_rightValue)
           || (targetVelocity.IsZero())
           || (m_sprintValue != 0.f
               && !m_sprintBackwards
               && ((!m_forwardValue && !m_leftValue && !m_rightValue) ||
                   (!m_forwardValue && -m_leftValue == m_rightValue) ||
                   (targetVelocity.GetY() < 0.f)) ))
            m_sprintValue = 0.f;

        if((m_sprintViaScript && m_sprintEnableDisableScript) && (targetVelocity.GetY() > 0.f || m_sprintBackwards))
        {
            m_sprintValue = 1.f;
            m_sprintAccelValue = m_sprintAccelScale;
        }
        else if(m_sprintViaScript && !m_sprintEnableDisableScript)
            m_sprintValue = 0.f;

        m_sprintPrevValue = m_sprintValue;

        // Reset the counter if there is no movement
        if(!m_applyVelocity.GetY() && !m_applyVelocity.GetX())
            m_sprintAccumulateAccelTime = 0.f;

        if(m_sprintValue == 0.f)
            m_sprintVelocityAdjust = 1.f;
        else
            m_sprintVelocityAdjust = CreateEllipseScaledVector(targetVelocity.GetNormalized(), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();

        // If sprint is to be applied then increment the sprint counter
        if(!AZ::IsClose(m_sprintVelocityAdjust, 1.f) && m_sprintHeldDuration < m_sprintMaxTime && m_sprintCooldown == 0.f)
        {
            // Cause the character to stand if trying to sprint while crouched and the setting is enabled
            if(m_crouchSprintCausesStanding && m_crouched)
                m_crouching = false;

            // Figure out which of the scaled sprint velocity directions is the greatest
            float greatestSprintScale = 1.f;
            for(const float scale : {m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight})
                if(abs(scale) > abs(greatestSprintScale))
                        greatestSprintScale = scale;

            m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale - 1.f) * (m_sprintVelocityAdjust - 1) + 1.f;

            m_sprintHeldDuration += deltaTime * (m_sprintVelocityAdjust-1.f)/(greatestSprintScale-1.f);

            if(m_sprintHeldDuration > m_sprintMaxTime)
                m_sprintHeldDuration = m_sprintMaxTime;

            m_sprintPause = m_sprintPauseTime;

            m_staminaIncrementing = false;

            if(m_applyVelocity.GetLength() > m_sprintPrevVelocityLength && m_sprintVelocityAdjust != 1.f)
            {
                m_sprintAccumulateAccelTime += deltaTime;

                const float totalSprintTime = (abs(greatestSprintScale)*m_speed)/(m_sprintAccelValue*m_accel);
                if(m_sprintAccumulateAccelTime > totalSprintTime)
                    m_sprintAccumulateAccelTime = totalSprintTime;
            }
            else if(m_applyVelocity.GetLength() < m_sprintPrevVelocityLength)
            {
                const AZ::Vector2 applyVelocityLocal = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocity)));
                if(applyVelocityLocal.AngleSafe(AZ::Vector2::CreateAxisY()) > AZ::Constants::HalfPi)
                    m_sprintAccumulateAccelTime -= deltaTime * m_decelerationFactor;
                else
                    m_sprintAccumulateAccelTime -= deltaTime;

                if(m_sprintAccumulateAccelTime <= 0.f)
                {
                    m_sprintAccumulateAccelTime = 0.f;
                    m_sprintPrevVelocityLength = 0.f;
                }
            }
            m_sprintPrevVelocityLength = m_applyVelocity.GetLength();
        }
        // Otherwise if the sprint velocity isn't applied then decrement the sprint counter
        else if(AZ::IsClose(m_sprintVelocityAdjust, 1.f) || m_sprintHeldDuration >= m_sprintMaxTime || m_sprintCooldown != 0.f)
        {
            m_sprintValue = 0.f;

            // Set the sprint acceleration adjust according to the local direction we're moving
            if((m_instantVelocityRotation || !m_sprintStopAccelAdjustCaptured) && targetVelocity.IsZero())
            {
                // Figure out which of the scaled sprint velocity directions is the greatest
                float greatestSprintScale = 0.f;
                for(const float scale : {m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight})
                    if(abs(scale) > abs(greatestSprintScale))
                            greatestSprintScale = scale;

                float lastAdjustScale = CreateEllipseScaledVector(AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3::CreateAxisY()).GetNormalized()), m_sprintScaleForward, m_sprintScaleBack, m_sprintScaleLeft, m_sprintScaleRight).GetLength();
                m_sprintAccelAdjust = (m_sprintAccelValue - 1.f)/(greatestSprintScale - 1.f) * (lastAdjustScale - 1) + 1.f;
                m_sprintStopAccelAdjustCaptured = true;
            }

            m_sprintAccumulateAccelTime -= deltaTime * m_decelerationFactor;

            if(m_sprintAccumulateAccelTime <= 0.f)
            {
                m_sprintAccumulateAccelTime = 0.f;
                m_sprintPrevVelocityLength = 0.f;
                m_sprintStopAccelAdjustCaptured = false;
                m_sprintAccelAdjust = 1.f;
            }

            // When the sprint held duration exceeds the maximum sprint time then initiate the cooldown period
            if(m_sprintHeldDuration >= m_sprintMaxTime && m_sprintCooldown == 0.f)
            {
                m_sprintHeldDuration = 0.f;
                m_sprintCooldown = m_sprintCooldownTime;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSprintCooldown);
            }
            else if(m_sprintCooldown != 0.f)
            {
                m_sprintCooldown -= deltaTime;
                if(m_sprintCooldown < 0.f)
                    m_sprintCooldown = 0.f;
            }

            if(m_sprintHeldDuration > 0.f && !m_staminaIncrementing && m_sprintPause == 0.f)
            {
                // Here is an option to apply a pause instead of incrementing m_sprintHeldDuration at a
                // rate that results in the same waiting period for the stamina to regenerate to its full
                // value when nearly depleted as compared to waiting the cooldown time
                //m_sprintPause = (m_sprintCooldownTime - m_sprintMaxTime)*(m_sprintHeldDuration/m_sprintMaxTime) + deltaTime;
                m_staminaIncrementing = true;
            }

            m_sprintPause -= deltaTime;

            if(m_sprintPause <= 0.f)
            {
                // Decrement the sprint held duration at a rate which makes it so that the stamina
                // will regenerate when nearly depleted at the same time it would take if you were
                // just wait through the cooldown time.
                // Decrement this value by only deltaTime if you wish to instead use m_sprintPause
                // to achieve the same timing but instead through the use of a pause.
                m_sprintHeldDuration -= deltaTime * ((m_sprintMaxTime+m_sprintPauseTime)/m_sprintCooldownTime) * m_sprintRegenRate;
                m_sprintPause = 0.f;
                if(m_sprintHeldDuration <= 0.f)
                {
                    m_sprintHeldDuration = 0.f;
                    m_staminaIncrementing = false;
                }
            }
        }

        if(m_sprintMaxTime != 0.f)
            m_staminaPercentage = (m_sprintCooldown == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;
        else
            m_staminaPercentage = 0.f;
        //AZ_Printf("", "Stamina = %.10f\%", m_staminaPercentage);
    }

    void FirstPersonControllerComponent::CrouchManager(const float& deltaTime)
    {
        AZ::TransformInterface* cameraTransform = m_activeCameraEntity->GetTransform();

        if(m_crouchEnableToggle && !m_crouchScriptLocked && m_crouchPrevValue == 0.f && m_crouchValue == 1.f)
        {
            m_crouching = !m_crouching;
        }
        else if(!m_crouchEnableToggle && !m_crouchScriptLocked)
        {
            if(m_crouchValue != 0.f)
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
            && m_crouching
            && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
           m_crouching = false;

        //AZ_Printf("", "m_crouching = %s", m_crouching ? "true" : "false");

        // Crouch down
        if(m_crouching && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
        {
            if(m_standing)
                m_standing = false;

            float cameraTravelDelta = -1.f * m_crouchDistance * deltaTime / m_crouchTime;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            if(m_cameraLocalZTravelDistance <= -1.f * m_crouchDistance)
            {
                cameraTravelDelta += abs(m_cameraLocalZTravelDistance) - m_crouchDistance;
                m_cameraLocalZTravelDistance = -1.f * m_crouchDistance;
                m_crouched = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnCrouched);
            }

            // Adjust the height of the collider capsule based on the crouching height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Subtract the distance to get down to the crouching height
            m_capsuleHeight += cameraTravelDelta;
            //AZ_Printf("", "Crouching capsule height = %.10f", m_capsuleHeight);

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleHeight);

            cameraTransform->SetLocalZ(cameraTransform->GetLocalZ() + cameraTravelDelta);
        }
        // Stand up
        else if(!m_crouching && m_cameraLocalZTravelDistance != 0.f)
        {
            if(m_crouched)
                m_crouched = false;

            // Create a shapecast sphere that will be used to detect whether there is an obstruction
            // above the players head, and prevent them from fully standing up if there is
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

            // Move the sphere to the location of the character and apply the Z offset
            sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius));

            AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ();

            // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
            if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
            {
                sphereCastDirection = m_sphereCastsAxisDirectionPose;
                if(m_sphereCastsAxisDirectionPose.GetZ() >= 0.f)
                    sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius)));
                else
                    sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius)));
            }

            AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(

                m_capsuleRadius,
                sphereCastPose,
                sphereCastDirection,
                m_uncrouchHeadSphereCastOffset,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                AzPhysics::CollisionGroup::All,
                nullptr);

            request.m_reportMultipleHits = true;

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

            // Disregard intersections with the character's collider and its child entities,
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

            // Bail if something is detected above the player
            if(hits)
                return;

            float cameraTravelDelta = m_crouchDistance * deltaTime / m_crouchTime;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            if(m_cameraLocalZTravelDistance >= 0.f)
            {
                cameraTravelDelta -= m_cameraLocalZTravelDistance;
                m_cameraLocalZTravelDistance = 0.f;
                m_standing = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnStoodUp);
            }

            // Adjust the height of the collider capsule based on the standing height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Add the distance to get back to the standing height
            m_capsuleHeight += cameraTravelDelta;
            //AZ_Printf("", "Standing capsule height = %.10f", m_capsuleHeight);

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleHeight);

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

        if(leftRight >= 0.f)
            leftRight /= m_rightScale;
        else
            leftRight /= m_leftScale;

        AZ::Vector2 targetVelocity = AZ::Vector2(leftRight, forwardBack);

        // Normalize the vector if its magnitude is greater than 1 and then scale it
        if((forwardBack || leftRight) && sqrt(forwardBack*forwardBack + leftRight*leftRight) > 1.f)
            targetVelocity.Normalize();

        targetVelocity = CreateEllipseScaledVector(targetVelocity, m_forwardScale, m_backScale, m_leftScale, m_rightScale);

        // Call the sprint manager
        if(!m_scriptSetsXYTargetVelocity)
            SprintManager(targetVelocity, deltaTime);

        // Apply the speed, sprint factor, and crouch factor
        if(m_standing)
            targetVelocity *= m_speed * m_sprintVelocityAdjust;
        else if(m_sprintWhileCrouched && !m_standing)
            targetVelocity *= m_speed * m_sprintVelocityAdjust * m_crouchScale;
        else
            targetVelocity *= m_speed * m_crouchScale;

        if(m_scriptSetsXYTargetVelocity)
        {
            targetVelocity.SetX(m_scriptTargetXYVelocity.GetX());
            targetVelocity.SetY(m_scriptTargetXYVelocity.GetY());
            SprintManager(targetVelocity, deltaTime);
        }
        else
            m_scriptTargetXYVelocity = targetVelocity;

        // Rotate the target velocity vector so that it can be compared against the applied velocity
        const AZ::Vector2 targetVelocityWorld = AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3(targetVelocity)));

        // Obtain the last applied velocity if the target velocity changed
        if(m_instantVelocityRotation ? (m_prevTargetVelocity != targetVelocity)
                                        : (m_prevTargetVelocity != targetVelocityWorld))
        {
            if(m_instantVelocityRotation)
            {
                // Set the previous target velocity to the new one
                m_prevTargetVelocity = targetVelocity;
                // Store the last applied velocity to be used for the lerping
                m_lastAppliedVelocity = AZ::Vector2(AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(AZ::Vector3(m_applyVelocity)));
            }
            else
            {
                // Set the previous target velocity to the new one
                m_prevTargetVelocity = targetVelocityWorld;
                // Store the last applied velocity to be used for the lerping
                m_lastAppliedVelocity = m_applyVelocity;
            }

            // Reset the lerp time since the target velocity changed
            m_lerpTime = 0.f;
        }

        // Lerp to the velocity if we're not already there
        if(m_applyVelocity != targetVelocityWorld)
        {
            if(m_instantVelocityRotation)
                m_applyVelocity = AZ::Vector2(AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(AZ::Vector3(LerpVelocity(targetVelocity, deltaTime))));
            else
                m_applyVelocity = LerpVelocity(targetVelocityWorld, deltaTime);
        }

        // Debug print statements to observe the velocity, acceleration, and position
        //AZ_Printf("", "m_currentHeading = %.10f", m_currentHeading);
        //AZ_Printf("", "m_applyVelocity.GetLength() = %.10f", m_applyVelocity.GetLength());
        //AZ_Printf("", "m_applyVelocity.GetX() = %.10f", m_applyVelocity.GetX());
        //AZ_Printf("", "m_applyVelocity.GetY() = %.10f", m_applyVelocity.GetY());
        //AZ_Printf("", "m_sprintAccumulateAccelTime = %.10f", m_sprintAccumulateAccelTime);
        //AZ_Printf("", "m_sprintValue = %.10f", m_sprintValue);
        //AZ_Printf("", "m_sprintAccelValue = %.10f", m_sprintAccelValue);
        //AZ_Printf("", "m_sprintAccelAdjust = %.10f", m_sprintAccelAdjust);
        //AZ_Printf("", "m_decelerationFactor = %.10f", m_decelerationFactor);
        //AZ_Printf("", "m_sprintVelocityAdjust = %.10f", m_sprintVelocityAdjust);
        //AZ_Printf("", "m_sprintHeldDuration = %.10f", m_sprintHeldDuration);
        //AZ_Printf("", "m_sprintPause = %.10f", m_sprintPause);
        //AZ_Printf("", "m_sprintPauseTime = %.10f", m_sprintPauseTime);
        //AZ_Printf("", "m_sprintCooldown = %.10f", m_sprintCooldown);
        //static AZ::Vector2 prevVelocity = m_applyVelocity;
        //AZ_Printf("", "dv/dt = %.10f", prevVelocity.GetDistance(m_applyVelocity)/deltaTime);
        //prevVelocity = m_applyVelocity;
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "X Position = %.10f", pos.GetX());
        //AZ_Printf("", "Y Position = %.10f", pos.GetY());
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("","");
    }

    void FirstPersonControllerComponent::CheckGrounded(const float& deltaTime)
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Used to determine when event notifications occur
        const bool prevGrounded = m_grounded;
        const bool prevGroundClose = m_groundClose;

        AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

        // Move the sphere to the location of the character and apply the Z offset
        sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ((1.f + m_groundedSphereCastRadiusPercentageIncrease/100.f)*m_capsuleRadius));

        AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ(-1.f);

        // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
        if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
        {
            sphereCastDirection = -m_sphereCastsAxisDirectionPose;
            if(m_sphereCastsAxisDirectionPose.GetZ() > 0.f)
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ((1.f + m_groundedSphereCastRadiusPercentageIncrease/100.f)*m_capsuleRadius)));
            else
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ((1.f + m_groundedSphereCastRadiusPercentageIncrease/100.f)*m_capsuleRadius)));
        }

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            (1.f + m_groundedSphereCastRadiusPercentageIncrease/100.f)*m_capsuleRadius,
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

        bool groundedOtherwiseGroundClose = true;
        // Disregard intersections with the character's collider, its child entities,
        // and if the slope angle of the thing that's intersecting is greater than the max grounded angle
        auto selfChildSlopeEntityCheck = [this, &steepNormals, &groundedOtherwiseGroundClose](AzPhysics::SceneQueryHit& hit)
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
                    //AZ_Printf("", "Steep Angle EntityId = %s", hit.m_entityId.ToString().c_str());
                    //AZ_Printf("", "Steep Angle = %.10f", hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
                    return true;
                }

                if(groundedOtherwiseGroundClose)
                    m_groundHits.push_back(hit);
                else
                    m_groundCloseHits.push_back(hit);

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

            //AZ_Printf("", "Sum of Steep Angles = %.10f", sumNormals.AngleSafeDeg(m_sphereCastsAxisDirectionPose));
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

        if(m_grounded)
            m_airTime = 0.f;

        // Check to see if the character is close to an acceptable ground
        m_airTime += deltaTime;

        request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            (1.f + m_groundedSphereCastRadiusPercentageIncrease/100.f)*m_capsuleRadius,
            sphereCastPose,
            sphereCastDirection,
            m_groundCloseSphereCastOffset,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            m_groundedCollisionGroup,
            nullptr);

        request.m_reportMultipleHits = true;

        groundedOtherwiseGroundClose = false;

        hits = sceneInterface->QueryScene(sceneHandle, &request);
        m_groundCloseHits.clear();
        AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
        m_groundClose = hits ? true : false;

        if(m_scriptSetGroundCloseTick)
        {
            m_groundClose = m_scriptGroundClose;
            m_scriptSetGroundCloseTick = false;
        }
        //AZ_Printf("", "m_groundClose = %s", m_groundClose ? "true" : "false");

        // Trigger an event notification if the player hits the ground, is about to hit the ground,
        // or just left the ground (via jumping or otherwise)
        if(!prevGrounded && m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundHit);
        else if(!prevGroundClose && m_groundClose)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundSoonHit);
        else if(prevGrounded && !m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnUngrounded);
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
            m_jumpMaxHoldTime = abs(m_jumpInitialVelocity / (m_gravity*m_jumpHeldGravityFactor));
    }

    void FirstPersonControllerComponent::UpdateVelocityZ(const float& deltaTime)
    {
        // Create a shapecast sphere that will be used to detect whether there is an obstruction
        // above the players head, and prevent them from fully standing up if there is
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        AZ::Transform sphereCastPose = AZ::Transform::CreateIdentity();

        // Move the sphere to the location of the character and apply the Z offset
        sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius));

        AZ::Vector3 sphereCastDirection = AZ::Vector3::CreateAxisZ();

        // Adjust the pose and direction of the sphere cast based on m_sphereCastsAxisDirectionPose
        if(m_sphereCastsAxisDirectionPose != AZ::Vector3::CreateAxisZ())
        {
            sphereCastDirection = m_sphereCastsAxisDirectionPose;
            if(m_sphereCastsAxisDirectionPose.GetZ() >= 0.f)
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_sphereCastsAxisDirectionPose).TransformVector(AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius)));
            else
                sphereCastPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_sphereCastsAxisDirectionPose).TransformVector(-AZ::Vector3::CreateAxisZ(m_capsuleHeight - m_capsuleRadius)));
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

        // Disregard intersections with the character's collider and its child entities,
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

                if(m_jumpHeadIgnoreNonKinematicRigidBodies)
                {
                    // Check to see if the entity hit is kinematic
                    bool isKinematic = true;
                    Physics::RigidBodyRequestBus::EventResult(isKinematic, hit.m_entityId,
                        &Physics::RigidBodyRequests::IsKinematic);

                    if(!isKinematic)
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

        if(m_headHit)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnHeadHit);

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

        // Used for the Verlet integration averaging calculation
        m_zVelocityPrevDelta = m_zVelocityCurrentDelta;

        if(m_grounded && (m_jumpReqRepress || (currentVelocity.GetZ() <= 0.f && m_zVelocity <= 0.f)))
        {
            if(m_jumpValue && !m_jumpHeld && !m_headHit)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                        m_crouching = false;
                    return;
                }
                m_zVelocityCurrentDelta = m_jumpInitialVelocity;
                m_jumpHeld = true;
                m_jumpReqRepress = false;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnFirstJump);
            }
            else
            {
                m_zVelocity = 0.f;
                m_zVelocityCurrentDelta = 0.f;
                m_jumpCounter = 0.f;

                if(m_jumpValue == 0.f && m_jumpHeld)
                    m_jumpHeld = false;

                if(m_doubleJumpEnabled && m_secondJump)
                    m_secondJump = false;
            }
        }
        else if(m_jumpCounter < m_jumpMaxHoldTime && currentVelocity.GetZ() > 0.f && m_jumpHeld && !m_jumpReqRepress)
        {
            if(m_jumpValue == 0.f)
            {
                m_jumpHeld = false;
                m_jumpCounter = 0.f;
                m_zVelocityCurrentDelta = m_gravity * deltaTime;
            }
            else
            {
                m_jumpCounter += deltaTime;
                m_zVelocityCurrentDelta = m_gravity * m_jumpHeldGravityFactor * deltaTime;
            }
        }
        else
        {
            if(!m_jumpReqRepress)
                m_jumpReqRepress = true;

            if(m_jumpCounter != 0.f)
                m_jumpCounter = 0.f;

            if(currentVelocity.GetZ() <= 0.f)
                m_zVelocityCurrentDelta = m_gravity * m_jumpFallingGravityFactor * deltaTime;
            else
                m_zVelocityCurrentDelta = m_gravity * deltaTime;

            if(!m_doubleJumpEnabled && !m_jumpHeld)
                m_jumpHeld = true;
            else if(m_doubleJumpEnabled && m_jumpValue == 0.f && m_jumpHeld)
                m_jumpHeld = false;

            if(m_doubleJumpEnabled && !m_secondJump && !m_jumpHeld && m_jumpValue != 0.f)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                        m_crouching = false;
                    return;
                }
                m_zVelocity = m_jumpSecondInitialVelocity;
                m_zVelocityCurrentDelta = 0.f;
                m_secondJump = true;
                m_jumpHeld = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSecondJump);
            }
        }

        // Perform an average of the current and previous Z velocity delta
        // as described by Verlet integration, which should reduce accumulated error
        m_zVelocity += (m_zVelocityCurrentDelta + m_zVelocityPrevDelta) / 2.f;

        if(m_headHit && m_zVelocity > 0.f)
            m_zVelocity = m_zVelocityCurrentDelta = 0.f;

        // Account for the case where the PhysX Character Gameplay component's gravity is used instead
        if(m_gravity == 0.f && m_grounded && currentVelocity.GetZ() < 0.f)
            m_zVelocity = m_zVelocityCurrentDelta = 0.f;

        // Debug print statements to observe the jump mechanic
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("", "currentVelocity.GetZ() = %.10f", currentVelocity.GetZ());
        //AZ_Printf("", "m_zVelocityPrevDelta = %.10f", m_zVelocityPrevDelta);
        //AZ_Printf("", "m_zVelocityCurrentDelta = %.10f", m_zVelocityCurrentDelta);
        //AZ_Printf("", "m_zVelocity = %.10f", m_zVelocity);
        //AZ_Printf("", "m_grounded = %s", m_grounded ? "true" : "false");
        //AZ_Printf("", "m_jumpCounter = %.10f", m_jumpCounter);
        //AZ_Printf("", "deltaTime = %.10f", deltaTime);
        //AZ_Printf("", "m_jumpMaxHoldTime = %.10f", m_jumpMaxHoldTime);
        //AZ_Printf("", "m_jumpHoldDistance = %.10f", m_jumpHoldDistance);
        //static float prevZVelocity = m_zVelocity;
        //AZ_Printf("", "dvz/dt = %.10f", (m_zVelocity - prevZVelocity)/deltaTime);
        //AZ_Printf("","");
        //prevZVelocity = m_zVelocity;
    }

    void FirstPersonControllerComponent::UpdateXYVelocityPlaneTilt(AZ::Vector3& targetVelocity)
    {
        // Track the sum of the normal vectors for the velocity's XY plane if its set
        if(m_velocityXCrossYTracksNormal)
            SetVelocityXCrossYDirection(GetGroundSumNormalsDirection());

        if(m_velocityXCrossYDirection != AZ::Vector3::CreateAxisZ())
        {
            if(m_velocityXCrossYDirection.GetZ() >= 0.f)
                targetVelocity = AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_velocityXCrossYDirection).TransformVector(targetVelocity);
            else
            {
                targetVelocity.SetY(-targetVelocity.GetY());
                targetVelocity = AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_velocityXCrossYDirection).TransformVector(targetVelocity);
            }
        }
    }

    void FirstPersonControllerComponent::UpdateZVelocityPosDirection(AZ::Vector3& targetVelocity)
    {
        if(m_velocityZPosDirection != AZ::Vector3::CreateAxisZ())
        {
            if(m_velocityZPosDirection.GetZ() >= 0.f)
                targetVelocity += AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(), m_velocityZPosDirection).TransformVector(AZ::Vector3::CreateAxisZ(m_zVelocity));
            else
                targetVelocity += AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisZ(-1.f), m_velocityZPosDirection).TransformVector(AZ::Vector3::CreateAxisZ(-m_zVelocity));
        }
        else
            targetVelocity += AZ::Vector3::CreateAxisZ(m_zVelocity);
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime)
    {
        UpdateRotation(deltaTime);

        CheckGrounded(deltaTime);

        if(m_grounded)
            CrouchManager(deltaTime);

        // So long as the character is grounded or depending on how the update X&Y velocity while jumping
        // boolean values are set, and based on the state of jumping/falling, update the X&Y velocity accordingly
        if(m_grounded || (m_updateXYAscending && m_updateXYDecending && !m_updateXYOnlyNearGround)
           || ((m_updateXYAscending && m_zVelocity >= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose))
           || ((m_updateXYDecending && m_zVelocity <= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose)) )
            UpdateVelocityXY(deltaTime);

        UpdateVelocityZ(deltaTime);

        AZ::Vector3 targetVelocity = AZ::Vector3(m_applyVelocity);

        // Tilt the XY velocity plane based on m_velocityXCrossYDirection
        UpdateXYVelocityPlaneTilt(targetVelocity);
        // Change the +Z direction based on m_velocityZPosDirection
        UpdateZVelocityPosDirection(targetVelocity);

        // Placed here for when CharacterControllerComponent::SetUpDirection() is implemented
        /* Physics::CharacterRequestBus::Event(GetEntityId(),
              &Physics::CharacterRequestBus::Events::SetUpDirection, m_velocityZPosDirection); */

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            targetVelocity);
    }

    // Event Notification methods for use in scripts
    void FirstPersonControllerComponent::OnGroundHit(){}
    void FirstPersonControllerComponent::OnGroundSoonHit(){}
    void FirstPersonControllerComponent::OnUngrounded(){}
    void FirstPersonControllerComponent::OnHeadHit(){}
    void FirstPersonControllerComponent::OnCrouched(){}
    void FirstPersonControllerComponent::OnStoodUp(){}
    void FirstPersonControllerComponent::OnFirstJump(){}
    void FirstPersonControllerComponent::OnSecondJump(){}
    void FirstPersonControllerComponent::OnSprintCooldown(){}

    // Request Bus getter and setter methods for use in scripts
    AZ::EntityId FirstPersonControllerComponent::GetActiveCameraId() const
    {
        return m_activeCameraEntity->GetId();
    }
    void FirstPersonControllerComponent::ReacquireChildEntityIds()
    {
        AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
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
    bool FirstPersonControllerComponent::GetGrounded() const
    {
        return m_grounded;
    }
    void FirstPersonControllerComponent::SetGroundedForTick(const bool& new_grounded)
    {
        m_scriptGrounded = new_grounded;
        m_scriptSetGroundTick = true;
    }
    AZStd::vector<AzPhysics::SceneQueryHit> FirstPersonControllerComponent::GetGroundSceneQueryHits() const
    {
        return m_groundHits;
    }
    AZStd::vector<AzPhysics::SceneQueryHit> FirstPersonControllerComponent::GetGroundCloseSceneQueryHits() const
    {
        return m_groundCloseHits;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetGroundSumNormalsDirection() const
    {
        if(m_groundHits.empty())
            return AZ::Vector3::CreateAxisZ();
        AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
        for(AzPhysics::SceneQueryHit hit : m_groundHits)
            sumNormals += hit.m_normal;
        return sumNormals.GetNormalized();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetGroundCloseSumNormalsDirection() const
    {
        if(m_groundCloseHits.empty())
            return AZ::Vector3::CreateAxisZ();
        AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
        for(AzPhysics::SceneQueryHit hit : m_groundCloseHits)
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
    Physics::Shape* FirstPersonControllerComponent::GetSceneQueryHitShapePtr(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_shape;
    }
    AzPhysics::SimulatedBodyHandle FirstPersonControllerComponent::GetSceneQueryHitSimulatedBodyHandle(AzPhysics::SceneQueryHit hit) const
    {
        return hit.m_bodyHandle;
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
    AZ::Vector3 FirstPersonControllerComponent::GetVelocityXCrossYDirection() const
    {
        return m_velocityXCrossYDirection;
    }
    void FirstPersonControllerComponent::SetVelocityXCrossYDirection(const AZ::Vector3& new_velocityXCrossYDirection)
    {
        m_velocityXCrossYDirection = new_velocityXCrossYDirection;
        if(m_velocityXCrossYDirection.IsZero())
            m_velocityXCrossYDirection = AZ::Vector3::CreateAxisZ();
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVelocityZPosDirection() const
    {
        return m_velocityZPosDirection;
    }
    void FirstPersonControllerComponent::SetVelocityZPosDirection(const AZ::Vector3& new_velocityZPosDirection)
    {
        m_velocityZPosDirection = new_velocityZPosDirection;
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
    }
    bool FirstPersonControllerComponent::GetVelocityXCrossYTracksNormal() const
    {
        return m_velocityXCrossYTracksNormal;
    }
    void FirstPersonControllerComponent::SetVelocityXCrossYTracksNormal(const bool& new_velocityXCrossYTracksNormal)
    {
        m_velocityXCrossYTracksNormal = new_velocityXCrossYTracksNormal;
    }
    AZ::Vector3 FirstPersonControllerComponent::GetVectorAnglesBetweenVectorsRadians(const AZ::Vector3& v1, const AZ::Vector3& v2)
    {
        if(v1 == v2)
            return AZ::Vector3::CreateZero();
        AZ::Vector3 angle = AZ::Quaternion::CreateShortestArc(v1, v2).ConvertToScaledAxisAngle();
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
        return m_updateXYAscending;
    }
    void FirstPersonControllerComponent::SetUpdateXYDescending(const bool& new_updateXYDecending)
    {
        m_updateXYDecending = new_updateXYDecending;
    }
    bool FirstPersonControllerComponent::GetUpdateXYOnlyNearGround() const
    {
        return m_updateXYOnlyNearGround;
    }
    void FirstPersonControllerComponent::SetUpdateXYOnlyNearGround(const bool& new_updateXYOnlyNearGround)
    {
        m_updateXYOnlyNearGround = new_updateXYOnlyNearGround;
    }
    bool FirstPersonControllerComponent::GetScriptSetsXYTargetVelocity() const
    {
        return m_scriptSetsXYTargetVelocity;
    }
    void FirstPersonControllerComponent::SetScriptSetsXYTargetVelocity(const bool& new_scriptSetsXYTargetVelocity)
    {
        m_scriptSetsXYTargetVelocity = new_scriptSetsXYTargetVelocity;
    }
    AZ::Vector2 FirstPersonControllerComponent::GetTargetXYVelocity() const
    {
        return m_scriptTargetXYVelocity;
    }
    void FirstPersonControllerComponent::SetTargetXYVelocity(const AZ::Vector2& new_scriptTargetXYVelocity)
    {
        m_scriptTargetXYVelocity = new_scriptTargetXYVelocity;
    }
    float FirstPersonControllerComponent::GetZVelocity() const
    {
        return m_zVelocity;
    }
    void FirstPersonControllerComponent::SetZVelocity(const float& new_zVelocity)
    {
        SetGroundedForTick(false);
        m_zVelocity = new_zVelocity;
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
    bool FirstPersonControllerComponent::GetDoubleJump() const
    {
        return m_doubleJumpEnabled;
    }
    void FirstPersonControllerComponent::SetDoubleJump(const bool& new_doubleJumpEnabled)
    {
        m_doubleJumpEnabled = new_doubleJumpEnabled;
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
    bool FirstPersonControllerComponent::GetHeadHit() const
    {
        return m_headHit;
    }
    void FirstPersonControllerComponent::SetHeadHit(const bool& new_headHit)
    {
        m_headHit = new_headHit;
    }
    bool FirstPersonControllerComponent::GetJumpHeadIgnoreNonKinematicRigidBodies() const
    {
        return m_jumpHeadIgnoreNonKinematicRigidBodies;
    }
    void FirstPersonControllerComponent::SetJumpHeadIgnoreNonKinematicRigidBodies(const bool& new_jumpHeadIgnoreNonKinematicRigidBodies)
    {
        m_jumpHeadIgnoreNonKinematicRigidBodies = new_jumpHeadIgnoreNonKinematicRigidBodies;
    }
    AZStd::string FirstPersonControllerComponent::GetHeadCollisionGroupName() const
    {
        AZStd::string groupName;
        Physics::CollisionRequestBus::BroadcastResult(
            groupName, &Physics::CollisionRequests::GetCollisionGroupName, m_headCollisionGroup);
        return groupName;
    }
    void FirstPersonControllerComponent::SetHeadCollisionGroup(const AZStd::string& new_headCollisionGroupName)
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
    float FirstPersonControllerComponent::GetGroundedSphereCastRadiusPercentageIncrease() const
    {
        return m_groundedSphereCastRadiusPercentageIncrease;
    }
    void FirstPersonControllerComponent::SetGroundedSphereCastRadiusPercentageIncrease(const float& new_groundedSphereCastRadiusPercentageIncrease)
    {
        m_groundedSphereCastRadiusPercentageIncrease = new_groundedSphereCastRadiusPercentageIncrease;
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
    float FirstPersonControllerComponent::GetWalkDeceleration() const
    {
        return m_decel;
    }
    void FirstPersonControllerComponent::SetWalkDeceleration(const float& new_decel)
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
    bool FirstPersonControllerComponent::GetInstantVelocityRotation() const
    {
        return m_instantVelocityRotation;
    }
    void FirstPersonControllerComponent::SetInstantVelocityRotation(const bool& new_instantVelocityRotation)
    {
        m_instantVelocityRotation = new_instantVelocityRotation;
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
    float FirstPersonControllerComponent::GetSprintMaxTime() const
    {
        return m_sprintMaxTime;
    }
    void FirstPersonControllerComponent::SetSprintMaxTime(const float& new_sprintMaxTime)
    {
        m_sprintMaxTime = new_sprintMaxTime;
        m_staminaPercentage = (m_sprintCooldown == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;
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
        m_staminaPercentage = (m_sprintCooldown == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;
        if(m_sprintHeldDuration > prevSprintHeldDuration)
            m_staminaIncrementing = false;
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
            m_staminaIncrementing = false;
    }
    float FirstPersonControllerComponent::GetSprintCooldownTime() const
    {
        return m_sprintCooldownTime;
    }
    void FirstPersonControllerComponent::SetSprintCooldownTime(const float& new_sprintCooldownTime)
    {
        m_sprintCooldownTime = new_sprintCooldownTime;
    }
    float FirstPersonControllerComponent::GetSprintCooldown() const
    {
        return m_sprintCooldown;
    }
    void FirstPersonControllerComponent::SetSprintCooldown(const float& new_sprintCooldown)
    {
        m_sprintCooldown = new_sprintCooldown;
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
    bool FirstPersonControllerComponent::GetSprintEnableDisableScript() const
    {
        return m_sprintEnableDisableScript;
    }
    void FirstPersonControllerComponent::SetSprintEnableDisableScript(const bool& new_sprintEnableDisableScript)
    {
        m_sprintEnableDisableScript = new_sprintEnableDisableScript;
    }
    bool FirstPersonControllerComponent::GetCrouching() const
    {
        return m_crouching;
    }
    void FirstPersonControllerComponent::SetCrouching(const bool& new_crouching)
    {
        m_crouching = new_crouching;
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
    }
    float FirstPersonControllerComponent::GetCrouchTime() const
    {
        return m_crouchTime;
    }
    void FirstPersonControllerComponent::SetCrouchTime(const float& new_crouchTime)
    {
        m_crouchTime = new_crouchTime;
    }
    float FirstPersonControllerComponent::GetUncrouchHeadSphereCastOffset() const
    {
        return m_uncrouchHeadSphereCastOffset;
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
    float FirstPersonControllerComponent::GetCameraYawSensitivity() const
    {
        return m_yawSensitivity;
    }
    void FirstPersonControllerComponent::SetCameraYawSensitivity(const float& new_yawSensitivity)
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
    bool FirstPersonControllerComponent::GetCameraSlerpInsteadOfLerpRotation() const
    {
        return m_cameraSlerpInsteadOfLerpRotation;
    }
    void FirstPersonControllerComponent::SetCameraSlerpInsteadOfLerpRotation(const bool& new_cameraSlerpInsteadOfLerpRotation)
    {
        m_cameraSlerpInsteadOfLerpRotation = new_cameraSlerpInsteadOfLerpRotation;
    }
    bool FirstPersonControllerComponent::GetUpdateCameraYawIgnoresInput() const
    {
        return m_updateCameraYawIgnoresInput;
    }
    void FirstPersonControllerComponent::SetUpdateCameraYawIgnoresInput(const bool& new_updateCameraYawIgnoresInput)
    {
        m_updateCameraYawIgnoresInput = new_updateCameraYawIgnoresInput;
    }
    bool FirstPersonControllerComponent::GetUpdateCameraPitchIgnoresInput() const
    {
        return m_updateCameraPitchIgnoresInput;
    }
    void FirstPersonControllerComponent::SetUpdateCameraPitchIgnoresInput(const bool& new_updateCameraPitchIgnoresInput)
    {
        m_updateCameraPitchIgnoresInput = new_updateCameraPitchIgnoresInput;
    }
    void FirstPersonControllerComponent::UpdateCameraYaw(const float& new_cameraYawAngle)
    {
        if(m_updateCameraYawIgnoresInput)
            m_cameraRotationAngles[2] = new_cameraYawAngle;
        else
            m_cameraRotationAngles[2] = new_cameraYawAngle - m_yawValue * m_yawSensitivity;
        m_rotatingYawViaScriptGamepad = true;
    }
    void FirstPersonControllerComponent::UpdateCameraPitch(const float& new_cameraPitchAngle)
    {
        if(m_updateCameraPitchIgnoresInput)
            m_cameraRotationAngles[0] = new_cameraPitchAngle;
        else
            m_cameraRotationAngles[0] = new_cameraPitchAngle - m_pitchValue * m_pitchSensitivity;
        m_rotatingPitchViaScriptGamepad = true;
    }
    float FirstPersonControllerComponent::GetHeading() const
    {
        return m_currentHeading;
    }
    float FirstPersonControllerComponent::GetPitch() const
    {
        return m_currentPitch;
    }
}
