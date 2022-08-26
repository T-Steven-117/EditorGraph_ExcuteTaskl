


#include "MissionObject.h"
#include "TaskObject.h"
#include "Tasks/Task_ConditionJumpTaskBase.h"


void UMissionObject::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);

	// 添加Tag给AssetData，在ContentBrowser悬浮此资源时能显示任务描述
	OutTags.Add(FAssetRegistryTag(TEXT("MissionDescription"), MissionDescription, FAssetRegistryTag::TT_Hidden));
}

#if WITH_EDITOR
void UMissionObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//FName PropertyName = PropertyChangedEvent.GetPropertyName();

	//// 更新设置子任务列表中的TaskID (如果有新的Task，分配一个新的唯一ID，原来的ID不变)
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(UMissionObject, Tasks))
	//{
	//	for (int32 i = 0; i < Tasks.Num(); ++i)
	//	{
	//		// 检查当前ID是否没有重复
	//		if (Tasks[i] == nullptr || !Tasks.FindByPredicate([CurTask = Tasks[i]](const UTaskObject* Task) {return Task && CurTask != Task && Task->TaskID == CurTask->TaskID; }))
	//		{ 
	//			// 跳过
	//			continue;
	//		}

	//		// 分配新ID
	//		bool InValidID = true;
	//		int32 ID = i;
	//		while (InValidID)
	//		{
	//			UTaskObject** FoundTask = Tasks.FindByPredicate([ID, CurTask= Tasks[i]](const UTaskObject* Task) {return Task && CurTask != Task && Task->TaskID == ID; });
	//			if (FoundTask)
	//			{
	//				++ID;
	//			}
	//			else
	//			{
	//				InValidID = false;
	//			}
	//		}

	//		Tasks[i]->TaskID = ID;
	//	}
	//}
	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	
	// 更新设置子任务列表中的TaskID (如果有新的Task，分配一个新的唯一ID，原来的ID不变)
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMissionObject, Tasks))
	{
		// 缓存ConditionJumpTask子任务类型需要对应的原来ID
		//TMap<UTask_ConditionJumpTaskBase*, UTaskObject*> JumpTaskMap;
		//for (int32 i = 0; i < Tasks.Num(); ++i)
		//{
		//	// 跳过没有实例的item
		//	if (!Tasks[i])
		//		continue;

		//	UTask_ConditionJumpTaskBase* JumpTask = Cast<UTask_ConditionJumpTaskBase>(Tasks[i]);
		//	if (JumpTask)
		//	{
		//		UTaskObject** TargetTask = Tasks.FindByPredicate([ID = JumpTask->TaskID_ToJump](const UTaskObject* Task){return Task && ID == Task->TaskID; });
		//		if (TargetTask)
		//		{
		//			JumpTaskMap.Add(JumpTask, *TargetTask);
		//		}
		//		else
		//		{
		//			// 配置的跳跃ID无效
		//			JumpTask->TaskID_ToJump = -1;
		//			UE_LOG(LogTemp, Error, TEXT("MissionList: %s 中的UTask_ConditionJumpTask 所配置的TaskID_ToJump: %d 对应的Task不存在，请修改!!"), *GetName(), JumpTask->TaskID_ToJump);
		//		}
		//	}
		//}

		// 重新更新各个ID
		for (int32 i = 0; i < Tasks.Num(); ++i)
		{
			if (!Tasks[i])
				continue;

			// 更新ID
			Tasks[i]->TaskID = i;

			// 更新Outer (避免从其他蓝图资源复制过来的东西Outer不对,无法执行Task)
			Tasks[i]->Rename(nullptr, this);
		}

		//// 重新修改ConditionJumpTask子任务类型TaskID_ToJump
		//for (auto item : JumpTaskMap)
		//{
		//	if (item.Key && item.Value)
		//	{
		//		item.Key->TaskID_ToJump = item.Value->TaskID;
		//	}
		//}
	}
}
void UMissionObject::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.PropertyChain.GetTail())
		return;

	UProperty* TaskIDProperty = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
	//UObject* TaskProperty = TaskIDProperty->GetOuter();
	FName PropertyName = TaskIDProperty ? TaskIDProperty->GetFName() : NAME_None;

	// 更新设置子任务列表中的TaskID (如果有新的Task，分配一个新的唯一ID，原来的ID不变)
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UTaskObject, TaskID))
	{
		//UObjectProperty* TaskObjectProperty = static_cast<UObjectProperty*>(TaskProperty);
		//if (!TaskObjectProperty)
		//	return;

		//UObject* TaskObject = UObjectProperty::GetPropertyValue(TaskObjectProperty);
		//if (!TaskObject)
		//	return;
		//return;
		//UTaskObject* CurTask = Cast<UTaskObject>(TaskObject);
		//if (!CurTask)
		//	return;

		//UIntProperty* IDProperty = static_cast<UIntProperty*>(TaskIDProperty);
		//if (!IDProperty)
		//	return;

		//// 检查当前ID是否没有重复
		//int32 CurTaskID = IDProperty->GetPropertyValue_InContainer(this);
		//if (!CurTaskID)
		//	return;

		//int32 DuplicatedID = 0;
		//for (auto item : Tasks)
		//{
		//	if (CurTaskID == item->TaskID)
		//	{
		//		++DuplicatedID;
		//	}
		//}
		//if (DuplicatedID <= 1)
		//{
		//	return;
		//}

		////// 检查当前ID是否没有重复
		////if (CurTask == nullptr || !Tasks.FindByPredicate([CurTask](const UTaskObject* Task) {return Task && CurTask != Task && Task->TaskID == CurTask->TaskID; }))
		////{
		////	// 跳过
		////	return;
		////}

		//// 分配新ID
		//bool InValidID = true;
		//int32 ID = 1;
		//while (InValidID)
		//{
		//	UTaskObject** FoundTask = Tasks.FindByPredicate([ID](const UTaskObject* Task) {return Task && Task->TaskID == ID; });
		//	if (FoundTask)
		//	{
		//		++ID;
		//	}
		//	else
		//	{
		//		InValidID = false;
		//	}
		//}

		//IDProperty->SetPropertyValue_InContainer(this, ID);
	}
}
#endif // WITH_EDITOR
