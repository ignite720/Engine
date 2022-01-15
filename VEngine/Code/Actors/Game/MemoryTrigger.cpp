#include "MemoryTrigger.h"
#include "Components/BoxTriggerComponent.h"
#include "Components/MemoryComponent.h"
#include "Gameplay/GameUtils.h"
#include "Input.h"
#include "World.h"
#include "UI/AcceptMemoryWidget.h"

MemoryTrigger::MemoryTrigger()
{
    trigger = BoxTriggerComponent::system.Add(this);
    rootComponent = trigger;

    memoryComponent = MemoryComponent::system.Add(this);
}

void MemoryTrigger::Start()
{
    trigger->targetActor = (Actor*)GameUtils::GetPlayer();
}

void MemoryTrigger::Tick(float deltaTime)
{
    if (trigger->ContainsTarget())
    {
        if (Input::GetKeyUp(Keys::Down))
        {
            auto linkedActor = world.GetActorByNameAllowNull(actorName);
            if (linkedActor)
            {
                auto acceptMemoryWidget = CreateWidget<AcceptMemoryWidget>();
                acceptMemoryWidget->AddToViewport();
                acceptMemoryWidget->actorName = linkedActor->name;
                acceptMemoryWidget->acceptButtonCallback = std::bind(
                    &MemoryComponent::CreateMemory, memoryComponent, linkedActor->name);
            }
        }
    }
}

Properties MemoryTrigger::GetProps()
{
    auto props = __super::GetProps();
    props.AddProp(actorName);
    return props;
}
