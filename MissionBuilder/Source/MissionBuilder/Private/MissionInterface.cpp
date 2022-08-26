


#include "MissionInterface.h"

// Add default functionality here for any IMissionInterface functions that are not pure virtual.

void IMissionInterface::ToNextTask()
{

}

void IMissionInterface::JumpToTask(int32 TaskID)
{
}

void IMissionInterface::OnTaskFinished(UTaskObject* Task, bool bSuccess)
{
}

APawn* IMissionInterface::GetPlayerPawn()
{
    return nullptr;
}

FString IMissionInterface::GetMissionName()
{
    return FString();
}

void IMissionInterface::RegisterPersistentTick(UTaskObject* Task)
{

}

void IMissionInterface::UnRegisterPersistentTick(UTaskObject* Task)
{

}

bool IMissionInterface::GetHasPersistenTickTask()
{
    return false;
}
