// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionObjectGraph.h"
#include "UObject/UObjectHash.h"
#include "EdGraph/EdGraphSchema.h"
#include "AIGraphTypes.h"
#include "MOGraphNode.h"
#include "MOGraphNode_Root.h"
#include "MOGraphNode_Task.h"
#include "MOGraphNode_Branch.h"

#include "MissionObject.h"
#include "TaskObject.h"
#include "Tasks/Task_Branch.h"
#include "Tasks/Task_Parallel.h"
//#include "MissionObjectGraphModule.h"

void UMissionObjectGraph::UpdateAsset(int32 UpdateFlags)
{
	UMissionObject* MO = Cast<UMissionObject>(GetOuter());
	if (!MO)
		return;

	UMOGraphNode_Root* RootNode = nullptr;
	UMOGraphNode* FirtNode = nullptr;
	// 找出根节点的下一个节点，也就是第一个Task Node
	for (auto n : Nodes)
	{
		RootNode = Cast<UMOGraphNode_Root>(n);
		if (RootNode)
		{
			break;
		}
	}

	// we can't look at pins until pin references have been fixed up post undo:
	UEdGraphPin::ResolveAllPinReferences();
	if (RootNode && RootNode->Pins.Num() > 0 && RootNode->Pins[0]->LinkedTo.Num() > 0)
	{
		UMOGraphNode* Node = Cast<UMOGraphNode>(RootNode->Pins[0]->LinkedTo[0]->GetOwningNode());
		if (Node)
		{
			// NodeInstace为TaskObject
			if (UTaskObject* Task = Cast<UTaskObject>(Node->NodeInstance))
			{
				// 重新设置NodeInstance的Outer为MissionObject
				Node->NodeInstance->Rename(nullptr, MO);
				// 设置MissionObject的第一个Task
				MO->RootTask = Task;
				// 第一个节点为并行节点
				if (Cast<UTask_Parallel>(Task))
				{
					CreateTaskFromParallelNode(Node, Cast<UTask_Parallel>(Task), MO);
				}

				// 生成Task树
				CreateTaskFromNode(Node, Task, MO);
			}
		}
	}
	// 更新TaskID
	UpdateTaskID(MO->RootTask);
}

void UMissionObjectGraph::OnCreated()
{
	UMOGraph::OnCreated();
}

void UMissionObjectGraph::OnLoaded()
{
	UMOGraph::OnLoaded();
}

void UMissionObjectGraph::Initialize()
{
	UMOGraph::Initialize();
}

void UMissionObjectGraph::OnNodeInstanceRemoved(UObject* NodeInstance)
{
	// empty in base class
}

void UMissionObjectGraph::CreateTaskFromNode(UMOGraphNode* ParentNode, UTaskObject* ParentTask, UMissionObject* MO)
{
	if (ParentNode == nullptr)
		return;

	// 是否为分支节点判断
	UMOGraphNode_Branch* BranchNode = Cast<UMOGraphNode_Branch>(ParentNode);
	if (BranchNode)	// 分支节点
	{
		// 遍历所有Pin
		for (int32 i = 0; i < BranchNode->Pins.Num(); ++i)
		{
			// 父类确保是UTask_Branch
			UTask_Branch* BranchTask = Cast<UTask_Branch>(ParentTask);
			if (BranchNode->Pins[i]->Direction == EGPD_Output && BranchNode->Pins[i]->LinkedTo.Num() > 0 && BranchNode->Pins[i]->LinkedTo[0]->GetOwningNode())
			{
				// 获取子节点
				UMOGraphNode* Node = Cast<UMOGraphNode>(ParentNode->Pins[i]->LinkedTo[0]->GetOwningNode());
				if (Node)
				{
					// NodeInstace为TaskObject
					if (UTaskObject* Task = Cast<UTaskObject>(Node->NodeInstance))
					{
						// 重新设置NodeInstance的Outer为MissionObject
						Node->NodeInstance->Rename(nullptr, MO);

						if (ParentTask == nullptr)
						{
							// 设置MissionObject的第一个Task
							MO->RootTask = Task;
						}
						else
						{
							// 1号pin左边，2号pin右边 （左假右真）
							if (BranchNode->Pins[i]->PinName.ToString().Equals(TEXT("False")))
								//if (i == 1)
								BranchTask->NextTask = Task;
							else
								BranchTask->NextTask2 = Task;
						}

						// 节点为并行节点
						if (Cast<UTask_Parallel>(Task))
						{
							CreateTaskFromParallelNode(Node, Cast<UTask_Parallel>(Task), MO);
						}

						// 生成Task树
						CreateTaskFromNode(Node, Task, MO);
					}
				}
			}
			else if(BranchNode->Pins[i]->Direction == EGPD_Output)// 没有子节点，设置TaskNext为空指针
			{
				// 1号pin左边，2号pin右边 （左假右真）
				if (BranchNode->Pins[i]->PinName.ToString().Equals(TEXT("False")))
					//if (i == 1)
					BranchTask->NextTask = nullptr;
				else
					BranchTask->NextTask2 = nullptr;
			}
		}
	}
	else // 普通Task（非分支）
	{
		if (ParentNode && ParentNode->Pins.Num() > 0 && ParentNode->Pins[1]->LinkedTo.Num() > 0)
		{
			// 下一个连接节点存在
			UMOGraphNode* Node = Cast<UMOGraphNode>(ParentNode->Pins[1]->LinkedTo[0]->GetOwningNode());
			if (Node)
			{
				// NodeInstace为TaskObject
				if (UTaskObject* Task = Cast<UTaskObject>(Node->NodeInstance))
				{
					// 重新设置NodeInstance的Outer为MissionObject
					Node->NodeInstance->Rename(nullptr, MO);
					if (ParentTask == nullptr)
					{
						// 设置MissionObject的第一个Task
						MO->RootTask = Task;
					}
					else
					{
						ParentTask->NextTask = Task;
					}

					// 节点为并行节点
					if (Cast<UTask_Parallel>(Task))
					{
						CreateTaskFromParallelNode(Node, Cast<UTask_Parallel>(Task), MO);
					}
					// 生成Task树
					CreateTaskFromNode(Node, Task, MO);
				}
			}
		}
		else // 没有子节点，设置TaskNext为空指针
		{
			ParentTask->NextTask = nullptr;
		}
	}
}

void UMissionObjectGraph::CreateTaskFromParallelNode(UMOGraphNode* ParallelNode, class UTask_Parallel* ParallelTask, class UMissionObject* MO)
{
	// 清空RootTasks
	ParallelTask->RootTasks.Empty();

	// 遍历并行Pin的所有连接，每个连接做查找，每条是一个TaskObject数组
	for (int32 i = 0; i < ParallelNode->Pins.Num(); ++i)
	{
		// 并行Pin且元素大于0
		if (!ParallelNode->Pins[i]->PinName.ToString().Equals(TEXT("Parallel")) || ParallelNode->Pins[i]->LinkedTo.Num() == 0)
			continue;

		// 遍历所有LinkedTo
		for (auto Pin : ParallelNode->Pins[i]->LinkedTo)
		{
			UMOGraphNode* Node = Cast<UMOGraphNode>(Pin->GetOwningNode());
			if (!Node)
				continue;
			UTaskObject* RootTask = Cast<UTaskObject>(Node->NodeInstance);
			if (!RootTask)
				continue;

			// 重新设置NodeInstance的Outer为MissionObject
			Node->NodeInstance->Rename(nullptr, MO);
			// 并行任务添加一个RootTask
			ParallelTask->RootTasks.Add(RootTask);
			// 当前Root节点继续向下遍历查找
			CreateTaskFromNode(Node, RootTask, MO);
		}
	}
}

#if WITH_EDITOR
void UMissionObjectGraph::PostEditUndo()
{
	Super::PostEditUndo();

	// make sure that all execution indices are up to date
	UpdateAsset(2);
	Modify();
}
#endif // WITH_EDITOR

void UMissionObjectGraph::UpdateTaskID(UTaskObject* RootTask)
{
	// 遍历所有节点设置ID为-1
	for (auto n : Nodes)
	{
		if (UMOGraphNode* Node = Cast<UMOGraphNode>(n))
		{
			// NodeInstace为TaskObject
			if (UTaskObject* Task = Cast<UTaskObject>(Node->NodeInstance))
			{
				Task->TaskID = -1;
			}
		}
	}

	int32 TaskID = 0;
	RecursionUpdateTaskID(TaskID, RootTask);
}

void UMissionObjectGraph::RecursionUpdateTaskID(int32& TaskID, UTaskObject* LastTask)
{
	if (!LastTask)
		return;

	// 设置过ID的不更新
	if(LastTask->TaskID == -1)
		LastTask->TaskID = TaskID++;

	if (UTask_Branch* Branch = Cast<UTask_Branch>(LastTask))
	{
		// 分支中查找 - 遇到左右分支连接相同的Task时跳出（代表分支结束）
		UTaskObject* BranchEndTask = UpdateBranchTaskID(TaskID, Branch);

		RecursionUpdateTaskID(TaskID, BranchEndTask);
	}
	else if (UTask_Parallel* Parallel = Cast <UTask_Parallel>(LastTask))
	{
		for (auto RootTask : Parallel->RootTasks)
		{
			RecursionUpdateTaskID(TaskID, RootTask);
		}

		RecursionUpdateTaskID(TaskID, Parallel->NextTask);
	}
	else
	{
		RecursionUpdateTaskID(TaskID, LastTask->NextTask);
	}
}

UTaskObject* UMissionObjectGraph::UpdateBranchTaskID(int32& TaskID, class UTask_Branch* Branch)
{
	if (!Branch)
		return nullptr;

	// 更新Branch的ID
	//Branch->TaskID = TaskID++;

	bool LeftTurn = true;

	UTaskObject* CurLeft = Branch->NextTask;
	UTaskObject* CurRight = Branch->NextTask2;

	UTaskObject* EndBranch = nullptr;

	// 遇到左右分支连接相同的Task时跳出（代表分支结束）==》左边遍历一个节点，再接着右边遍历一个节点，一层层往下（遇到子分支则整个分支看成一个节点）
	while (CurLeft != CurRight)
	{
		if (LeftTurn)
		{
			// 左边Task为空
			if (!CurLeft)
			{
				LeftTurn = false; // 下一轮操作右边
			}
			else
			{
				EndBranch = CurLeft;

				// 设置过ID的不更新
				if (CurLeft->TaskID == -1)
					CurLeft->TaskID = TaskID++;

				if (UTask_Branch* LeftBranch = Cast<UTask_Branch>(CurLeft))
				{
					UTaskObject* BranchEndTask = UpdateBranchTaskID(TaskID, LeftBranch);

					CurLeft = BranchEndTask; // 更新左边节点
					LeftTurn = false;	// 轮到右边判断

					if (CurLeft != nullptr)
						EndBranch = CurLeft;
				}
				else
				{
					CurLeft = CurLeft->NextTask; // 更新左边节点
					LeftTurn = false;	// 轮到右边判断

					if (CurLeft != nullptr)
						EndBranch = CurLeft;
				}
			}
		}
		else
		{
			// 右边Task为空
			if (!CurRight)
			{
				LeftTurn = true; // 下一轮操作左边
			}
			else
			{
				EndBranch = CurRight;

				// 设置过ID的不更新
				if (CurRight->TaskID == -1)
					CurRight->TaskID = TaskID++;

				if (UTask_Branch* RightBranch = Cast<UTask_Branch>(CurRight))
				{
					UTaskObject* BranchEndTask = UpdateBranchTaskID(TaskID, RightBranch);

					CurRight = BranchEndTask; // 更新右边节点
					LeftTurn = true;	// 轮到左边判断

					if (CurRight != nullptr)
						EndBranch = CurRight;
				}
				else
				{
					CurRight = CurRight->NextTask; // 更新右边节点
					LeftTurn = true;	// 轮到左边判断

					if (CurRight != nullptr)
						EndBranch = CurRight;
				}
			}
		}
	}

	return EndBranch;
}

void UMissionObjectGraph::OnNodesPasted(const FString& ImportStr)
{
	// empty in base class
}

void UMissionObjectGraph::OnSave()
{
	UpdateAsset();
}

void UMissionObjectGraph::OnSubNodeDropped()
{
	NotifyGraphChanged();
}
