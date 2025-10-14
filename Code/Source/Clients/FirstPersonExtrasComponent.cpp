/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Clients/FirstPersonControllerComponent.h"
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
        AZ::TickBus::Handler::BusConnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());

        AZ::Entity* e = GetEntity();
        m_firstPersonControllerObject = e->FindComponent<FirstPersonControllerComponent>();

        if(m_firstPersonControllerObject)
        {
            m_jumpValue = &(m_firstPersonControllerObject->m_jumpValue);
            m_grounded = &(m_firstPersonControllerObject->m_grounded);
        }

        AssignConnectInputEvents();
    }

    void FirstPersonExtrasComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
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

    void FirstPersonExtrasComponent::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if(m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("First Person Extras Component", false, "Number of input IDs not equal to number of input names!");
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

    void FirstPersonExtrasComponent::OnPressed(float value)
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
                //AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonExtrasComponent::OnReleased(float value)
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

    void FirstPersonExtrasComponent::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;
    }

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), false);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::ProcessInput(const float& deltaTime, const bool& timestepElseTick)
    {
        if(!timestepElseTick && !m_prevJumpValue && *m_jumpValue && !*m_grounded)
        {
            AZ_Printf("First Person Extras Component", "Trying to jump!");
        }
        m_prevJumpValue = *m_jumpValue;
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
