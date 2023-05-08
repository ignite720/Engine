#include "vpch.h"
#include "OrientationLock.h"
#include "Core/VMath.h"
#include "Core/Log.h"

void OrientationLock::Tick(float deltaTime)
{
    __super::Tick(deltaTime);

    if (!orientationActivated && IsOrientationCorrect())
    {
        orientationActivated = true;

        auto actor = World::GetActorByNameAllowNull(gridActorToActivateOnCorrectOrientation);
        if (actor)
        {
            auto gridActor = dynamic_cast<GridActor*>(actor);
            if (gridActor)
            {
                gridActor->Activate();
            }
        }
        else
        {
            Log("[%s] not found.", gridActorToActivateOnCorrectOrientation.c_str());
        }
    }
}

Properties OrientationLock::GetProps()
{
    auto props = __super::GetProps();
    props.title = GetTypeName();
    props.Add("Dir Forward", &forward);
    props.Add("Dir Up", &up);
    props.Add("Dir Right", &right);
    props.Add("Linked GridActor", &gridActorToActivateOnCorrectOrientation);
    return props;
}

bool OrientationLock::IsOrientationCorrect()
{
    auto forwardVector = GetForwardVectorV();
    auto upVector = GetUpVectorV();
    auto rightVector = GetRightVectorV();

    auto dirForward = XMLoadFloat3(&forward);
    auto dirUp = XMLoadFloat3(&up);
    auto dirRight = XMLoadFloat3(&right);

    int neededOrientations = 3;

    if (VMath::VecEqual(XMVectorZero(), dirForward)) { neededOrientations--; }
    if (VMath::VecEqual(XMVectorZero(), dirRight)) { neededOrientations--; }
    if (VMath::VecEqual(XMVectorZero(), dirUp)) { neededOrientations--; }

    bool forwardEqual = VMath::VecEqual(forwardVector, dirForward);
    bool rightEqual = VMath::VecEqual(rightVector, dirRight);
    bool upEqual = VMath::VecEqual(upVector, dirUp);

    int finalMatchingOrientations = forwardEqual + rightEqual + upEqual;
    return finalMatchingOrientations == neededOrientations;
}
