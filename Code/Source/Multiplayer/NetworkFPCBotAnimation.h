/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/NetworkFPCBotAnimationControllerBus.h>

#include <Source/AutoGen/NetworkFPCBotAnimation.AutoComponent.h>

#include <Clients/FirstPersonControllerComponent.h>
#include <Multiplayer/NetworkFPC.h>

#include <Integration/ActorComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>

#include <Multiplayer/Components/NetBindComponent.h>

namespace EMotionFX
{
    class AnimGraphComponentNetworkRequests;
    namespace Integration
    {
        class ActorComponentRequests;
        class AnimGraphComponentRequests;
    } // namespace Integration
} // namespace EMotionFX

namespace FirstPersonController
{
    class FirstPersonControllerComponent;

    class NetworkFPCBotAnimation
        : public NetworkFPCBotAnimationBase
        , public EMotionFX::Integration::ActorComponentNotificationBus::Handler
        , public EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            FirstPersonController::NetworkFPCBotAnimation,
            s_networkFPCBotAnimationConcreteUuid,
            FirstPersonController::NetworkFPCBotAnimationBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkFPCBotAnimation();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnPreRender(float deltaTime);

        // EnableAnimationNetworkFPCBotAnimation Changed Event
        AZ::Event<bool>::Handler m_enableNetworkAnimationChangedEvent;
        void OnEnableNetworkAnimationChanged(const bool& enable);

        //! EMotionFX::Integration::ActorComponentNotificationBus::Handler
        //! @{
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;
        //! @}

        // FirstPersonControllerComponent object
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;

        //! EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
        //! @{
        void OnAnimGraphInstanceCreated(EMotionFX::AnimGraphInstance* animGraphInstance) override;
        //! @}

        // Network animation members
        Multiplayer::EntityPreRenderEvent::Handler m_preRenderEventHandler;

        EMotionFX::Integration::ActorComponentRequests* m_actorRequests = nullptr;
        EMotionFX::AnimGraphComponentNetworkRequests* m_networkRequests = nullptr;
        EMotionFX::Integration::AnimGraphComponentRequests* m_animationGraph = nullptr;

        bool m_animationChildFound = false;

        AZ::EntityId m_animationEntityId = AZ::EntityId();

        void DetectAnimationChild();
        void SetupAnimationConnections(const AZ::EntityId& targetId);

        bool m_paramIdsSet = false;
        size_t m_walkSpeedParamId = InvalidParamIndex;
        size_t m_sprintParamId = InvalidParamIndex;
        size_t m_crouchToStandParamId = InvalidParamIndex;
        size_t m_crouchParamId = InvalidParamIndex;
        size_t m_standToCrouchParamId = InvalidParamIndex;
        size_t m_jumpStartParamId = InvalidParamIndex;
        size_t m_fallParamId = InvalidParamIndex;
        size_t m_landParamId = InvalidParamIndex;
        size_t m_groundedParamId = InvalidParamIndex;

        // NOTE: Make sure to add any new param Ids to this param Ids array
        size_t* m_paramIds[9] = { &m_walkSpeedParamId, &m_sprintParamId,        &m_crouchToStandParamId,
                                  &m_crouchParamId,    &m_standToCrouchParamId, &m_jumpStartParamId,
                                  &m_fallParamId,      &m_landParamId,          &m_groundedParamId };
    };

    class NetworkFPCBotAnimationController
        : public NetworkFPCBotAnimationControllerBase
        , public NetworkFPCBotAnimationControllerRequestBus::Handler
    {
        friend class FirstPersonControllerComponent;

    public:
        explicit NetworkFPCBotAnimationController(NetworkFPCBotAnimation& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        //! Common input creation logic for the NetworkInput.
        //! Fill out the input struct and the MultiplayerInputDriver will send the input data over the network
        //!    to ensure it's processed.
        //! @param input  input structure which to store input data for sending to the authority
        //! @param deltaTime amount of time to integrate the provided inputs over
        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;

        //! Common input processing logic for the NetworkInput.
        //! @param input  input structure to process
        //! @param deltaTime amount of time to integrate the provided inputs over
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;

    private:
        void OnPreRender(float deltaTime);
    };
} // namespace FirstPersonController
