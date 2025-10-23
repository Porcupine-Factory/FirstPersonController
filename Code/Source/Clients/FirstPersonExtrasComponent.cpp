/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

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
                ->Version(1);

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
                    ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f);
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
                    &FirstPersonExtrasComponentRequests::SetJumpPressedInAirQueueTimeThreshold);

            bc->Class<FirstPersonExtrasComponent>()->RequestBus("FirstPersonExtrasComponentRequestBus");
        }
    }

    void FirstPersonExtrasComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());

        // Get access to the FirstPersonControllerComponent object and its members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();

        // Assign pointer attributes to the associated attributes of the FirstPersonControllerComponent, accessible via friendship
        if (m_firstPersonControllerObject)
        {
            m_jumpValue = &(m_firstPersonControllerObject->m_jumpValue);
            m_scriptJump = &(m_firstPersonControllerObject->m_scriptJump);
            m_grounded = &(m_firstPersonControllerObject->m_grounded);
        }

        // Assign the FirstPersonExtrasComponent specific inputs
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
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepStart([[maybe_unused]] const float& physicsTimestep)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish(const float& physicsTimestep)
    {
        ProcessInput(((physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor + m_prevTimestep) / 2.f), true);
        m_prevTimestep = physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor;
    }

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), false);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::QueueJump(const float& deltaTime, const bool& timestepElseTick)
    {
        // Bail if the threshold is set to zero
        if (m_jumpPressedInAirQueueTimeThreshold == 0.f)
            return;

        if (!timestepElseTick && !*m_grounded && m_prevJumpValue == 0.f && *m_jumpValue != 0.f)
        {
            // Reset the timer
            m_jumpPressedInAirTimer = 0.f;

            // Queue up the jump when it's pressed while in the air
            m_prevQueueJump = m_queueJump;
            m_queueJump = true;
        }
        else if (timestepElseTick && m_queueJump && !*m_grounded)
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
        else if (timestepElseTick && m_queueJump && *m_grounded)
        {
            // Perform the jump when the ground is hit
            m_prevQueueJump = m_queueJump;
            m_queueJump = false;
            *m_scriptJump = true;
        }
        else if (timestepElseTick && m_prevQueueJump && !m_queueJump)
        {
            // Clear the previous queue and script jump variables
            m_prevQueueJump = m_queueJump;
            *m_scriptJump = false;
        }

        // Keep track of the previous jump value
        if (!timestepElseTick)
            m_prevJumpValue = *m_jumpValue;
    }

    void FirstPersonExtrasComponent::ProcessInput(const float& deltaTime, const bool& timestepElseTick)
    {
        QueueJump(deltaTime, timestepElseTick);
    }

    // Event Notification methods for use in scripts
    void FirstPersonExtrasComponent::OnPlaceholder()
    {
    }

    // Notification Events from the First Person Controller component
    void FirstPersonExtrasComponent::OnGroundHit([[maybe_unused]] const float& fellDistance)
    {
    }
    void FirstPersonExtrasComponent::OnGroundSoonHit([[maybe_unused]] const float& soonFellDistance)
    {
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
    }
    void FirstPersonExtrasComponent::OnFinalJump()
    {
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

    float FirstPersonExtrasComponent::GetJumpPressedInAirQueueTimeThreshold() const
    {
        return m_jumpPressedInAirQueueTimeThreshold;
    }
    void FirstPersonExtrasComponent::SetJumpPressedInAirQueueTimeThreshold(const float& new_jumpPressedInAirQueueTimeThreshold)
    {
        if (new_jumpPressedInAirQueueTimeThreshold < 0.f)
            m_jumpPressedInAirQueueTimeThreshold = 0.f;
        else
            m_jumpPressedInAirQueueTimeThreshold = new_jumpPressedInAirQueueTimeThreshold;
    }
} // namespace FirstPersonController
