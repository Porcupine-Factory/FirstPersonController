/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonExtrasComponent::Reflect(AZ::ReflectContext* rc)
    {
        if(auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonExtrasComponent, AZ::Component>()
              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonExtrasComponent>("First Person Extras",
                    "The First Person Extras component provides you with extra features for your first person character controller")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ");
            }
        }

        if(auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonExtrasComponentNotificationBus>("FirstPersonExtrasComponentNotificationBus")
                ->Handler<FirstPersonExtrasComponentNotificationHandler>();

            bc->EBus<FirstPersonExtrasComponentRequestBus>("FirstPersonExtrasComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Extras");

            bc->Class<FirstPersonExtrasComponent>()->RequestBus("FirstPersonExtrasComponentRequestBus");
        }
    }

    void FirstPersonExtrasComponent::Activate()
    {
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void FirstPersonExtrasComponent::Deactivate()
    {
        FirstPersonControllerComponentNotificationBus::Handler::BusDisconnect();
        FirstPersonExtrasComponentRequestBus::Handler::BusDisconnect();
    }

    void FirstPersonExtrasComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
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

    // Event Notification methods for use in scripts
    void FirstPersonExtrasComponent::OnPlaceholder(){}

    // Notification Events from the First Person Controller component
    void FirstPersonExtrasComponent::OnPhysicsTimestepStart([[maybe_unused]] const float& timeStep){}
    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish([[maybe_unused]] const float& timeStep){}
    void FirstPersonExtrasComponent::OnGroundHit([[maybe_unused]] const float& fellDistance){}
    void FirstPersonExtrasComponent::OnGroundSoonHit([[maybe_unused]] const float& soonFellDistance){}
    void FirstPersonExtrasComponent::OnUngrounded(){}
    void FirstPersonExtrasComponent::OnStartedFalling(){}
    void FirstPersonExtrasComponent::OnJumpApogeeReached(){}
    void FirstPersonExtrasComponent::OnStartedMoving(){}
    void FirstPersonExtrasComponent::OnTargetVelocityReached(){}
    void FirstPersonExtrasComponent::OnStopped(){}
    void FirstPersonExtrasComponent::OnTopWalkSpeedReached(){}
    void FirstPersonExtrasComponent::OnTopSprintSpeedReached(){}
    void FirstPersonExtrasComponent::OnHeadHit(){}
    void FirstPersonExtrasComponent::OnCharacterShapecastHitSomething([[maybe_unused]] const AZStd::vector<AzPhysics::SceneQueryHit> characterHits){}
    void FirstPersonExtrasComponent::OnVelocityXYObstructed(){}
    void FirstPersonExtrasComponent::OnCharacterGravityObstructed(){}
    void FirstPersonExtrasComponent::OnCrouched(){}
    void FirstPersonExtrasComponent::OnStoodUp(){}
    void FirstPersonExtrasComponent::OnStandPrevented(){}
    void FirstPersonExtrasComponent::OnStartedCrouching(){}
    void FirstPersonExtrasComponent::OnStartedStanding(){}
    void FirstPersonExtrasComponent::OnFirstJump(){}
    void FirstPersonExtrasComponent::OnFinalJump(){}
    void FirstPersonExtrasComponent::OnStaminaCapped(){}
    void FirstPersonExtrasComponent::OnStaminaReachedZero(){}
    void FirstPersonExtrasComponent::OnSprintStarted(){}
    void FirstPersonExtrasComponent::OnSprintStopped(){}
    void FirstPersonExtrasComponent::OnCooldownStarted(){}
    void FirstPersonExtrasComponent::OnCooldownDone(){}
} // namespace FirstPersonController
