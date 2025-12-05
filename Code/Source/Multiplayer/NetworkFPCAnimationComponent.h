#pragma once

#include <Integration/ActorComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Source/AutoGen/NetworkFPCAnimationComponent.AutoComponent.h>

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
    // This is not documented, you kind of have to jump into EMotionFX's private headers to find this, invalid parameter index values are
    // max size_t See InvalidIndex in Gems\EMotionFX\Code\EMotionFX\Source\EMotionFXConfig.h
    constexpr size_t InvalidParamIndex = 0xffffffffffffffff;

    class NetworkFPCAnimationComponent
        : public NetworkFPCAnimationComponentBase
        , private EMotionFX::Integration::ActorComponentNotificationBus::Handler
        , private EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            FirstPersonController::NetworkFPCAnimationComponent,
            s_networkFPCAnimationComponentConcreteUuid,
            FirstPersonController::NetworkFPCAnimationComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkFPCAnimationComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    protected:
        void OnPreRender(float deltaTime);

        //! EMotionFX::Integration::ActorComponentNotificationBus::Handler
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;

        //! EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
        void OnAnimGraphInstanceCreated(EMotionFX::AnimGraphInstance* animGraphInstance) override;

        Multiplayer::EntityPreRenderEvent::Handler m_preRenderEventHandler;

        EMotionFX::Integration::ActorComponentRequests* m_actorRequests = nullptr;
        EMotionFX::AnimGraphComponentNetworkRequests* m_networkRequests = nullptr;
        EMotionFX::Integration::AnimGraphComponentRequests* m_animationGraph = nullptr;

        size_t m_walkSpeedParamId = InvalidParamIndex;
        size_t m_sprintParamId = InvalidParamIndex;
    };

} // namespace FirstPersonController
