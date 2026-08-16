/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>
#ifdef NETWORKFPC
#include <Multiplayer/NetworkFPC.h>
#endif

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Physics/NameConstants.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonExtrasComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonExtrasComponent, AZ::Component>()
                // Jumping group
                ->Field("Jump Pressed In Air Queue Time Threshold", &FirstPersonExtrasComponent::m_jumpPressedInAirQueueTimeThreshold)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                ->Attribute(AZ::Edit::Attributes::Min, 0.f)

                // Jump Head Tilt group
                ->Field("Jump Head Tilt", &FirstPersonExtrasComponent::m_jumpHeadTiltEnabled)
                ->Field("Head Angle Jump", &FirstPersonExtrasComponent::m_headAngleJump)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Head Angle Land", &FirstPersonExtrasComponent::m_headAngleLand)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Jump Delta Angle Speed Factor", &FirstPersonExtrasComponent::m_deltaAngleFactorJump)
                ->Field("Land Delta Angle Speed Factor", &FirstPersonExtrasComponent::m_deltaAngleFactorLand)
                ->Field("Complete Head Angle Land Velocity", &FirstPersonExtrasComponent::m_completeHeadLandVelocity)
                ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())

                // Sprint FoV Group
                ->Field("Sprint FoV", &FirstPersonExtrasComponent::m_sprintFoVEnabled)
                ->Field("FoV Increase When Sprinting", &FirstPersonExtrasComponent::m_sprintFoVDelta)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Sprint FoV Lerp Time", &FirstPersonExtrasComponent::m_sprintFoVLerpTime)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                ->Attribute(AZ::Edit::Attributes::Min, 0.0166f)

                // Headbob group
                ->Field("Headbob", &FirstPersonExtrasComponent::m_headbobEnabled)
                ->Field("Headbob Starting Direction", &FirstPersonExtrasComponent::m_headbobStartingDirection)
                ->Field("Headbob Overall Intensity", &FirstPersonExtrasComponent::m_headbobOverallIntensity)
                ->Field("Headbob Max Frequency Hz", &FirstPersonExtrasComponent::m_headbobMaxFrequency)
                ->Attribute(AZ::Edit::Attributes::Suffix, " Hz")
                ->Field("Headbob Max Vertical Amplitude", &FirstPersonExtrasComponent::m_headbobMaxVerticalAmplitude)
                ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
                ->Field("Headbob Max Horizontal Amplitude", &FirstPersonExtrasComponent::m_headbobMaxHorizontalAmplitude)
                ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
                ->Field("Headbob Smooth Time", &FirstPersonExtrasComponent::m_headbobSmoothTime)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                ->Field("Headbob Realism", &FirstPersonExtrasComponent::m_headbobRealism)
                ->Field("Headbob Max Pitch Amplitude", &FirstPersonExtrasComponent::m_headbobMaxPitchAmplitude)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Headbob Max Roll Amplitude", &FirstPersonExtrasComponent::m_headbobMaxRollAmplitude)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Headbob Max Yaw Amplitude", &FirstPersonExtrasComponent::m_headbobMaxYawAmplitude)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Headbob Footstep Sharpness", &FirstPersonExtrasComponent::m_headbobFootstepSharpness)
                ->Field("Headbob Alternating Step Difference", &FirstPersonExtrasComponent::m_headbobAlternatingStepDifference)
                ->Field("Headbob Horizontal Sway Imbalance", &FirstPersonExtrasComponent::m_headbobHorizontalSwayImbalance)
                ->Field("Headbob Horizontal Sway Flatness", &FirstPersonExtrasComponent::m_headbobHorizontalSwayFlatness)
                ->Field("Headbob Footstep Acceleration", &FirstPersonExtrasComponent::m_headbobFootstepAcceleration)
                ->Field("Headbob Step Variation Over Time", &FirstPersonExtrasComponent::m_headbobStepVariationOverTime)
                ->Field("Headbob Vertical Sprint Scale", &FirstPersonExtrasComponent::m_headbobVerticalSprintScale)
                ->Field("Headbob Horizontal Sprint Scale", &FirstPersonExtrasComponent::m_headbobHorizontalSprintScale)
                ->Field("Headbob Rotation Sprint Scale", &FirstPersonExtrasComponent::m_headbobRotationSprintScale)
                ->Field("Headbob Vertical Crouch Scale", &FirstPersonExtrasComponent::m_headbobVerticalCrouchScale)
                ->Field("Headbob Horizontal Crouch Scale", &FirstPersonExtrasComponent::m_headbobHorizontalCrouchScale)
                ->Field("Headbob Rotation Crouch Scale", &FirstPersonExtrasComponent::m_headbobRotationCrouchScale)
                ->Version(2);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonExtrasComponent>(
                      "First Person Extras",
                      "The First Person Extras component provides you with extra features for your first person character controller")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Jumping")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_jumpPressedInAirQueueTimeThreshold,
                        "Jump Pressed In Air Queue Time Threshold",
                        "The duration prior to the character being grounded where pressing and releasing the jump key will be queued up "
                        "for a jump once the character becomes grounded; if the jump key is pressed and released outside of this timing "
                        "window then a jump will not be queued.")

                    // Jump Head Tilt group
                    ->GroupElementToggle("Jump Head Tilt", &FirstPersonExtrasComponent::m_jumpHeadTiltEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headAngleJump,
                        "Head Angle Jump",
                        "The change in the camera's pitch when the character jumps.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headAngleLand,
                        "Head Angle Land",
                        "The change in the camera's pitch when the character lands.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_deltaAngleFactorJump,
                        "Jump Delta Angle Speed Factor",
                        "Factor that determines how quickly the camera's pitch changes at the start of a jump, "
                        "bigger numbers make it faster.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_deltaAngleFactorLand,
                        "Land Delta Angle Speed Factor",
                        "Factor that determines how quickly the camera's pitch changes when landing, "
                        "bigger numbers make it faster.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_completeHeadLandVelocity,
                        "Complete Head Angle Land Velocity",
                        "The velocity required when hitting the ground for the entire Head Angle Land to apply, "
                        "any velocity less than this will cause the angle to be proportional to the velocity when the ground is hit "
                        "divided by this value.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)

                    // Sprint FoV group
                    ->GroupElementToggle("Sprint FoV", &FirstPersonExtrasComponent::m_sprintFoVEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFoVDelta,
                        "FoV Increase When Sprinting",
                        "The increase in the camera's field of view while sprinting.")
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFoVLerpTime,
                        "Sprint FoV Lerp Time",
                        "The time it takes to get to the maximum field of view when sprinting, and the time it takes to get back.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetSprintFoVEnabled)

                    ->GroupElementToggle("Headbob", &FirstPersonExtrasComponent::m_headbobEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobStartingDirection,
                        "Starting Direction",
                        "Determines the starting direction of the headbob figure-8, false starts swaying left while true starts swaying "
                        "right.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobOverallIntensity,
                        "Overall Intensity",
                        "Multiplies the size of the whole headbob without changing its speed (higher = more pronounced; 0 = no bob).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 2.f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.01f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxFrequency,
                        "Max Frequency",
                        "The maximum rate of the headbob figure-8, reached at top sprinting speed, "
                        "one cycle being a full left/right sway and two up/down bobs.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 5.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxVerticalAmplitude,
                        "Max Vertical Amplitude",
                        "Peak up/down camera travel at top sprinting speed (higher = deeper bounce; total travel is twice this).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxHorizontalAmplitude,
                        "Max Horizontal Amplitude",
                        "Peak left/right camera travel at top sprinting speed (higher = wider sway; total travel is twice this).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobSmoothTime,
                        "Smooth Time",
                        "Low-pass time constant for easing the bob in and out of movement (higher = softer, laggier; 0 = none).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 0.5f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.005f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobRealism,
                        "Realism",
                        "Blend for the individual realism fields below (higher = more like a handheld camera and roughly double the "
                        "apparent motion, so lower Overall Intensity; 0 = plain figure-8).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 1.f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.01f)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonExtrasComponent::OnHeadbobRealismChanged)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxPitchAmplitude,
                        "Max Pitch Amplitude",
                        "Peak up/down head nod at top sprinting speed, counter-rotating against the bounce (higher = more rotational "
                        "pitch).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 10.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxRollAmplitude,
                        "Max Roll Amplitude",
                        "Peak side-to-side head tilt at top sprinting speed, lagging the yaw (higher = more rotational roll).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 10.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobMaxYawAmplitude,
                        "Max Yaw Amplitude",
                        "Peak left/right head turn at top sprinting speed (higher = more rotational yaw).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 10.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobFootstepSharpness,
                        "Footstep Sharpness",
                        "Sharpens the drop as each foot plants and rounds the rise between (higher = snappier footfall; 0 = smooth sine "
                        "dip).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 0.25f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.005f)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonExtrasComponent::UpdateHeadbobShapePeaks)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobAlternatingStepDifference,
                        "Alternating Step Difference",
                        "Makes every other footfall dip deeper (higher = more of a limp; 0 = both feet dip the same).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 2.f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.01f)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonExtrasComponent::UpdateHeadbobShapePeaks)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobHorizontalSwayImbalance,
                        "Horizontal Sway Imbalance",
                        "Extra swing to one side of the sway (higher = one side reaches much further than the other; 0 = even left and "
                        "right).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 0.25f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.005f)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonExtrasComponent::UpdateHeadbobShapePeaks)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobHorizontalSwayFlatness,
                        "Horizontal Sway Flatness",
                        "Flattens the peaks of the sway (higher = the head holds longer at each side; 0 = smoothly rounded).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 1.f / 9.f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.002f)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FirstPersonExtrasComponent::UpdateHeadbobShapePeaks)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobFootstepAcceleration,
                        "Footstep Acceleration",
                        "Speeds the camera forward into each footstep, then eases back as the foot lifts (higher = a stronger surge per "
                        "step).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 2.f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.01f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Slider,
                        &FirstPersonExtrasComponent::m_headbobStepVariationOverTime,
                        "Step Variation Over Time",
                        "Slowly varies step size and speed over eight walk cycles (higher = more drift; 0 = every step exactly alike).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 0.25f)
                    ->Attribute(AZ::Edit::Attributes::Step, 0.005f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobVerticalSprintScale,
                        "Vertical Sprint Scale",
                        "Vertical bob at top sprinting speed relative to walking (higher = more bounce when running).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobHorizontalSprintScale,
                        "Horizontal Sprint Scale",
                        "Left/right sway at top sprinting speed relative to walking (lower = narrower sway when running).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobRotationSprintScale,
                        "Rotation Sprint Scale",
                        "Head pitch, roll and yaw at top sprinting speed relative to walking (higher = more head sway when running).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobVerticalCrouchScale,
                        "Vertical Crouch Scale",
                        "Vertical bob while fully crouched relative to walking upright (lower = more absorbed by the bent knees).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobHorizontalCrouchScale,
                        "Horizontal Crouch Scale",
                        "Left/right sway while fully crouched relative to walking upright (higher = wider from the wider stance).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobRotationCrouchScale,
                        "Rotation Crouch Scale",
                        "Head pitch, roll and yaw while fully crouched relative to walking upright "
                        "(higher = more head sway when crouched).")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Max, 20.f)
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero);
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonExtrasComponentNotificationBus>("FirstPersonExtrasComponentNotificationBus")
                ->Handler<FirstPersonExtrasComponentNotificationHandler>();

            bc->EBus<FirstPersonExtrasComponentRequestBus>("FirstPersonExtrasComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Extras")
                ->Event(
                    "Get Jump Pressed In Air Queue Time Threshold",
                    &FirstPersonExtrasComponentRequests::GetJumpPressedInAirQueueTimeThreshold)
                ->Event(
                    "Set Jump Pressed In Air Queue Time Threshold",
                    &FirstPersonExtrasComponentRequests::SetJumpPressedInAirQueueTimeThreshold)
                ->Event("Get Jump Head Tilt Enabled", &FirstPersonExtrasComponentRequests::GetJumpHeadTiltEnabled)
                ->Event("Set Jump Head Tilt Enabled", &FirstPersonExtrasComponentRequests::SetJumpHeadTiltEnabled)
                ->Event("Get Head Angle Jump", &FirstPersonExtrasComponentRequests::GetHeadAngleJump)
                ->Event("Set Head Angle Jump", &FirstPersonExtrasComponentRequests::SetHeadAngleJump)
                ->Event("Get Head Angle Land", &FirstPersonExtrasComponentRequests::GetHeadAngleLand)
                ->Event("Set Head Angle Land", &FirstPersonExtrasComponentRequests::SetHeadAngleLand)
                ->Event("Get Delta Angle Factor Jump", &FirstPersonExtrasComponentRequests::GetDeltaAngleFactorJump)
                ->Event("Set Delta Angle Factor Jump", &FirstPersonExtrasComponentRequests::SetDeltaAngleFactorJump)
                ->Event("Get Delta Angle Factor Land", &FirstPersonExtrasComponentRequests::GetDeltaAngleFactorLand)
                ->Event("Set Delta Angle Factor Land", &FirstPersonExtrasComponentRequests::SetDeltaAngleFactorLand)
                ->Event("Get Complete Head Land Velocity", &FirstPersonExtrasComponentRequests::GetCompleteHeadLandVelocity)
                ->Event("Set Complete Head Land Velocity", &FirstPersonExtrasComponentRequests::SetCompleteHeadLandVelocity)
                ->Event("Get Sprint FoV Enabled", &FirstPersonExtrasComponentRequests::GetSprintFoVEnabled)
                ->Event("Set Sprint FoV Enabled", &FirstPersonExtrasComponentRequests::SetSprintFoVEnabled)
                ->Event("Get Sprint FoV Lerp Time", &FirstPersonExtrasComponentRequests::GetSprintFoVLerpTime)
                ->Event("Set Sprint FoV Lerp Time", &FirstPersonExtrasComponentRequests::SetSprintFoVLerpTime)
                ->Event("Get Sprinting FoV", &FirstPersonExtrasComponentRequests::GetSprintingFoV)
                ->Event("Set Sprinting FoV", &FirstPersonExtrasComponentRequests::SetSprintingFoV)
                ->Event("Get Walking FoV", &FirstPersonExtrasComponentRequests::GetWalkingFoV)
                ->Event("Set Walking FoV", &FirstPersonExtrasComponentRequests::SetWalkingFoV)
                ->Event("Get Headbob Enabled", &FirstPersonExtrasComponentRequests::GetHeadbobEnabled)
                ->Event("Set Headbob Enabled", &FirstPersonExtrasComponentRequests::SetHeadbobEnabled)
                ->Event("Get Headbob Starting Direction", &FirstPersonExtrasComponentRequests::GetHeadbobStartingDirection)
                ->Event("Set Headbob Starting Direction", &FirstPersonExtrasComponentRequests::SetHeadbobStartingDirection)
                ->Event("Get Headbob Max Frequency", &FirstPersonExtrasComponentRequests::GetHeadbobMaxFrequency)
                ->Event("Set Headbob Max Frequency", &FirstPersonExtrasComponentRequests::SetHeadbobMaxFrequency)
                ->Event("Get Headbob Max Vertical Amplitude", &FirstPersonExtrasComponentRequests::GetHeadbobMaxVerticalAmplitude)
                ->Event("Set Headbob Max Vertical Amplitude", &FirstPersonExtrasComponentRequests::SetHeadbobMaxVerticalAmplitude)
                ->Event("Get Headbob Max Horizontal Amplitude", &FirstPersonExtrasComponentRequests::GetHeadbobMaxHorizontalAmplitude)
                ->Event("Set Headbob Max Horizontal Amplitude", &FirstPersonExtrasComponentRequests::SetHeadbobMaxHorizontalAmplitude)
                ->Event("Get Headbob Overall Intensity", &FirstPersonExtrasComponentRequests::GetHeadbobOverallIntensity)
                ->Event("Set Headbob Overall Intensity", &FirstPersonExtrasComponentRequests::SetHeadbobOverallIntensity)
                ->Event("Get Headbob Smooth Time", &FirstPersonExtrasComponentRequests::GetHeadbobSmoothTime)
                ->Event("Set Headbob Smooth Time", &FirstPersonExtrasComponentRequests::SetHeadbobSmoothTime)
                ->Event("Get Headbob Realism", &FirstPersonExtrasComponentRequests::GetHeadbobRealism)
                ->Event("Set Headbob Realism", &FirstPersonExtrasComponentRequests::SetHeadbobRealism)
                ->Event("Get Headbob Footstep Sharpness", &FirstPersonExtrasComponentRequests::GetHeadbobFootstepSharpness)
                ->Event("Set Headbob Footstep Sharpness", &FirstPersonExtrasComponentRequests::SetHeadbobFootstepSharpness)
                ->Event("Get Headbob Alternating Step Difference", &FirstPersonExtrasComponentRequests::GetHeadbobAlternatingStepDifference)
                ->Event("Set Headbob Alternating Step Difference", &FirstPersonExtrasComponentRequests::SetHeadbobAlternatingStepDifference)
                ->Event("Get Headbob Horizontal Sway Imbalance", &FirstPersonExtrasComponentRequests::GetHeadbobHorizontalSwayImbalance)
                ->Event("Set Headbob Horizontal Sway Imbalance", &FirstPersonExtrasComponentRequests::SetHeadbobHorizontalSwayImbalance)
                ->Event("Get Headbob Horizontal Sway Flatness", &FirstPersonExtrasComponentRequests::GetHeadbobHorizontalSwayFlatness)
                ->Event("Set Headbob Horizontal Sway Flatness", &FirstPersonExtrasComponentRequests::SetHeadbobHorizontalSwayFlatness)
                ->Event("Get Headbob Footstep Acceleration", &FirstPersonExtrasComponentRequests::GetHeadbobFootstepAcceleration)
                ->Event("Set Headbob Footstep Acceleration", &FirstPersonExtrasComponentRequests::SetHeadbobFootstepAcceleration)
                ->Event("Get Headbob Max Pitch Amplitude", &FirstPersonExtrasComponentRequests::GetHeadbobMaxPitchAmplitude)
                ->Event("Set Headbob Max Pitch Amplitude", &FirstPersonExtrasComponentRequests::SetHeadbobMaxPitchAmplitude)
                ->Event("Get Headbob Max Roll Amplitude", &FirstPersonExtrasComponentRequests::GetHeadbobMaxRollAmplitude)
                ->Event("Set Headbob Max Roll Amplitude", &FirstPersonExtrasComponentRequests::SetHeadbobMaxRollAmplitude)
                ->Event("Get Headbob Max Yaw Amplitude", &FirstPersonExtrasComponentRequests::GetHeadbobMaxYawAmplitude)
                ->Event("Set Headbob Max Yaw Amplitude", &FirstPersonExtrasComponentRequests::SetHeadbobMaxYawAmplitude)
                ->Event("Get Headbob Step Variation Over Time", &FirstPersonExtrasComponentRequests::GetHeadbobStepVariationOverTime)
                ->Event("Set Headbob Step Variation Over Time", &FirstPersonExtrasComponentRequests::SetHeadbobStepVariationOverTime)
                ->Event("Get Headbob Vertical Sprint Scale", &FirstPersonExtrasComponentRequests::GetHeadbobVerticalSprintScale)
                ->Event("Set Headbob Vertical Sprint Scale", &FirstPersonExtrasComponentRequests::SetHeadbobVerticalSprintScale)
                ->Event("Get Headbob Horizontal Sprint Scale", &FirstPersonExtrasComponentRequests::GetHeadbobHorizontalSprintScale)
                ->Event("Set Headbob Horizontal Sprint Scale", &FirstPersonExtrasComponentRequests::SetHeadbobHorizontalSprintScale)
                ->Event("Get Headbob Rotation Sprint Scale", &FirstPersonExtrasComponentRequests::GetHeadbobRotationSprintScale)
                ->Event("Set Headbob Rotation Sprint Scale", &FirstPersonExtrasComponentRequests::SetHeadbobRotationSprintScale)
                ->Event("Get Headbob Vertical Crouch Scale", &FirstPersonExtrasComponentRequests::GetHeadbobVerticalCrouchScale)
                ->Event("Set Headbob Vertical Crouch Scale", &FirstPersonExtrasComponentRequests::SetHeadbobVerticalCrouchScale)
                ->Event("Get Headbob Horizontal Crouch Scale", &FirstPersonExtrasComponentRequests::GetHeadbobHorizontalCrouchScale)
                ->Event("Set Headbob Horizontal Crouch Scale", &FirstPersonExtrasComponentRequests::SetHeadbobHorizontalCrouchScale)
                ->Event("Get Headbob Rotation Crouch Scale", &FirstPersonExtrasComponentRequests::GetHeadbobRotationCrouchScale)
                ->Event("Set Headbob Rotation Crouch Scale", &FirstPersonExtrasComponentRequests::SetHeadbobRotationCrouchScale)
                ->Event("Get Headbob Last Step Strength", &FirstPersonExtrasComponentRequests::GetHeadbobLastStepStrength)
                ->Event("Get Camera Translation Without Headbob", &FirstPersonExtrasComponentRequests::GetCameraTranslationWithoutHeadbob)
                ->Event("Get Previous Camera Headbob Offset", &FirstPersonExtrasComponentRequests::GetPreviousOffset);

            bc->Class<FirstPersonExtrasComponent>()->RequestBus("FirstPersonExtrasComponentRequestBus");
        }
    }

    void FirstPersonExtrasComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
#ifdef NETWORKFPC
        NetworkFPCControllerNotificationBus::Handler::BusConnect(GetEntityId());
#endif
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());

        // Get access to the FirstPersonControllerComponent and NetwworkFPC objects and their members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
#ifdef NETWORKFPC
        m_networkFPCObject = entity->FindComponent<NetworkFPC>();
#endif

        // Assign pointer attributes to the associated attributes of the FirstPersonControllerComponent, accessible via friendship
        if (m_firstPersonControllerObject)
        {
            m_jumpValue = &(m_firstPersonControllerObject->m_jumpValue);
            m_scriptJump = &(m_firstPersonControllerObject->m_scriptJump);
            m_grounded = &(m_firstPersonControllerObject->m_grounded);
        }

        // Convert Jump Head Tilt angles to radians
        m_headAngleJump = AZ::DegToRad(m_headAngleJump);
        m_headAngleLand = AZ::DegToRad(m_headAngleLand);

        // Compute the headbob waveform normalization from the serialized shaping values
        UpdateHeadbobShapePeaks();

        // Assign the FirstPersonExtrasComponent specific inputs
        AssignConnectInputEvents();

        // Headbob activation
        if (m_headbobEnabled)
        {
            // Setup Headbob entity
            if (!m_cameraEntityId.IsValid())
            {
                m_cameraEntityPtr = GetActiveCamera();
                if (m_cameraEntityPtr == nullptr)
                {
                    m_needsHeadbobFallback = true;
                    Camera::CameraNotificationBus::Handler::BusConnect();
                }
                else
                    m_cameraChildOfCharacter = m_cameraEntityPtr->GetTransform()->GetParentId() == GetEntityId();
            }
            else
            {
                AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
            }

            // Initialize original translation and offsets if pointer is set
            if (m_cameraEntityPtr)
            {
                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
                m_prevHeadbobOffset = AZ::Vector3::CreateZero();
                m_prevHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
                // Clear the smoothing state too, so a bob left over from a previous camera cannot bleed onto
                // this one over the first smoothing time constant
                m_smoothedHeadbobOffset = AZ::Vector3::CreateZero();
                m_smoothedHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
            }
        }
    }

    void FirstPersonExtrasComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        FirstPersonExtrasComponentRequestBus::Handler::BusDisconnect();
#ifdef NETWORKFPC
        NetworkFPCControllerNotificationBus::Handler::BusDisconnect();
#endif
        FirstPersonControllerComponentNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();

        // Headbob deactivation
        if (m_headbobEnabled)
        {
            if (m_needsHeadbobFallback)
                Camera::CameraNotificationBus::Handler::BusDisconnect();

            AZ::EntityBus::Handler::BusDisconnect();
        }
        m_cameraEntityPtr = nullptr;
    }

    void FirstPersonExtrasComponent::OnCameraAdded(const AZ::EntityId& cameraId)
    {
        if (!m_cameraEntityId.IsValid())
        {
            m_cameraEntityId = cameraId;
            m_cameraEntityPtr = GetEntityPtr(cameraId);

            if (m_cameraEntityPtr)
            {
                m_cameraChildOfCharacter = m_cameraEntityPtr->GetTransform()->GetParentId() == GetEntityId();
                Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);
                // AZ_Printf("First Person Extras Component", "Default camera %s assigned and set as active view.",
                //     m_activeCameraEntity->GetName().empty() ? m_cameraEntityId.ToString().c_str() :
                //     m_activeCameraEntity->GetName().c_str());
            }
            else
            {
                AZ_Warning(
                    "FirstPersonExtrasComponent",
                    false,
                    "Default camera ID %s from CameraNotificationBus is invalid.",
                    cameraId.ToString().c_str());
                m_cameraEntityId = AZ::EntityId();
            }
        }
        // Whenever a camera is added, use its FoV as the walking FoV value
        Camera::CameraRequestBus::EventResult(m_walkFoV, m_cameraEntityId, &Camera::CameraComponentRequests::GetFovDegrees);
        m_sprintFoV = m_walkFoV + m_sprintFoVDelta;
    }

    void FirstPersonExtrasComponent::OnActiveViewChanged(const AZ::EntityId& activeEntityId)
    {
        if (m_needsHeadbobFallback)
        {
            m_cameraEntityPtr = GetEntityPtr(activeEntityId);
            if (m_cameraEntityPtr != nullptr)
            {
                m_cameraEntityId = activeEntityId;
                Camera::CameraNotificationBus::Handler::BusDisconnect();
                m_needsHeadbobFallback = false;

                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
                m_prevHeadbobOffset = AZ::Vector3::CreateZero();
                m_prevHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
                // Clear the smoothing state too, so a bob left over from a previous camera cannot bleed onto
                // this one over the first smoothing time constant
                m_smoothedHeadbobOffset = AZ::Vector3::CreateZero();
                m_smoothedHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
            }
        }
    }

    void FirstPersonExtrasComponent::OnEntityActivated(const AZ::EntityId& entityId)
    {
        // Get access to the NetworkFPC object and its member
#ifdef NETWORKFPC
        const AZ::Entity* entity = GetEntity();
        m_networkFPCObject = entity->FindComponent<NetworkFPC>();
#endif

        // Determine if the NetworkFPC is enabled
        if (m_networkFPCObject != nullptr)
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
#ifdef NETWORKFPC
            m_networkFPCEnabled = static_cast<NetworkFPCController*>(m_networkFPCObject->GetController())->GetEnableNetworkFPC();
#endif
        }

        if (entityId == m_cameraEntityId)
        {
            m_cameraEntityPtr = GetEntityPtr(m_cameraEntityId);
            AZ::EntityBus::Handler::BusDisconnect();
            if (m_cameraEntityPtr)
            {
                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
                m_prevHeadbobOffset = AZ::Vector3::CreateZero();
                m_prevHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
                // Clear the smoothing state too, so a bob left over from a previous camera cannot bleed onto
                // this one over the first smoothing time constant
                m_smoothedHeadbobOffset = AZ::Vector3::CreateZero();
                m_smoothedHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
            }
        }
    }

    AZ::Entity* FirstPersonExtrasComponent::GetActiveCamera() const
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void FirstPersonExtrasComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void FirstPersonExtrasComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("InputConfigurationService"));
    }

    void FirstPersonExtrasComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
    }

    void FirstPersonExtrasComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void FirstPersonExtrasComponent::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if (m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("First Person Extras Component", false, "Number of input IDs not equal to number of input names!");
        }
        else
        {
            for (auto& it_event : m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
    }

    void FirstPersonExtrasComponent::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonExtrasComponent::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Released", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonExtrasComponent::OnHeld([[maybe_unused]] float value)
    {
    }

    int FirstPersonExtrasComponent::GetTickOrder()
    {
        return AZ::TICK_PRE_RENDER + 1;
    }

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime, 0);
    }

    void FirstPersonExtrasComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if (!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
            !m_firstPersonControllerObject->m_isHost)
        {
            NotAutonomousSoDisconnect();
            FirstPersonExtrasComponentRequestBus::Handler::BusDisconnect(GetEntityId());
            return;
        }
        if (entityId != GetEntityId())
            return;
        if (!((m_firstPersonControllerObject->m_isHost && server) || (m_firstPersonControllerObject->m_isServer && !server)))
        {
            ProcessInput(deltaTime, 2);
        }
    }

    void FirstPersonExtrasComponent::OnNetworkTickFinish(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const bool server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnAutonomousClientActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnHostActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnNonAutonomousClientActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepStart(
        [[maybe_unused]] const float physicsTimestep, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish(const float physicsTimestep, [[maybe_unused]] const AZ::EntityId& entityId)
    {
        if (m_networkFPCEnabled && !m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
            !m_firstPersonControllerObject->m_isHost)
            return;
        ProcessInput((physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor), 1);
    }

    AZ::Entity* FirstPersonExtrasComponent::GetEntityPtr(AZ::EntityId pointer) const
    {
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(pointer);
    }

    void FirstPersonExtrasComponent::QueueJump(const float deltaTime, const AZ::u8 tickTimestepNetwork)
    {
        // Bail if the threshold is set to zero
        if (m_jumpPressedInAirQueueTimeThreshold == 0.f)
            return;

        if (tickTimestepNetwork == 0 && !*m_grounded && m_prevJumpValue == 0.f && *m_jumpValue != 0.f)
        {
            // Reset the timer
            m_jumpPressedInAirTimer = 0.f;

            // Queue up the jump when it's pressed while in the air
            m_prevQueueJump = m_queueJump;
            m_queueJump = true;
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_queueJump &&
            !*m_grounded)
        {
            // Increment the timer when the jump is queued
            m_jumpPressedInAirTimer += deltaTime;

            // Check if the timer exceeds the threshold before hitting the ground and if it does then don't perform the jump
            if (m_jumpPressedInAirTimer >= m_jumpPressedInAirQueueTimeThreshold)
            {
                m_jumpPressedInAirTimer = 0.f;
                m_prevQueueJump = false;
                m_queueJump = false;
            }
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_queueJump &&
            *m_grounded)
        {
            // Perform the jump when the ground is hit
            m_prevQueueJump = m_queueJump;
            m_queueJump = false;
            *m_scriptJump = true;

            // Broadcast a notification event that a jump is queued
            FirstPersonExtrasComponentNotificationBus::Broadcast(&FirstPersonExtrasComponentNotificationBus::Events::OnJumpFromQueue);
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_prevQueueJump &&
            !m_queueJump)
        {
            // Clear the previous queue and script jump variables
            m_prevQueueJump = m_queueJump;
            *m_scriptJump = false;
        }

        // Keep track of the previous jump value
        if (tickTimestepNetwork == 0)
            m_prevJumpValue = *m_jumpValue;
    }

    void FirstPersonExtrasComponent::PerformSprintFoV(const float deltaTime)
    {
        if (!m_sprintFoVEnabled)
            return;

        const float currentSpeed = m_firstPersonControllerObject->m_movingUpInclineSlowed
            ? m_firstPersonControllerObject->m_applyVelocityXY.GetLength() * m_firstPersonControllerObject->m_movingUpInclineFactor
            : m_firstPersonControllerObject->m_applyVelocityXY.GetLength();
        const float sprintScaleForward = m_firstPersonControllerObject->m_sprintScaleForward;
        const float forwardScale = m_firstPersonControllerObject->m_forwardScale;
        const float walkSpeed = m_firstPersonControllerObject->m_speed;

        const bool notRecentlyGrounded = AZStd::all_of(
            m_firstPersonControllerObject->m_prevNTicksGrounded.begin(),
            m_firstPersonControllerObject->m_prevNTicksGrounded.end(),
            [](bool gnd)
            {
                return gnd == false;
            });
        const bool groundedRecently = !notRecentlyGrounded;

        // Scale the FoV based on the current speed, assuming forward is the fastest direction
        if (m_firstPersonControllerObject != nullptr &&
            (m_firstPersonControllerObject->m_sprintInAir || m_firstPersonControllerObject->m_coyoteTimeNoGravityActive ||
             groundedRecently) &&
            GetSprinting() &&
            (currentSpeed - walkSpeed) / (sprintScaleForward * forwardScale * walkSpeed - walkSpeed) >=
                m_sprintFoVTimeAccumulator / m_sprintFoVLerpTime)
        {
            m_sprintFoVTimeAccumulator += deltaTime;
            if ((currentSpeed - walkSpeed) / (sprintScaleForward * forwardScale * walkSpeed - walkSpeed) <
                m_sprintFoVTimeAccumulator / m_sprintFoVLerpTime)
                m_sprintFoVTimeAccumulator =
                    (currentSpeed - walkSpeed) / (sprintScaleForward * forwardScale * walkSpeed - walkSpeed) * m_sprintFoVLerpTime;
        }
        else
        {
            m_sprintFoVTimeAccumulator -= deltaTime;
            if (m_sprintFoVTimeAccumulator < 0.f)
                m_sprintFoVTimeAccumulator = 0.f;
        }
        // Lerp the FoV and apply it
        const float newCameraFoV = AZ::Lerp(m_walkFoV, m_sprintFoV, m_sprintFoVTimeAccumulator / m_sprintFoVLerpTime);
        Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraComponentRequests::SetFovDegrees, newCameraFoV);
    }

    bool FirstPersonExtrasComponent::GetSprinting()
    {
        float currentSpeed = m_firstPersonControllerObject->m_applyVelocityXY.GetLength();

        // Check to see if sprinting is obstructed for several ticks in a row
        m_sprintingObstructedCheck[m_sprintingObstructedIndex] =
            m_firstPersonControllerObject->m_correctedVelocityXY.IsZero(m_firstPersonControllerObject->m_speed / 2.f);
        m_sprintingObstructedIndex++;
        if (m_sprintingObstructedIndex == AZStd::size(m_sprintingObstructedCheck))
            m_sprintingObstructedIndex = 0;
        const bool sprintingObstructed = AZStd::all_of(
            AZStd::begin(m_sprintingObstructedCheck),
            AZStd::end(m_sprintingObstructedCheck),
            [](bool sprintingObstructedCheckElement)
            {
                return sprintingObstructedCheckElement;
            });

        if (AZ::IsClose(currentSpeed, 0.f) || sprintingObstructed)
            return false;

        float topWalkSpeedInDirection = m_firstPersonControllerObject->m_speed *
            m_firstPersonControllerObject
                ->CreateEllipseScaledVector(
                    m_firstPersonControllerObject->m_applyVelocityXY.GetNormalized(),
                    m_firstPersonControllerObject->m_forwardScale,
                    m_firstPersonControllerObject->m_backScale,
                    m_firstPersonControllerObject->m_leftScale,
                    m_firstPersonControllerObject->m_rightScale)
                .GetLength();
        if (m_firstPersonControllerObject->m_movingUpInclineSlowed)
        {
            currentSpeed *= m_firstPersonControllerObject->m_movingUpInclineFactor;
            topWalkSpeedInDirection *= m_firstPersonControllerObject->m_movingUpInclineFactor;
        }

        if (m_firstPersonControllerObject->m_sprintVelocityAdjust != 1.f &&
            (m_firstPersonControllerObject->m_standing || m_firstPersonControllerObject->m_sprintWhileCrouched) &&
            currentSpeed > topWalkSpeedInDirection)
            return true;
        else
            return false;
    }

    void FirstPersonExtrasComponent::PerformJumpHeadTilt(const float deltaTime)
    {
        if (!m_jumpHeadTiltEnabled)
            return;

        if (m_tiltJumped)
            m_deltaAngle = deltaTime * m_totalHeadAngle * -m_deltaAngleFactorJump;
        else if (m_tiltLanded)
            m_deltaAngle = deltaTime * m_totalHeadAngle * -m_deltaAngleFactorLand;
        else
            return;

        if (m_moveHeadDown)
        {
            m_currentHeadPitchAngle -= m_deltaAngle;
            m_firstPersonControllerObject->UpdateCameraPitch(-m_deltaAngle, true);
        }
        else
        {
            m_currentHeadPitchAngle += m_deltaAngle;
            m_firstPersonControllerObject->UpdateCameraPitch(m_deltaAngle, true);
        }

        if (m_currentHeadPitchAngle >= 0.f)
        {
            m_moveHeadDown = true;
            m_tiltJumped = false;
            m_tiltLanded = false;
            m_firstPersonControllerObject->UpdateCameraPitch(-m_currentHeadPitchAngle, true);
            m_currentHeadPitchAngle = 0.f;
        }
        else if (m_currentHeadPitchAngle <= m_totalHeadAngle)
        {
            m_moveHeadDown = false;
            m_firstPersonControllerObject->UpdateCameraPitch(m_totalHeadAngle - m_currentHeadPitchAngle, true);
            m_currentHeadPitchAngle = m_totalHeadAngle;
        }
    }

    // Sum the harmonics of the walk cycle phase, scaled by the realism factor. The bounce is the
    // second harmonic. The first harmonic is offset 45 degrees so its peak lands on a dip, making
    // one footfall deeper without changing footstep timing. The fourth harmonic runs at four times
    // the phase as a cosine, so it sharpens each dip and rounds the rise between them
    float FirstPersonExtrasComponent::CalculateHeadbobVerticalShape(const float phase) const
    {
        return -sinf(2.f * phase) +
            m_headbobRealism *
            (m_headbobAlternatingStepDifference * sinf(phase + AZ::DegToRad(45.f)) + m_headbobFootstepSharpness * cosf(4.f * phase));
    }
    // Shape the horizontal sway with two harmonics. The even one is offset -94 degrees to peak on the
    // sway extremes, so it deepens one side without moving when the sway reaches it, and only the odd
    // one can flatten the tops
    float FirstPersonExtrasComponent::CalculateHeadbobHorizontalShape(const float phase) const
    {
        return sinf(phase) +
            m_headbobRealism *
            (m_headbobHorizontalSwayImbalance * sinf(2.f * phase + AZ::DegToRad(-94.f)) +
             m_headbobHorizontalSwayFlatness * sinf(3.f * phase));
    }
    // Build the forward and back motion as a Fourier series of a VR headset capture of walking data,
    // keeping only the first three harmonics. The first is 122 degrees into the walk cycle, the
    // second is 0.215 of its magnitude at 80 degrees, and the third is 0.093 at an angle which
    // measured within a degree of zero, so it has no phase offset
    float FirstPersonExtrasComponent::CalculateHeadbobForwardShape(const float phase) const
    {
        return sinf(phase + AZ::DegToRad(122.f)) + 0.215f * sinf(2.f * phase + AZ::DegToRad(80.f)) + 0.093f * sinf(3.f * phase);
    }

    void FirstPersonExtrasComponent::UpdateHeadbobShapePeaks()
    {
        // Sample one walk cycle to find each shape's peak, since there is no closed form for the peak
        // of a harmonic sum
        constexpr AZ::u32 Samples = 256;
        float verticalPeak = 0.f;
        float horizontalPeak = 0.f;
        float forwardPeak = 0.f;

        for (AZ::u32 i = 0; i < Samples; ++i)
        {
            const float phase = AZ::Constants::TwoPi * static_cast<float>(i) / static_cast<float>(Samples);

            const float verticalShape = CalculateHeadbobVerticalShape(phase);
            verticalPeak = AZ::GetMax(verticalPeak, AZ::GetMax(verticalShape, -verticalShape));

            const float horizontalShape = CalculateHeadbobHorizontalShape(phase);
            horizontalPeak = AZ::GetMax(horizontalPeak, AZ::GetMax(horizontalShape, -horizontalShape));

            const float forwardShape = CalculateHeadbobForwardShape(phase);
            forwardPeak = AZ::GetMax(forwardPeak, AZ::GetMax(forwardShape, -forwardShape));
        }

        // Guard against a shape that is flat zero, which would otherwise divide by zero
        m_headbobVerticalShapePeak = AZ::IsClose(verticalPeak, 0.f) ? 1.f : verticalPeak;
        m_headbobHorizontalShapePeak = AZ::IsClose(horizontalPeak, 0.f) ? 1.f : horizontalPeak;
        m_headbobForwardShapePeak = AZ::IsClose(forwardPeak, 0.f) ? 1.f : forwardPeak;
    }

    // Recompute the shape peaks and refresh the editor
    AZ::u32 FirstPersonExtrasComponent::OnHeadbobRealismChanged()
    {
        UpdateHeadbobShapePeaks();
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    // Every shaping field below Realism is multiplied by it, so hide them all when it's zero
    bool FirstPersonExtrasComponent::GetHeadbobEnabledAndRealismGreaterThanZero() const
    {
        return m_headbobEnabled && m_headbobRealism > 0.f;
    }

    AZ::Vector3 FirstPersonExtrasComponent::CalculateHeadbobOffset(const float deltaTime)
    {
        // Walking if FirstPersonController XYs velocity non-zero and grounded
        m_isWalking = !m_firstPersonControllerObject->m_correctedVelocityXY.IsZero(0.1f) &&
            (m_firstPersonControllerObject->m_groundClose || m_firstPersonControllerObject->m_coyoteTimeNoGravityActive);

        // Get the speed values
        const float currentSpeed = m_firstPersonControllerObject->m_movingUpInclineSlowed
            ? m_firstPersonControllerObject->m_correctedVelocityXY.GetLength() * m_firstPersonControllerObject->m_movingUpInclineFactor
            : m_firstPersonControllerObject->m_correctedVelocityXY.GetLength();
        const float walkSpeed = m_firstPersonControllerObject->m_speed;
        const float sprintScaleForward = m_firstPersonControllerObject->m_sprintScaleForward;

        // Scale by the ratio of the current speed to the top sprint speed so accel and decel stay continuous
        const float topSprintSpeed = walkSpeed * sprintScaleForward;
        // Zero when not walking, since the waveforms are not zero at the phase reset and the speed
        // stays high in the air
        const float currentSpeedToTopSprintSpeedRatio =
            (m_isWalking && topSprintSpeed > 0.f) ? AZStd::min(currentSpeed / topSprintSpeed, 1.f) : 0.f;
        const float effectiveRadialFrequency = AZ::Constants::TwoPi * m_headbobMaxFrequency * currentSpeedToTopSprintSpeedRatio;
        // Replace the uniform speed scaling with the per-axis sprint scales, reaching the maximums at the
        // top sprint speed so neither end depends on how Sprint Forward Scale is configured
        float horizontalSpeedScale = currentSpeedToTopSprintSpeedRatio;
        float verticalSpeedScale = currentSpeedToTopSprintSpeedRatio;
        float rotationSpeedScale = currentSpeedToTopSprintSpeedRatio;
        if (sprintScaleForward > 1.f)
        {
            const float sprintBlend =
                AZ::GetClamp((currentSpeedToTopSprintSpeedRatio * sprintScaleForward - 1.f) / (sprintScaleForward - 1.f), 0.f, 1.f);
            // Ramp to the walking value and then to one, so the scale never overshoots either end
            const float walkFraction = AZStd::min(currentSpeedToTopSprintSpeedRatio * sprintScaleForward, 1.f);
            const float horizontalTarget = AZ::Lerp(walkFraction / AZ::GetMax(m_headbobHorizontalSprintScale, 0.01f), 1.f, sprintBlend);
            const float verticalTarget = AZ::Lerp(walkFraction / AZ::GetMax(m_headbobVerticalSprintScale, 0.01f), 1.f, sprintBlend);
            const float rotationTarget = AZ::Lerp(walkFraction / AZ::GetMax(m_headbobRotationSprintScale, 0.01f), 1.f, sprintBlend);
            horizontalSpeedScale = AZ::Lerp(currentSpeedToTopSprintSpeedRatio, horizontalTarget, m_headbobRealism);
            verticalSpeedScale = AZ::Lerp(currentSpeedToTopSprintSpeedRatio, verticalTarget, m_headbobRealism);
            rotationSpeedScale = AZ::Lerp(currentSpeedToTopSprintSpeedRatio, rotationTarget, m_headbobRealism);
        }

        float effectiveHorizontalAmplitude = m_headbobMaxHorizontalAmplitude * horizontalSpeedScale;
        float effectiveVerticalAmplitude = m_headbobMaxVerticalAmplitude * verticalSpeedScale;

        // Blend the crouch scales on the camera's crouch travel rather than the speed, so slow walking
        // is not mistaken for crouching
        const float crouchDistance = m_firstPersonControllerObject->m_crouchDistance;
        const float crouchScale = m_firstPersonControllerObject->m_crouchScale;
        if (crouchDistance > 0.f && crouchScale > 0.f)
        {
            const float crouchBlend =
                AZ::GetClamp(-1.f * m_firstPersonControllerObject->m_cameraLocalZTravelDistance / crouchDistance, 0.f, 1.f);
            effectiveHorizontalAmplitude *= AZ::Lerp(1.f, m_headbobHorizontalCrouchScale / crouchScale, m_headbobRealism * crouchBlend);
            effectiveVerticalAmplitude *= AZ::Lerp(1.f, m_headbobVerticalCrouchScale / crouchScale, m_headbobRealism * crouchBlend);
            rotationSpeedScale *= AZ::Lerp(1.f, m_headbobRotationCrouchScale / crouchScale, m_headbobRealism * crouchBlend);
        }

        // Vary the speed and size slightly over this many walk cycles so no two steps are exactly alike
        const float stepVariationCycles = 8.f;
        const float stepWander = 1.f + m_headbobRealism * m_headbobStepVariationOverTime * sinf(m_headbobPhase / stepVariationCycles);
        effectiveHorizontalAmplitude *= stepWander;
        effectiveVerticalAmplitude *= stepWander;

        // Apply the overall intensity last so it scales the amplitudes and the shaping together. The head
        // rotation below uses the same value
        effectiveHorizontalAmplitude *= m_headbobOverallIntensity;
        effectiveVerticalAmplitude *= m_headbobOverallIntensity;

        // Advance the phase only when walking, otherwise reset it. Accumulating the phase keeps the
        // waveforms continuous when the frequency changes
        if (m_isWalking)
            m_headbobPhase += deltaTime * effectiveRadialFrequency * stepWander;
        else
            m_headbobPhase = 0.f;
        // Wrap on a whole number of variation cycles, which both the waveforms and the variation
        // repeat on, so nothing changes value as it wraps
        m_headbobPhase = fmodf(m_headbobPhase, stepVariationCycles * AZ::Constants::TwoPi);

        // Compute the offsets using a Lemniscate of Gerono (figure-8 pattern for natural sway and
        // bounce), shaped by the measured harmonics when Realism is non-zero
        float horizontalOffset =
            m_headbobStartingDirection ? CalculateHeadbobHorizontalShape(m_headbobPhase) : -CalculateHeadbobHorizontalShape(m_headbobPhase);
        float verticalOffset = CalculateHeadbobVerticalShape(m_headbobPhase);

        // Normalize the vertical waveform for the pitch below, which is an angle and so cannot be derived
        // from the metres the vertical bob travels, and for the step notifications
        m_headbobNormalizedVerticalShape = verticalOffset / m_headbobVerticalShapePeak;

        // Divide by each shape's peak so the added harmonics do not increase the travel beyond what the
        // amplitude fields ask for
        horizontalOffset *= effectiveHorizontalAmplitude / m_headbobHorizontalShapePeak;
        verticalOffset *= effectiveVerticalAmplitude / m_headbobVerticalShapePeak;

        // Speed the camera into each footstep, which the figure-8 has no term for, scaled off the
        // horizontal amplitude so it stays in proportion with the sway
        float forwardOffset = 0.f;
        if (m_headbobFootstepAcceleration != 0.f && m_headbobRealism != 0.f)
            forwardOffset = m_headbobRealism * m_headbobFootstepAcceleration * effectiveHorizontalAmplitude *
                CalculateHeadbobForwardShape(m_headbobPhase) / m_headbobForwardShapePeak;

        // Compute the head rotation riding on top of the bob, where the pitch counter-rotates against the
        // vertical bob and the roll lags the yaw by a fixed 30 degrees. All three take the rotation scale
        const float directionSign = m_headbobStartingDirection ? 1.f : -1.f;
        const float pitchOffset = -m_headbobRealism * m_headbobOverallIntensity * AZ::DegToRad(m_headbobMaxPitchAmplitude) *
            rotationSpeedScale * m_headbobNormalizedVerticalShape;
        const float rollOffset = directionSign * m_headbobRealism * m_headbobOverallIntensity * AZ::DegToRad(m_headbobMaxRollAmplitude) *
            rotationSpeedScale * sinf(m_headbobPhase + AZ::DegToRad(-30.f));
        const float yawOffset = directionSign * m_headbobRealism * m_headbobOverallIntensity * AZ::DegToRad(m_headbobMaxYawAmplitude) *
            rotationSpeedScale * sinf(m_headbobPhase);
        // Pitch about X (right), roll about Y (forward), yaw about Z (up)
        m_headbobRotationOffset = AZ::Quaternion::CreateRotationX(pitchOffset) * AZ::Quaternion::CreateRotationY(rollOffset) *
            AZ::Quaternion::CreateRotationZ(yawOffset);

        // Create a vector from the offets, horizontal along X, forward along Y, vertical along Z
        const AZ::Vector3 offsetVector = AZ::Vector3(horizontalOffset, forwardOffset, verticalOffset);
        if (m_cameraChildOfCharacter)
            return offsetVector;
        else
            return AZ::Quaternion::CreateRotationZ(m_firstPersonControllerObject->m_currentHeading).TransformVector(offsetVector);
    }

    void FirstPersonExtrasComponent::UpdateHeadbob(const float deltaTime)
    {
        if (!m_headbobEnabled || m_cameraEntityPtr == nullptr)
            return;

        // The bob is a local view effect, so apply no offset on the server or on a network bot
        // Zero them so anything already applied is taken back off
        if (!(m_firstPersonControllerObject->m_networkFPCEnabled &&
              (m_firstPersonControllerObject->m_isServer || m_firstPersonControllerObject->m_isNetBot)))
        {
            // Compute new headbob offset
            m_headbobOffset = CalculateHeadbobOffset(deltaTime);
        }
        else
        {
            m_headbobOffset = AZ::Vector3::CreateZero();
            m_headbobRotationOffset = AZ::Quaternion::CreateIdentity();
            m_headbobNormalizedVerticalShape = 0.f;
            m_isWalking = false;
        }

        // Low-pass the bob so it eases in and out of movement, with the exponential form keeping the
        // smoothing identical at any frame rate
        if (m_headbobSmoothTime > 0.f)
        {
            const float smoothFactor = 1.f - expf(-deltaTime / m_headbobSmoothTime);
            m_smoothedHeadbobOffset += (m_headbobOffset - m_smoothedHeadbobOffset) * smoothFactor;
            m_smoothedHeadbobRotationOffset = m_smoothedHeadbobRotationOffset.NLerp(m_headbobRotationOffset, smoothFactor);
            m_headbobSmoothedVerticalShape += (m_headbobNormalizedVerticalShape - m_headbobSmoothedVerticalShape) * smoothFactor;
        }
        else
        {
            m_smoothedHeadbobOffset = m_headbobOffset;
            m_smoothedHeadbobRotationOffset = m_headbobRotationOffset;
            m_headbobSmoothedVerticalShape = m_headbobNormalizedVerticalShape;
        }

        auto* headbobEntityTransform = m_cameraEntityPtr->GetTransform();
        const AZ::Vector3 currentCameraTranslation = headbobEntityTransform->GetLocalTM().GetTranslation();
        const AZ::Quaternion currentLocalRotation = headbobEntityTransform->GetLocalTM().GetRotation();

        // Get whether the First Person Controller replaced the camera's translation, or just its local Z
        // in CrouchManager, and whether it replaced the rotation
        const bool cameraTranslationOverwritten = m_firstPersonControllerObject->m_cameraTranslationOverwritten;
        const bool cameraLocalZOverwritten = cameraTranslationOverwritten || m_firstPersonControllerObject->m_cameraLocalZOverwritten;
        const bool cameraRotationOverwritten = m_firstPersonControllerObject->m_cameraRotationOverwritten;
        m_firstPersonControllerObject->m_cameraTranslationOverwritten = false;
        m_firstPersonControllerObject->m_cameraLocalZOverwritten = false;
        m_firstPersonControllerObject->m_cameraRotationOverwritten = false;

        // Get the "clean" local translation by removing the previous bob offset from the axes which still contain it
        m_cameraTranslationWithoutHeadbob = currentCameraTranslation;
        if (!cameraTranslationOverwritten)
        {
            m_cameraTranslationWithoutHeadbob.SetX(currentCameraTranslation.GetX() - m_prevHeadbobOffset.GetX());
            m_cameraTranslationWithoutHeadbob.SetY(currentCameraTranslation.GetY() - m_prevHeadbobOffset.GetY());
        }
        if (!cameraLocalZOverwritten)
            m_cameraTranslationWithoutHeadbob.SetZ(currentCameraTranslation.GetZ() - m_prevHeadbobOffset.GetZ());
        // Compute the target local translation by adding the new bob offset to the clean position
        const AZ::Vector3 targetLocalTranslation = m_cameraTranslationWithoutHeadbob + m_smoothedHeadbobOffset;

        // Set local translation
        headbobEntityTransform->SetLocalTranslation(targetLocalTranslation);
        // Store the applied offset so it can be removed on the next update
        m_prevHeadbobOffset = m_smoothedHeadbobOffset;

        // Recover the clean local rotation the same way. The pitch-delta branch leaves the previous
        // sway in place and does not set the flag
        const AZ::Quaternion cleanLocalRotation = cameraRotationOverwritten
            ? currentLocalRotation
            : (currentLocalRotation * m_prevHeadbobRotationOffset.GetInverseFull()).GetNormalized();
        headbobEntityTransform->SetLocalRotationQuaternion((cleanLocalRotation * m_smoothedHeadbobRotationOffset).GetNormalized());
        // Store the applied rotation so it can be removed on the next update
        m_prevHeadbobRotationOffset = m_smoothedHeadbobRotationOffset;

        // Broadcast a notification everytime a "step" is taken from the figure-8 headbobbing pattern,
        // detected on the smoothed waveform. The m_isWalking gate stops the decay from counting as a step
        if (m_isWalking && !m_stepTaken && m_headbobSmoothedVerticalShape > m_prevHeadbobSmoothedVerticalShape)
        {
            m_stepTaken = true;
            // Record how far the head dropped on this footfall, which the realism shaping makes uneven
            // between the two footfalls, before the broadcast so a listener can read it back
            m_headbobLastStepStrength = AZ::GetMax(-m_prevHeadbobSmoothedVerticalShape, 0.f);
            FirstPersonExtrasComponentNotificationBus::Broadcast(&FirstPersonExtrasComponentNotificationBus::Events::OnHeadbobStepTaken);
        }
        // Broadcast a notification everytime the zero is crossed
        else if (m_isWalking && m_prevHeadbobSmoothedVerticalShape >= 0.f && m_headbobSmoothedVerticalShape < 0.f)
            FirstPersonExtrasComponentNotificationBus::Broadcast(&FirstPersonExtrasComponentNotificationBus::Events::OnHeadbobOriginCross);
        else if (!m_isWalking || m_headbobSmoothedVerticalShape < m_prevHeadbobSmoothedVerticalShape)
            m_stepTaken = false;

        // Store the previous smoothed vertical shape for the comparisons above
        m_prevHeadbobSmoothedVerticalShape = m_headbobSmoothedVerticalShape;
    }

    // Frame tick == 0, physics fixed timestep == 1, network tick == 2
    void FirstPersonExtrasComponent::ProcessInput(const float deltaTime, const AZ::u8 tickTimestepNetwork)
    {
        // Queue up jumps
        QueueJump(deltaTime, tickTimestepNetwork);

        PerformSprintFoV(deltaTime);

        if (tickTimestepNetwork == 0)
        {
            if (!m_networkFPCEnabled)
            {
                // Perform Jump Head Tilt
                PerformJumpHeadTilt(deltaTime);
            }
            // Update Headbob
            UpdateHeadbob(deltaTime);
        }
        else if (tickTimestepNetwork == 2)
        {
            // Perform Jump Head Tilt
            PerformJumpHeadTilt(deltaTime);
        }
    }

    // Notification Events from the First Person Controller component
    void FirstPersonExtrasComponent::OnNetworkFPCTickStart(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnNetworkFPCTickFinish(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnNetworkFPCAutonomousClientActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnNetworkFPCHostActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnNetworkFPCNonAutonomousClientActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnFPCActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void FirstPersonExtrasComponent::OnGroundHit([[maybe_unused]] const float fellVelocity)
    {
    }
    void FirstPersonExtrasComponent::OnGroundSoonHit([[maybe_unused]] const float soonFellVelocity)
    {
        if (m_networkFPCEnabled &&
            ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
              !m_firstPersonControllerObject->m_isHost)))
            return;
        if (soonFellVelocity > 0.f)
            return;
        m_tiltLanded = true;
        m_tiltJumped = false;
        m_moveHeadDown = true;
        if (-soonFellVelocity < m_completeHeadLandVelocity)
            m_totalHeadAngle = -m_headAngleLand * -soonFellVelocity / m_completeHeadLandVelocity;
        else
            m_totalHeadAngle = -m_headAngleLand;
    }
    void FirstPersonExtrasComponent::OnUngrounded()
    {
    }
    void FirstPersonExtrasComponent::OnStartedFalling()
    {
    }
    void FirstPersonExtrasComponent::OnJumpApogeeReached()
    {
    }
    void FirstPersonExtrasComponent::OnStartedMoving()
    {
    }
    void FirstPersonExtrasComponent::OnTargetVelocityReached()
    {
    }
    void FirstPersonExtrasComponent::OnStopped()
    {
    }
    void FirstPersonExtrasComponent::OnTopWalkSpeedReached()
    {
    }
    void FirstPersonExtrasComponent::OnTopSprintSpeedReached()
    {
    }
    void FirstPersonExtrasComponent::OnHeadHit()
    {
    }
    void FirstPersonExtrasComponent::OnCharacterShapecastHitSomething(
        [[maybe_unused]] const AZStd::vector<AzPhysics::SceneQueryHit> characterHits)
    {
    }
    void FirstPersonExtrasComponent::OnVelocityXYObstructed()
    {
    }
    void FirstPersonExtrasComponent::OnCharacterGravityObstructed()
    {
    }
    void FirstPersonExtrasComponent::OnCrouched()
    {
    }
    void FirstPersonExtrasComponent::OnStoodUp()
    {
    }
    void FirstPersonExtrasComponent::OnStoodUpFromJump()
    {
    }
    void FirstPersonExtrasComponent::OnStandPrevented()
    {
    }
    void FirstPersonExtrasComponent::OnStartedCrouching()
    {
    }
    void FirstPersonExtrasComponent::OnStartedStanding()
    {
    }
    void FirstPersonExtrasComponent::OnFirstJump()
    {
        if (m_networkFPCEnabled &&
            ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
              !m_firstPersonControllerObject->m_isHost)))
            return;
        m_tiltJumped = true;
        m_tiltLanded = false;
        m_moveHeadDown = true;
        m_totalHeadAngle = -m_headAngleJump;
    }
    void FirstPersonExtrasComponent::OnFinalJump()
    {
        if (m_networkFPCEnabled &&
            ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
              !m_firstPersonControllerObject->m_isHost)))
            return;
        m_tiltJumped = true;
        m_tiltLanded = false;
        m_moveHeadDown = true;
        m_totalHeadAngle = -m_headAngleJump;
    }
    void FirstPersonExtrasComponent::OnStaminaCapped()
    {
    }
    void FirstPersonExtrasComponent::OnStaminaReachedZero()
    {
    }
    void FirstPersonExtrasComponent::OnSprintStarted()
    {
    }
    void FirstPersonExtrasComponent::OnSprintStopped()
    {
    }
    void FirstPersonExtrasComponent::OnCooldownStarted()
    {
    }
    void FirstPersonExtrasComponent::OnCooldownDone()
    {
    }

    // Request Bus getter and setter methods for use in scripts
    float FirstPersonExtrasComponent::GetJumpPressedInAirQueueTimeThreshold() const
    {
        return m_jumpPressedInAirQueueTimeThreshold;
    }
    void FirstPersonExtrasComponent::SetJumpPressedInAirQueueTimeThreshold(const float jumpPressedInAirQueueTimeThreshold)
    {
        if (jumpPressedInAirQueueTimeThreshold < 0.f)
            m_jumpPressedInAirQueueTimeThreshold = 0.f;
        else
            m_jumpPressedInAirQueueTimeThreshold = jumpPressedInAirQueueTimeThreshold;
    }
    bool FirstPersonExtrasComponent::GetJumpHeadTiltEnabled() const
    {
        return m_jumpHeadTiltEnabled;
    }
    void FirstPersonExtrasComponent::SetJumpHeadTiltEnabled(const bool jumpHeadTiltEnabled)
    {
        m_jumpHeadTiltEnabled = jumpHeadTiltEnabled;
    }
    float FirstPersonExtrasComponent::GetHeadAngleJump() const
    {
        return -AZ::RadToDeg(m_headAngleJump);
    }
    void FirstPersonExtrasComponent::SetHeadAngleJump(const float headAngleJump)
    {
        m_headAngleJump = AZ::DegToRad(headAngleJump);
    }
    float FirstPersonExtrasComponent::GetHeadAngleLand() const
    {
        return -AZ::RadToDeg(m_headAngleLand);
    }
    void FirstPersonExtrasComponent::SetHeadAngleLand(const float headAngleLand)
    {
        m_headAngleLand = AZ::DegToRad(headAngleLand);
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorJump() const
    {
        return -m_deltaAngleFactorJump;
    }
    void FirstPersonExtrasComponent::SetDeltaAngleFactorJump(const float deltaAngleFactorJump)
    {
        m_deltaAngleFactorJump = deltaAngleFactorJump;
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorLand() const
    {
        return -m_deltaAngleFactorLand;
    }
    void FirstPersonExtrasComponent::SetDeltaAngleFactorLand(const float deltaAngleFactorLand)
    {
        m_deltaAngleFactorLand = deltaAngleFactorLand;
    }
    float FirstPersonExtrasComponent::GetCompleteHeadLandVelocity() const
    {
        return m_completeHeadLandVelocity;
    }
    void FirstPersonExtrasComponent::SetCompleteHeadLandVelocity(const float completeHeadLandVelocity)
    {
        if (completeHeadLandVelocity < 0.f)
            m_completeHeadLandVelocity = -completeHeadLandVelocity;
        else
            m_completeHeadLandVelocity = completeHeadLandVelocity;
    }
    bool FirstPersonExtrasComponent::GetSprintFoVEnabled() const
    {
        return m_sprintFoVEnabled;
    }
    void FirstPersonExtrasComponent::SetSprintFoVEnabled(const bool sprintFoVEnabled)
    {
        m_sprintFoVEnabled = sprintFoVEnabled;
    }
    float FirstPersonExtrasComponent::GetSprintFoVLerpTime() const
    {
        return m_sprintFoVLerpTime;
    }
    void FirstPersonExtrasComponent::SetSprintFoVLerpTime(const float sprintFoVLerpTime)
    {
        if (sprintFoVLerpTime < 0.0166f)
            m_sprintFoVLerpTime = 0.0166f;
        else
            m_sprintFoVLerpTime = sprintFoVLerpTime;
    }
    float FirstPersonExtrasComponent::GetSprintingFoV() const
    {
        return m_sprintFoV;
    }
    void FirstPersonExtrasComponent::SetSprintingFoV(const float sprintFoV)
    {
        m_sprintFoV = sprintFoV;
        m_sprintFoVDelta = m_sprintFoV - m_walkFoV;
    }
    float FirstPersonExtrasComponent::GetWalkingFoV() const
    {
        return m_walkFoV;
    }
    void FirstPersonExtrasComponent::SetWalkingFoV(const float walkFoV)
    {
        m_walkFoV = walkFoV;
    }
    bool FirstPersonExtrasComponent::GetHeadbobEnabled() const
    {
        return m_headbobEnabled;
    }
    void FirstPersonExtrasComponent::SetHeadbobEnabled(const bool headbobEnabled)
    {
        m_headbobEnabled = headbobEnabled;
    }
    bool FirstPersonExtrasComponent::GetHeadbobStartingDirection() const
    {
        return m_headbobStartingDirection;
    }
    void FirstPersonExtrasComponent::SetHeadbobStartingDirection(const bool headbobStartingDirection)
    {
        m_headbobStartingDirection = headbobStartingDirection;
    }
    float FirstPersonExtrasComponent::GetHeadbobOverallIntensity() const
    {
        return m_headbobOverallIntensity;
    }
    void FirstPersonExtrasComponent::SetHeadbobOverallIntensity(const float headbobOverallIntensity)
    {
        if (headbobOverallIntensity < 0.f)
            m_headbobOverallIntensity = 0.f;
        else
            m_headbobOverallIntensity = headbobOverallIntensity;
    }
    float FirstPersonExtrasComponent::GetHeadbobSmoothTime() const
    {
        return m_headbobSmoothTime;
    }
    void FirstPersonExtrasComponent::SetHeadbobSmoothTime(const float headbobSmoothTime)
    {
        if (headbobSmoothTime < 0.f)
            m_headbobSmoothTime = 0.f;
        else
            m_headbobSmoothTime = headbobSmoothTime;
    }
    float FirstPersonExtrasComponent::GetHeadbobRealism() const
    {
        return m_headbobRealism;
    }
    void FirstPersonExtrasComponent::SetHeadbobRealism(const float headbobRealism)
    {
        if (headbobRealism < 0.f)
            m_headbobRealism = 0.f;
        else
            m_headbobRealism = headbobRealism;
        UpdateHeadbobShapePeaks();
    }
    float FirstPersonExtrasComponent::GetHeadbobFootstepSharpness() const
    {
        return m_headbobFootstepSharpness;
    }
    void FirstPersonExtrasComponent::SetHeadbobFootstepSharpness(const float headbobFootstepSharpness)
    {
        m_headbobFootstepSharpness = headbobFootstepSharpness;
        UpdateHeadbobShapePeaks();
    }
    float FirstPersonExtrasComponent::GetHeadbobAlternatingStepDifference() const
    {
        return m_headbobAlternatingStepDifference;
    }
    void FirstPersonExtrasComponent::SetHeadbobAlternatingStepDifference(const float headbobAlternatingStepDifference)
    {
        m_headbobAlternatingStepDifference = headbobAlternatingStepDifference;
        UpdateHeadbobShapePeaks();
    }
    float FirstPersonExtrasComponent::GetHeadbobHorizontalSwayImbalance() const
    {
        return m_headbobHorizontalSwayImbalance;
    }
    void FirstPersonExtrasComponent::SetHeadbobHorizontalSwayImbalance(const float headbobHorizontalSwayImbalance)
    {
        m_headbobHorizontalSwayImbalance = headbobHorizontalSwayImbalance;
        UpdateHeadbobShapePeaks();
    }
    float FirstPersonExtrasComponent::GetHeadbobHorizontalSwayFlatness() const
    {
        return m_headbobHorizontalSwayFlatness;
    }
    void FirstPersonExtrasComponent::SetHeadbobHorizontalSwayFlatness(const float headbobHorizontalSwayFlatness)
    {
        m_headbobHorizontalSwayFlatness = headbobHorizontalSwayFlatness;
        UpdateHeadbobShapePeaks();
    }
    float FirstPersonExtrasComponent::GetHeadbobFootstepAcceleration() const
    {
        return m_headbobFootstepAcceleration;
    }
    void FirstPersonExtrasComponent::SetHeadbobFootstepAcceleration(const float headbobFootstepAcceleration)
    {
        m_headbobFootstepAcceleration = headbobFootstepAcceleration;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxPitchAmplitude() const
    {
        return m_headbobMaxPitchAmplitude;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxPitchAmplitude(const float headbobMaxPitchAmplitude)
    {
        if (headbobMaxPitchAmplitude < 0.f)
            m_headbobMaxPitchAmplitude = 0.f;
        else
            m_headbobMaxPitchAmplitude = headbobMaxPitchAmplitude;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxRollAmplitude() const
    {
        return m_headbobMaxRollAmplitude;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxRollAmplitude(const float headbobMaxRollAmplitude)
    {
        m_headbobMaxRollAmplitude = headbobMaxRollAmplitude;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxYawAmplitude() const
    {
        return m_headbobMaxYawAmplitude;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxYawAmplitude(const float headbobMaxYawAmplitude)
    {
        m_headbobMaxYawAmplitude = headbobMaxYawAmplitude;
    }
    float FirstPersonExtrasComponent::GetHeadbobStepVariationOverTime() const
    {
        return m_headbobStepVariationOverTime;
    }
    void FirstPersonExtrasComponent::SetHeadbobStepVariationOverTime(const float headbobStepVariationOverTime)
    {
        if (headbobStepVariationOverTime < 0.f)
            m_headbobStepVariationOverTime = 0.f;
        else
            m_headbobStepVariationOverTime = headbobStepVariationOverTime;
    }
    float FirstPersonExtrasComponent::GetHeadbobVerticalSprintScale() const
    {
        return m_headbobVerticalSprintScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobVerticalSprintScale(const float headbobVerticalSprintScale)
    {
        m_headbobVerticalSprintScale = headbobVerticalSprintScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobHorizontalSprintScale() const
    {
        return m_headbobHorizontalSprintScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobHorizontalSprintScale(const float headbobHorizontalSprintScale)
    {
        m_headbobHorizontalSprintScale = headbobHorizontalSprintScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobRotationCrouchScale() const
    {
        return m_headbobRotationCrouchScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobRotationCrouchScale(const float headbobRotationCrouchScale)
    {
        m_headbobRotationCrouchScale = headbobRotationCrouchScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobRotationSprintScale() const
    {
        return m_headbobRotationSprintScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobRotationSprintScale(const float headbobRotationSprintScale)
    {
        m_headbobRotationSprintScale = headbobRotationSprintScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxFrequency() const
    {
        return m_headbobMaxFrequency;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxFrequency(const float headbobMaxFrequency)
    {
        m_headbobMaxFrequency = headbobMaxFrequency;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxVerticalAmplitude() const
    {
        return m_headbobMaxVerticalAmplitude;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxVerticalAmplitude(const float headbobMaxVerticalAmplitude)
    {
        m_headbobMaxVerticalAmplitude = headbobMaxVerticalAmplitude;
    }
    float FirstPersonExtrasComponent::GetHeadbobMaxHorizontalAmplitude() const
    {
        return m_headbobMaxHorizontalAmplitude;
    }
    void FirstPersonExtrasComponent::SetHeadbobMaxHorizontalAmplitude(const float headbobMaxHorizontalAmplitude)
    {
        m_headbobMaxHorizontalAmplitude = headbobMaxHorizontalAmplitude;
    }
    float FirstPersonExtrasComponent::GetHeadbobVerticalCrouchScale() const
    {
        return m_headbobVerticalCrouchScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobVerticalCrouchScale(const float headbobVerticalCrouchScale)
    {
        m_headbobVerticalCrouchScale = headbobVerticalCrouchScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobHorizontalCrouchScale() const
    {
        return m_headbobHorizontalCrouchScale;
    }
    void FirstPersonExtrasComponent::SetHeadbobHorizontalCrouchScale(const float headbobHorizontalCrouchScale)
    {
        m_headbobHorizontalCrouchScale = headbobHorizontalCrouchScale;
    }
    float FirstPersonExtrasComponent::GetHeadbobLastStepStrength() const
    {
        return m_headbobLastStepStrength;
    }
    AZ::Vector3 FirstPersonExtrasComponent::GetCameraTranslationWithoutHeadbob() const
    {
        return m_cameraTranslationWithoutHeadbob;
    }
    AZ::Vector3 FirstPersonExtrasComponent::GetPreviousOffset() const
    {
        return m_prevHeadbobOffset;
    }
    void FirstPersonExtrasComponent::IgnoreInputs(const bool ignoreInputs)
    {
        if (ignoreInputs)
            InputEventNotificationBus::MultiHandler::BusDisconnect();
        else
            AssignConnectInputEvents();
    }
    void FirstPersonExtrasComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
        Camera::CameraNotificationBus::Handler::BusConnect();
    }
    void FirstPersonExtrasComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
    }
} // namespace FirstPersonController
