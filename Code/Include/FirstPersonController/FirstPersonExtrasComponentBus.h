/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class FirstPersonExtrasComponentRequests : public AZ::ComponentBus
    {
    public:
        ~FirstPersonExtrasComponentRequests() override = default;

        virtual float GetJumpPressedInAirQueueTimeThreshold() const = 0;
        virtual void SetJumpPressedInAirQueueTimeThreshold(const float) = 0;
        virtual bool GetJumpHeadTiltEnabled() const = 0;
        virtual void SetJumpHeadTiltEnabled(const bool) = 0;
        virtual float GetJumpStaminaEquivalentSprintTime() const = 0;
        virtual void SetJumpStaminaEquivalentSprintTime(const float) = 0;
        virtual float GetJumpExhaustionFactor() const = 0;
        virtual void SetJumpExhaustionFactor(const float) = 0;
        virtual float GetJumpDefaultHoldDistance() const = 0;
        virtual void SetJumpDefaultHoldDistance(const float) = 0;
        virtual float GetJumpDefaultInitialVelocity() const = 0;
        virtual void SetJumpDefaultInitialVelocity(const float) = 0;
        virtual float GetHeadAngleJump() const = 0;
        virtual void SetHeadAngleJump(const float) = 0;
        virtual float GetHeadAngleLand() const = 0;
        virtual void SetHeadAngleLand(const float) = 0;
        virtual float GetDeltaAngleFactorJump() const = 0;
        virtual void SetDeltaAngleFactorJump(const float) = 0;
        virtual float GetDeltaAngleFactorLand() const = 0;
        virtual void SetDeltaAngleFactorLand(const float) = 0;
        virtual float GetCompleteHeadLandVelocity() const = 0;
        virtual void SetCompleteHeadLandVelocity(const float) = 0;
        virtual bool GetSprintFoVEnabled() const = 0;
        virtual void SetSprintFoVEnabled(const bool) = 0;
        virtual float GetSprintFoVLerpTime() const = 0;
        virtual void SetSprintFoVLerpTime(const float) = 0;
        virtual float GetSprintingFoV() const = 0;
        virtual void SetSprintingFoV(const float) = 0;
        virtual float GetWalkingFoV() const = 0;
        virtual void SetWalkingFoV(const float) = 0;
        virtual bool GetHeadbobEnabled() const = 0;
        virtual void SetHeadbobEnabled(const bool) = 0;
        virtual bool GetHeadbobStartingDirection() const = 0;
        virtual void SetHeadbobStartingDirection(const bool) = 0;
        virtual float GetHeadbobMaxFrequency() const = 0;
        virtual void SetHeadbobMaxFrequency(const float) = 0;
        virtual float GetHeadbobMaxVerticalAmplitude() const = 0;
        virtual void SetHeadbobMaxVerticalAmplitude(const float) = 0;
        virtual float GetHeadbobMaxHorizontalAmplitude() const = 0;
        virtual void SetHeadbobMaxHorizontalAmplitude(const float) = 0;
        virtual float GetHeadbobOverallIntensity() const = 0;
        virtual void SetHeadbobOverallIntensity(const float) = 0;
        virtual float GetHeadbobSmoothTime() const = 0;
        virtual void SetHeadbobSmoothTime(const float) = 0;
        virtual float GetHeadbobRealism() const = 0;
        virtual void SetHeadbobRealism(const float) = 0;
        virtual float GetHeadbobFootstepSharpness() const = 0;
        virtual void SetHeadbobFootstepSharpness(const float) = 0;
        virtual float GetHeadbobAlternatingStepDifference() const = 0;
        virtual void SetHeadbobAlternatingStepDifference(const float) = 0;
        virtual float GetHeadbobHorizontalSwayImbalance() const = 0;
        virtual void SetHeadbobHorizontalSwayImbalance(const float) = 0;
        virtual float GetHeadbobHorizontalSwayFlatness() const = 0;
        virtual void SetHeadbobHorizontalSwayFlatness(const float) = 0;
        virtual float GetHeadbobFootstepAcceleration() const = 0;
        virtual void SetHeadbobFootstepAcceleration(const float) = 0;
        virtual float GetHeadbobMaxPitchAmplitude() const = 0;
        virtual void SetHeadbobMaxPitchAmplitude(const float) = 0;
        virtual float GetHeadbobMaxRollAmplitude() const = 0;
        virtual void SetHeadbobMaxRollAmplitude(const float) = 0;
        virtual float GetHeadbobMaxYawAmplitude() const = 0;
        virtual void SetHeadbobMaxYawAmplitude(const float) = 0;
        virtual float GetHeadbobStepVariationOverTime() const = 0;
        virtual void SetHeadbobStepVariationOverTime(const float) = 0;
        virtual float GetHeadbobVerticalSprintScale() const = 0;
        virtual void SetHeadbobVerticalSprintScale(const float) = 0;
        virtual float GetHeadbobHorizontalSprintScale() const = 0;
        virtual void SetHeadbobHorizontalSprintScale(const float) = 0;
        virtual float GetHeadbobRotationSprintScale() const = 0;
        virtual void SetHeadbobRotationSprintScale(const float) = 0;
        virtual float GetHeadbobVerticalCrouchScale() const = 0;
        virtual void SetHeadbobVerticalCrouchScale(const float) = 0;
        virtual float GetHeadbobHorizontalCrouchScale() const = 0;
        virtual void SetHeadbobHorizontalCrouchScale(const float) = 0;
        virtual float GetHeadbobRotationCrouchScale() const = 0;
        virtual void SetHeadbobRotationCrouchScale(const float) = 0;
        virtual float GetHeadbobLastStepStrength() const = 0;
        virtual AZ::Vector3 GetCameraTranslationWithoutHeadbob() const = 0;
        virtual AZ::Vector3 GetPreviousOffset() const = 0;
        virtual void IgnoreInputs(const bool) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using FirstPersonExtrasComponentRequestBus = AZ::EBus<FirstPersonExtrasComponentRequests>;

    class FirstPersonExtrasComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnJumpFromQueue() = 0;
        virtual void OnHeadbobStepTaken() = 0;
        virtual void OnHeadbobOriginCross() = 0;
    };

    using FirstPersonExtrasComponentNotificationBus = AZ::EBus<FirstPersonExtrasComponentNotifications>;

    class FirstPersonExtrasComponentNotificationHandler
        : public FirstPersonExtrasComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            FirstPersonExtrasComponentNotificationHandler,
            "{3fafe1bc-50bb-42ba-bd1f-f20df1706517}",
            AZ::SystemAllocator,
            OnJumpFromQueue,
            OnHeadbobStepTaken,
            OnHeadbobOriginCross);

        void OnJumpFromQueue() override
        {
            Call(FN_OnJumpFromQueue);
        }
        void OnHeadbobStepTaken() override
        {
            Call(FN_OnHeadbobStepTaken);
        }
        void OnHeadbobOriginCross() override
        {
            Call(FN_OnHeadbobOriginCross);
        }
    };
} // namespace FirstPersonController
