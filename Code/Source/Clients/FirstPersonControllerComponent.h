#pragma once
#include <AzCore/Component/Component.h>

namespace FirstPersonController
{
    class FirstPersonControllerComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(FirstPersonControllerComponent, "{0a47c7c2-0f94-48dd-8e3f-fd55c30475b9}");

        static void Reflect(AZ::ReflectContext* rc);

        void Activate() override {};
        void Deactivate() override {};

    private:
        ;
    };
}
