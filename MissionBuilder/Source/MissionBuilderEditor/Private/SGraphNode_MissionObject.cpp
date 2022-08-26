// Copyright Epic Games, Inc. All Rights Reserved.


#include "SGraphNode_MissionObject.h"
#include "Types/SlateStructs.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
//#include "MissionObject/BTNode.h"
//#include "MissionObject/BTCompositeNode.h"
//#include "MissionObjectGraph.h"
//#include "MissionObjectGraphNode.h"
//#include "MissionObjectGraphNode_Composite.h"
//#include "MissionObjectGraphNode_CompositeDecorator.h"
//#include "MissionObjectGraphNode_Decorator.h"
//#include "MissionObjectGraphNode_Root.h"
//#include "MissionObjectGraphNode_Service.h"
//#include "MissionObjectGraphNode_Task.h"
#include "MOGraphNode_Root.h"
#include "MOGraphNode_Task.h"
#include "MOGraph.h"
#include "MOGraphNode.h"

#include "Editor.h"
//#include "MissionObjectDebugger.h"
#include "GraphEditorSettings.h"
#include "SGraphPanel.h"
#include "SCommentBubble.h"
#include "SGraphPreviewer.h"
#include "NodeFactory.h"
#include "MissionBuilderColors.h"
#include "MissionObject.h"
#include "TaskObject.h"
//#include "MissionObject/Tasks/BTTask_RunBehavior.h"
#include "IDocumentation.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SLevelOfDetailBranchNode.h"

#define LOCTEXT_NAMESPACE "MissionObjectEditor"

namespace
{
	static const bool bShowExecutionIndexInEditorMode = true;
}

/////////////////////////////////////////////////////
// SMissionObjectPin

class SMissionObjectPin : public SGraphPinMO
{
public:
	SLATE_BEGIN_ARGS(SMissionObjectPin){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	/** @return The color that we should use to draw this pin */
	virtual FSlateColor GetPinColor() const override;
};

void SMissionObjectPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPinMO::Construct(SGraphPinMO::FArguments(), InPin);
}

FSlateColor SMissionObjectPin::GetPinColor() const
{
	return 
		GraphPinObj->bIsDiffing ? MissionBuilderColors::Pin::Diff :
		IsHovered() ? MissionBuilderColors::Pin::Hover :
		(GraphPinObj->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleComposite) ? MissionBuilderColors::Pin::CompositeOnly :
		(GraphPinObj->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleTask) ? MissionBuilderColors::Pin::TaskOnly :
		(GraphPinObj->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleNode) ? MissionBuilderColors::Pin::SingleNode :
		MissionBuilderColors::Pin::Default;
}

/** Widget for overlaying an execution-order index onto a node */
class SMissionObjectIndex : public SCompoundWidget
{
public:
	/** Delegate event fired when the hover state of this widget changes */
	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetIndexColor, bool /* bHovered */);

	SLATE_BEGIN_ARGS(SMissionObjectIndex){}
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
		SLATE_EVENT(FOnGetIndexColor, OnGetIndexColor)
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs )
	{
		OnHoverStateChangedEvent = InArgs._OnHoverStateChanged;
		OnGetIndexColorEvent = InArgs._OnGetIndexColor;

		const FSlateBrush* IndexBrush = FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Index"));

		ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				// Add a dummy box here to make sure the widget doesnt get smaller than the brush
				SNew(SBox)
				.WidthOverride(IndexBrush->ImageSize.X)
				.HeightOverride(IndexBrush->ImageSize.Y)
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(IndexBrush)
				.BorderBackgroundColor(this, &SMissionObjectIndex::GetColor)
				.Padding(FMargin(4.0f, 0.0f, 4.0f, 1.0f))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(InArgs._Text)
					.Font(FEditorStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
				]
			]
		];
	}

	virtual void OnMouseEnter( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(true);
		SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	}

	virtual void OnMouseLeave( const FPointerEvent& MouseEvent ) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(false);
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}

	/** Get the color we use to display the rounded border */
	FSlateColor GetColor() const
	{
		if(OnGetIndexColorEvent.IsBound())
		{
			return OnGetIndexColorEvent.Execute(IsHovered());
		}

		return FSlateColor::UseForeground();
	}

private:
	/** Delegate event fired when the hover state of this widget changes */
	FOnHoverStateChanged OnHoverStateChangedEvent;

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	FOnGetIndexColor OnGetIndexColorEvent;
};

/////////////////////////////////////////////////////
// SGraphNode_MissionObject

void SGraphNode_MissionObject::Construct(const FArguments& InArgs, UMOGraphNode* InNode)
{
	DebuggerStateDuration = 0.0f;
	DebuggerStateCounter = INDEX_NONE;
	bSuppressDebuggerTriggers = false;

	SGraphNodeMO::Construct(SGraphNodeMO::FArguments(), InNode);
}

void SGraphNode_MissionObject::AddDecorator(TSharedPtr<SGraphNode> DecoratorWidget)
{
	DecoratorsBox->AddSlot().AutoHeight()
	[
		DecoratorWidget.ToSharedRef()
	];

	DecoratorWidgets.Add(DecoratorWidget);
	AddSubNode(DecoratorWidget);
}

void SGraphNode_MissionObject::AddService(TSharedPtr<SGraphNode> ServiceWidget)
{
	ServicesBox->AddSlot().AutoHeight()
	[
		ServiceWidget.ToSharedRef()
	];
	ServicesWidgets.Add(ServiceWidget);
	AddSubNode(ServiceWidget);
}

FSlateColor SGraphNode_MissionObject::GetBorderBackgroundColor() const
{
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	UMOGraphNode* BTParentNode = MOGraphNode ? Cast<UMOGraphNode>(MOGraphNode->ParentNode) : nullptr;
	const bool bIsInDebuggerActiveState = false;//MOGraphNode && MOGraphNode->bDebuggerMarkCurrentlyActive;
	const bool bIsInDebuggerPrevState = false;// MOGraphNode&& MOGraphNode->bDebuggerMarkPreviouslyActive;
	const bool bSelectedSubNode = BTParentNode && GetOwnerPanel()->SelectionManager.SelectedNodes.Contains(GraphNode);
	
	UTaskObject* NodeInstance = MOGraphNode ? Cast<UTaskObject>(MOGraphNode->NodeInstance) : NULL;
	const bool bIsConnectedTreeRoot = MOGraphNode && MOGraphNode->IsA<UMOGraphNode_Root>() && MOGraphNode->Pins.IsValidIndex(0) && MOGraphNode->Pins[0]->LinkedTo.Num() > 0;
	const bool bIsDisconnected = NodeInstance && NodeInstance->TaskID == -1;
	const bool bIsService = false;// MOGraphNode&& MOGraphNode->IsA(UMOGraphNode_Service::StaticClass());
	const bool bIsRootDecorator = false;//MOGraphNode && MOGraphNode->bRootLevel;
	const bool bIsInjected = false;// MOGraphNode&& MOGraphNode->bInjectedNode;
	const bool bIsBrokenWithParent = false;/* bIsService ?
		BTParentNode && BTParentNode->Services.Find(MOGraphNode) == INDEX_NONE ? true : false :
		BTParentNode && BTParentNode->Decorators.Find(MOGraphNode) == INDEX_NONE ? true :
		(MOGraphNode && MOGraphNode->NodeInstance != NULL && (Cast<UBTNode>(MOGraphNode->NodeInstance->GetOuter()) == NULL && Cast<UMissionObject>(MOGraphNode->NodeInstance->GetOuter()) == NULL)) ? true : false;*/

		/*if (FMissionObjectDebugger::IsPIENotSimulating() && MOGraphNode)
		{
			if (MOGraphNode->bHighlightInAbortRange0)
			{
				return MissionBuilderColors::NodeBorder::HighlightAbortRange0;
			}
			else if (MOGraphNode->bHighlightInAbortRange1)
			{
				return MissionBuilderColors::NodeBorder::HighlightAbortRange1;
			}
			else if (MOGraphNode->bHighlightInSearchTree)
			{
				return MissionBuilderColors::NodeBorder::QuickFind;
			}
		}*/

	return bSelectedSubNode ? MissionBuilderColors::NodeBorder::Selected : 
		!bIsRootDecorator && !bIsInjected && bIsBrokenWithParent ? MissionBuilderColors::NodeBorder::BrokenWithParent :
		!bIsRootDecorator && !bIsInjected && bIsDisconnected ? MissionBuilderColors::NodeBorder::Disconnected :
		bIsInDebuggerActiveState ? MissionBuilderColors::NodeBorder::ActiveDebugging :
		bIsInDebuggerPrevState ? MissionBuilderColors::NodeBorder::InactiveDebugging :
		bIsConnectedTreeRoot ? MissionBuilderColors::NodeBorder::Root :
		MissionBuilderColors::NodeBorder::Inactive;
}

FSlateColor SGraphNode_MissionObject::GetBackgroundColor() const
{
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	//UMOGraphNode_Decorator* BTGraph_Decorator = Cast<UMOGraphNode_Decorator>(GraphNode);
	const bool bIsActiveForDebugger = false;/* MOGraphNode ?
		!bSuppressDebuggerColor && (MOGraphNode->bDebuggerMarkCurrentlyActive || MOGraphNode->bDebuggerMarkPreviouslyActive) :
		false;*/

	FLinearColor NodeColor = MissionBuilderColors::NodeBody::Default;
	if (MOGraphNode && MOGraphNode->HasErrors())
	{
		NodeColor = MissionBuilderColors::NodeBody::Error;
	}
	//else if (MOGraphNode && MOGraphNode->bInjectedNode)
	//{
	//	NodeColor = bIsActiveForDebugger ? MissionBuilderColors::Debugger::ActiveDecorator : MissionBuilderColors::NodeBody::InjectedSubNode;
	//}
	//else if (BTGraph_Decorator || Cast<UMOGraphNode_CompositeDecorator>(GraphNode))
	//{
	//	check(MOGraphNode);
	//	NodeColor = bIsActiveForDebugger ? MissionBuilderColors::Debugger::ActiveDecorator : 
	//		MOGraphNode->bRootLevel ? MissionBuilderColors::NodeBody::InjectedSubNode : MissionBuilderColors::NodeBody::Decorator;
	//}
	else if (Cast<UMOGraphNode_Task>(GraphNode))
	{
		check(MOGraphNode);
		
		if (UTaskObject* Task = Cast<UTaskObject>(MOGraphNode->NodeInstance))
		{
			// 从Task配置中获取颜色
			NodeColor = Task->NodeColor;
		}
		else
		{
			const bool bIsSpecialTask = false;// <UBTTask_RunBehavior>(MOGraphNode->NodeInstance) != nullptr;
			NodeColor = bIsSpecialTask ? MissionBuilderColors::NodeBody::TaskSpecial : MissionBuilderColors::NodeBody::Task;
		}
	}
	//else if (Cast<UMOGraphNode_Composite>(GraphNode))
	//{
	//	check(MOGraphNode);
	//	UBTCompositeNode* CompositeNodeInstance = Cast<UBTCompositeNode>(MOGraphNode->NodeInstance);
	//	const bool bIsScoped = CompositeNodeInstance && CompositeNodeInstance->IsApplyingDecoratorScope();
	//	NodeColor = bIsScoped ? MissionBuilderColors::NodeBody::CompositeScoped : MissionBuilderColors::NodeBody::Composite;
	//}
	//else if (Cast<UMOGraphNode_Service>(GraphNode))
	//{
	//	NodeColor = bIsActiveForDebugger ? MissionBuilderColors::Debugger::ActiveService : MissionBuilderColors::NodeBody::Service;
	//}
	else if (Cast<UMOGraphNode_Root>(GraphNode) && GraphNode->Pins.IsValidIndex(0) && GraphNode->Pins[0]->LinkedTo.Num() > 0)
	{
		NodeColor = MissionBuilderColors::NodeBody::Root;
	}

	return (FlashAlpha > 0.0f) ? FMath::Lerp(NodeColor, FlashColor, FlashAlpha) : NodeColor;
}

void SGraphNode_MissionObject::UpdateGraphNode()
{
	bDragMarkerVisible = false;
	InputPins.Empty();
	OutputPins.Empty();

	if (DecoratorsBox.IsValid())
	{
		DecoratorsBox->ClearChildren();
	} 
	else
	{
		SAssignNew(DecoratorsBox,SVerticalBox);
	}

	if (ServicesBox.IsValid())
	{
		ServicesBox->ClearChildren();
	}
	else
	{
		SAssignNew(ServicesBox,SVerticalBox);
	}

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	DecoratorWidgets.Reset();
	ServicesWidgets.Reset();
	SubNodes.Reset();
	OutputPinBox.Reset();

	UMOGraphNode* MONode = Cast<UMOGraphNode>(GraphNode);

	//if (MONode)
	//{
	//	for (int32 i = 0; i < MONode->Decorators.Num(); i++)
	//	{
	//		if (MONode->Decorators[i])
	//		{
	//			TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(MONode->Decorators[i]);
	//			if (OwnerGraphPanelPtr.IsValid())
	//			{
	//				NewNode->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
	//				OwnerGraphPanelPtr.Pin()->AttachGraphEvents(NewNode);
	//			}
	//			AddDecorator(NewNode);
	//			NewNode->UpdateGraphNode();
	//		}
	//	}

	//	for (int32 i = 0; i < MONode->Services.Num(); i++)
	//	{
	//		if (MONode->Services[i])
	//		{
	//			TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(MONode->Services[i]);
	//			if (OwnerGraphPanelPtr.IsValid())
	//			{
	//				NewNode->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
	//				OwnerGraphPanelPtr.Pin()->AttachGraphEvents(NewNode);
	//			}
	//			AddService(NewNode);
	//			NewNode->UpdateGraphNode();
	//		}
	//	}
	//}

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<STextBlock> DescriptionText; 
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	/*弱指针存放内存待销毁*/
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;
	/*lab捕获*/
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
		return FMath::Max(75.0f, DesiredWidth);
	};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
		return FMath::Max(22.0f, DesiredHeight);
	};

	const FMargin NodePadding = /*(Cast<UMOGraphNode_Decorator>(GraphNode) || Cast<UMOGraphNode_CompositeDecorator>(GraphNode) || Cast<UMOGraphNode_Service>(GraphNode))
		? FMargin(2.0f)
		: */FMargin(8.0f);

	IndexOverlay = SNew(SMissionObjectIndex)
		.ToolTipText(this, &SGraphNode_MissionObject::GetIndexTooltipText)
		.Visibility(this, &SGraphNode_MissionObject::GetIndexVisibility)
		.Text(this, &SGraphNode_MissionObject::GetIndexText)
		.OnHoverStateChanged(this, &SGraphNode_MissionObject::OnIndexHoverStateChanged)
		.OnGetIndexColor(this, &SGraphNode_MissionObject::GetIndexColor);

	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );
	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage( FEditorStyle::GetBrush( "Graph.StateNode.Body" ) )
			.Padding(0.0f)
			.BorderBackgroundColor( this, &SGraphNode_MissionObject::GetBorderBackgroundColor )
			.OnMouseButtonDown(this, &SGraphNode_MissionObject::OnMouseDown)
			[
				SNew(SOverlay)

				// Pins and node details
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					// INPUT PIN AREA
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Top)
						[
							SAssignNew(LeftNodeBox, SVerticalBox)
						]
					]

					// STATE NAME AREA
					+SVerticalBox::Slot()
					.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							DecoratorsBox.ToSharedRef()
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(NodeBody, SBorder)
							.BorderImage( FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body") )
							.BorderBackgroundColor( this, &SGraphNode_MissionObject::GetBackgroundColor )
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							.Visibility(EVisibility::SelfHitTestInvisible)
							[
								SNew(SOverlay)
								+SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											// POPUP ERROR MESSAGE
											SAssignNew(ErrorText, SErrorText )
											.BackgroundColor( this, &SGraphNode_MissionObject::GetErrorColor )
											.ToolTipText( this, &SGraphNode_MissionObject::GetErrorMsgToolTip )
										]
										
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SLevelOfDetailBranchNode)
											.UseLowDetailSlot(this, &SGraphNode_MissionObject::UseLowDetailNodeTitles)
											.LowDetail()
											[
												SNew(SBox)
												.WidthOverride_Lambda(GetNodeTitlePlaceholderWidth)
												.HeightOverride_Lambda(GetNodeTitlePlaceholderHeight)
											]
											.HighDetail()
											[
												SNew(SHorizontalBox)
												+SHorizontalBox::Slot()
												.AutoWidth()
												.VAlign(VAlign_Center)
												[
													SNew(SImage)
													.Image(this, &SGraphNode_MissionObject::GetNameIcon)
												]
												+SHorizontalBox::Slot()
												.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
												[
													SNew(SVerticalBox)
													+SVerticalBox::Slot()
													.AutoHeight()
													[
														SAssignNew(InlineEditableText, SInlineEditableTextBlock)
														.Style( FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText" )
														.Text( NodeTitle.Get(), &SNodeTitle::GetHeadTitle )
														.OnVerifyTextChanged(this, &SGraphNode_MissionObject::OnVerifyNameTextChanged)
														.OnTextCommitted(this, &SGraphNode_MissionObject::OnNameTextCommited)
														.IsReadOnly( this, &SGraphNode_MissionObject::IsNameReadOnly )
														.IsSelected(this, &SGraphNode_MissionObject::IsSelectedExclusively)
													]
													+SVerticalBox::Slot()
													.AutoHeight()
													[
														NodeTitle.ToSharedRef()
													]
												]
											]
										]
									]
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										// DESCRIPTION MESSAGE
										SAssignNew(DescriptionText, STextBlock )
										.Visibility(this, &SGraphNode_MissionObject::GetDescriptionVisibility)
										.Text(this, &SGraphNode_MissionObject::GetDescription)
									]
								]
								+SOverlay::Slot()
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Fill)
								[
									SNew(SBorder)
									.BorderImage( FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body") )
									.BorderBackgroundColor(MissionBuilderColors::Debugger::SearchFailed)
									.Padding(FMargin(4.0f, 0.0f))
									.Visibility(this, &SGraphNode_MissionObject::GetDebuggerSearchFailedMarkerVisibility)
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(10.0f,0,0,0))
						[
							ServicesBox.ToSharedRef()
						]
					]

					// OUTPUT PIN AREA
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Bottom)
						[
							SAssignNew(RightNodeBox, SVerticalBox)
							+SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(20.0f,0.0f)
							.FillHeight(1.0f)
							[
								SAssignNew(OutputPinBox, SHorizontalBox)
							]
						]
					]
				]

				// Drag marker overlay
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SBorder)
					.BorderBackgroundColor(MissionBuilderColors::Action::DragMarker)
					.ColorAndOpacity(MissionBuilderColors::Action::DragMarker)
					.BorderImage(FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
					.Visibility(this, &SGraphNode_MissionObject::GetDragOverMarkerVisibility)
					[
						SNew(SBox)
						.HeightOverride(4)
					]
				]

				// Blueprint indicator overlay
				+SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Blueprint")))
					.Visibility(this, &SGraphNode_MissionObject::GetBlueprintIconVisibility)
				]
			]
		];
	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew( CommentBubble, SCommentBubble )
	.GraphNode( GraphNode )
	.Text( this, &SGraphNode::GetNodeComment )
	.OnTextCommitted( this, &SGraphNode::OnCommentTextCommitted )
	.ColorAndOpacity( CommentColor )
	.AllowPinning( true )
	.EnableTitleBarBubble( true )
	.EnableBubbleCtrls( true )
	.GraphLOD( this, &SGraphNode::GetCurrentLOD )
	.IsGraphNodeHovered( this, &SGraphNode::IsHovered );

	GetOrAddSlot( ENodeZone::TopCenter )
	.SlotOffset( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetOffset ))
	.SlotSize( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetSize ))
	.AllowScaling( TAttribute<bool>( CommentBubble.Get(), &SCommentBubble::IsScalingAllowed ))
	.VAlign( VAlign_Top )
	[
		CommentBubble.ToSharedRef()
	];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

EVisibility SGraphNode_MissionObject::GetDebuggerSearchFailedMarkerVisibility() const
{
	//UMOGraphNode_Decorator* MyNode = Cast<UMOGraphNode_Decorator>(GraphNode);
	return/* MyNode && MyNode->bDebuggerMarkSearchFailed ? EVisibility::HitTestInvisible :*/ EVisibility::Collapsed;
}

void SGraphNode_MissionObject::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SGraphNode::Tick( AllottedGeometry, InCurrentTime, InDeltaTime );
	CachedPosition = AllottedGeometry.AbsolutePosition / AllottedGeometry.Scale;

	UMOGraphNode* MyNode = Cast<UMOGraphNode>(GraphNode);
	//if (MyNode && MyNode->DebuggerUpdateCounter != DebuggerStateCounter)
	//{
	//	DebuggerStateCounter = MyNode->DebuggerUpdateCounter;
	//	DebuggerStateDuration = 0.0f;
	//	bSuppressDebuggerColor = false;
	//	bSuppressDebuggerTriggers = false;
	//}

	DebuggerStateDuration += InDeltaTime;

	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	float NewFlashAlpha = 0.0f;
	TriggerOffsets.Reset();

	//if (MOGraphNode && FMissionObjectDebugger::IsPlaySessionPaused())
	//{
	//	const float SearchPathDelay = 0.5f;
	//	const float SearchPathBlink = 1.0f;
	//	const float SearchPathBlinkFreq = 10.0f;
	//	const float SearchPathKeepTime = 2.0f;
	//	const float ActiveFlashDuration = 0.2f;

	//	const bool bHasResult = MOGraphNode->bDebuggerMarkSearchSucceeded || MOGraphNode->bDebuggerMarkSearchFailed;
	//	const bool bHasTriggers = !bSuppressDebuggerTriggers && (MOGraphNode->bDebuggerMarkSearchTrigger || MOGraphNode->bDebuggerMarkSearchFailedTrigger);
	//	if (bHasResult || bHasTriggers)
	//	{
	//		const float FlashStartTime = MOGraphNode->DebuggerSearchPathIndex * SearchPathDelay;
	//		const float FlashStopTime = (MOGraphNode->DebuggerSearchPathSize * SearchPathDelay) + SearchPathKeepTime;
	//		
	//		UMOGraphNode_Decorator* BTGraph_Decorator = Cast<UMOGraphNode_Decorator>(GraphNode);
	//		UMOGraphNode_CompositeDecorator* BTGraph_CompDecorator = Cast<UMOGraphNode_CompositeDecorator>(GraphNode);

	//		bSuppressDebuggerColor = (DebuggerStateDuration < FlashStopTime);
	//		if (bSuppressDebuggerColor)
	//		{
	//			if (bHasResult && (BTGraph_Decorator || BTGraph_CompDecorator))
	//			{
	//				NewFlashAlpha =
	//					(DebuggerStateDuration > FlashStartTime + SearchPathBlink) ? 1.0f :
	//					(FMath::TruncToInt(DebuggerStateDuration * SearchPathBlinkFreq) % 2) ? 1.0f : 0.0f;
	//			} 
	//		}

	//		FlashColor = MOGraphNode->bDebuggerMarkSearchSucceeded ?
	//			MissionBuilderColors::Debugger::SearchSucceeded :
	//			MissionBuilderColors::Debugger::SearchFailed;
	//	}
	//	else if (MOGraphNode->bDebuggerMarkFlashActive)
	//	{
	//		NewFlashAlpha = (DebuggerStateDuration < ActiveFlashDuration) ?
	//			FMath::Square(1.0f - (DebuggerStateDuration / ActiveFlashDuration)) : 
	//			0.0f;

	//		FlashColor = MissionBuilderColors::Debugger::TaskFlash;
	//	}

	//	if (bHasTriggers)
	//	{
	//		// find decorator that caused restart
	//		for (int32 i = 0; i < DecoratorWidgets.Num(); i++)
	//		{
	//			if (DecoratorWidgets[i].IsValid())
	//			{
	//				SGraphNode_MissionObject* TestSNode = (SGraphNode_MissionObject*)DecoratorWidgets[i].Get();
	//				UMOGraphNode* ChildNode = Cast<UMOGraphNode>(TestSNode->GraphNode);
	//				if (ChildNode && (ChildNode->bDebuggerMarkSearchFailedTrigger || ChildNode->bDebuggerMarkSearchTrigger))
	//				{
	//					TriggerOffsets.Add(FNodeBounds(TestSNode->GetCachedPosition() - CachedPosition, TestSNode->GetDesiredSize()));
	//				}
	//			}
	//		}

	//		// when it wasn't any of them, add node itself to triggers (e.g. parallel's main task)
	//		if (DecoratorWidgets.Num() == 0)
	//		{
	//			TriggerOffsets.Add(FNodeBounds(FVector2D(0,0),GetDesiredSize()));
	//		}
	//	}
	//}
	FlashAlpha = NewFlashAlpha;
}

FReply SGraphNode_MissionObject::OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent )
{
	return SGraphNode::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent );
}

FText SGraphNode_MissionObject::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	FText HoverText = FText::GetEmpty();

	check(GraphPinObj != nullptr);
	UEdGraphNode* OwningGraphNode = GraphPinObj->GetOwningNode();
	if (/*OwningGraphNode != nullptr && */OwningGraphNode->IsValidLowLevel())
	{
		FString HoverStr;
		OwningGraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
		if (!HoverStr.IsEmpty())
		{
			HoverText = FText::FromString(HoverStr);
		}
	}

	return HoverText;
}

void SGraphNode_MissionObject::CreatePinWidgets()
{
	UMOGraphNode* StateNode = CastChecked<UMOGraphNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SMissionObjectPin, MyPin)
				.ToolTipText( this, &SGraphNode_MissionObject::GetPinTooltip, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_MissionObject::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner( SharedThis(this) );

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(20.0f,0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		const bool bIsSingleTaskPin = PinObj && (PinObj->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleTask);
		if (bIsSingleTaskPin)
		{
			OutputPinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.4f)
			.Padding(0,0,20.0f,0)
			[
				PinToAdd
			];
		}
		else
		{
			OutputPinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			[
				PinToAdd
			];
		}
		OutputPins.Add(PinToAdd);
	}
}

TSharedPtr<SToolTip> SGraphNode_MissionObject::GetComplexTooltip()
{
	//UMOGraphNode_CompositeDecorator* DecoratorNode = Cast<UMOGraphNode_CompositeDecorator>(GraphNode);
	//if (DecoratorNode && DecoratorNode->GetBoundGraph())
	//{
	//	return SNew(SToolTip)
	//		[
	//			SNew(SOverlay)
	//			+SOverlay::Slot()
	//			[
	//				// Create the tooltip graph preview, make sure to disable state overlays to
	//				// prevent the PIE / read-only borders from obscuring the graph
	//				SNew(SGraphPreviewer, DecoratorNode->GetBoundGraph())
	//				.CornerOverlayText(LOCTEXT("CompositeDecoratorOverlayText", "Composite Decorator"))
	//				.ShowGraphStateOverlay(false)
	//			]
	//			+SOverlay::Slot()
	//			.Padding(2.0f)
	//			[
	//				SNew(STextBlock)
	//				.Text(LOCTEXT("CompositeDecoratorTooltip", "Double-click to Open"))
	//				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
	//			]
	//		];
	//}

	//UMOGraphNode_Task* TaskNode = Cast<UMOGraphNode_Task>(GraphNode);
	//if(TaskNode && TaskNode->NodeInstance)
	//{
	//	UBTTask_RunBehavior* RunBehavior = Cast<UBTTask_RunBehavior>(TaskNode->NodeInstance);
	//	if(RunBehavior && RunBehavior->GetSubtreeAsset() && RunBehavior->GetSubtreeAsset()->BTGraph)
	//	{
	//		return SNew(SToolTip)
	//			[
	//				SNew(SOverlay)
	//				+SOverlay::Slot()
	//				[
	//					// Create the tooltip graph preview, make sure to disable state overlays to
	//					// prevent the PIE / read-only borders from obscuring the graph
	//					SNew(SGraphPreviewer, RunBehavior->GetSubtreeAsset()->BTGraph)
	//					.CornerOverlayText(LOCTEXT("RunBehaviorOverlayText", "Run Behavior"))
	//					.ShowGraphStateOverlay(false)
	//				]
	//				+SOverlay::Slot()
	//				.Padding(2.0f)
	//				[
	//					SNew(STextBlock)
	//					.Text(LOCTEXT("RunBehaviorTooltip", "Double-click to Open"))
	//					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
	//				]
	//			];
	//	}
	//}

	return IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
}

const FSlateBrush* SGraphNode_MissionObject::GetNameIcon() const
{	
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	return MOGraphNode != nullptr ? FEditorStyle::GetBrush(MOGraphNode->GetNameIcon()) : FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

static UMOGraphNode* GetParentNode(UEdGraphNode* GraphNode)
{
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	if (MOGraphNode->ParentNode != nullptr)
	{
		MOGraphNode = Cast<UMOGraphNode>(MOGraphNode->ParentNode);
	}

	UEdGraphPin* MyInputPin = MOGraphNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = nullptr;
	if (MyInputPin != nullptr && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
		if(MyParentOutputPin != nullptr)
		{
			if(MyParentOutputPin->GetOwningNode() != nullptr)
			{
				return CastChecked<UMOGraphNode>(MyParentOutputPin->GetOwningNode());
			}
		}
	}

	return nullptr;
}

void SGraphNode_MissionObject::OnIndexHoverStateChanged(bool bHovered)
{
	UMOGraphNode* ParentNode = GetParentNode(GraphNode);
	if (ParentNode != nullptr)
	{
		ParentNode->bHighlightChildNodeIndices = bHovered;
	}
}

FSlateColor SGraphNode_MissionObject::GetIndexColor(bool bHovered) const
{
	UMOGraphNode* ParentNode = GetParentNode(GraphNode);
	const bool bHighlightHover = bHovered || (ParentNode && ParentNode->bHighlightChildNodeIndices);

	static const FName HoveredColor("BTEditor.Graph.BTNode.Index.HoveredColor");
	static const FName DefaultColor("BTEditor.Graph.BTNode.Index.Color");

	return bHighlightHover ? FEditorStyle::Get().GetSlateColor(HoveredColor) : FEditorStyle::Get().GetSlateColor(DefaultColor);
}

EVisibility SGraphNode_MissionObject::GetIndexVisibility() const
{
	// always hide the index on the root node
	if(GraphNode->IsA(UMOGraphNode_Root::StaticClass()))
	{
		return EVisibility::Collapsed;
	}

	UMOGraphNode* StateNode = CastChecked<UMOGraphNode>(GraphNode);
	UEdGraphPin* MyInputPin = StateNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	// Visible if we are in PIE or if we have siblings
	CA_SUPPRESS(6235);
	const bool bCanShowIndex = (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL) || (MyParentOutputPin && MyParentOutputPin->LinkedTo.Num() > 1);

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIndex && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNode_MissionObject::GetIndexText() const
{
	UMOGraphNode* StateNode = CastChecked<UMOGraphNode>(GraphNode);
	UEdGraphPin* MyInputPin = StateNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	int32 Index = 0;

	CA_SUPPRESS(6235);
	if (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL)
	{
		// special case: range of execution indices in composite decorator node
		//UMOGraphNode_CompositeDecorator* CompDecorator = Cast<UMOGraphNode_CompositeDecorator>(GraphNode);
		//if (CompDecorator && CompDecorator->FirstExecutionIndex != CompDecorator->LastExecutionIndex)
		//{
		//	return FText::Format(LOCTEXT("CompositeDecoratorFormat", "{0}..{1}"), FText::AsNumber(CompDecorator->FirstExecutionIndex), FText::AsNumber(CompDecorator->LastExecutionIndex));
		//}

		// show execution index (debugging purposes)
		UTaskObject* MONode = Cast<UTaskObject>(StateNode->NodeInstance);
		Index = (MONode && MONode->TaskID < 0xffff) ? MONode->TaskID : -1;
	}
	else
	{
		// show child index
		if (MyParentOutputPin != NULL)
		{
			for (Index = 0; Index < MyParentOutputPin->LinkedTo.Num(); ++Index)
			{
				if (MyParentOutputPin->LinkedTo[Index] == MyInputPin)
				{
					break;
				}
			}
		}
	}

	return FText::AsNumber(Index);
}

FText SGraphNode_MissionObject::GetIndexTooltipText() const
{
	CA_SUPPRESS(6235);
	if (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL)
	{
		return LOCTEXT("ExecutionIndexTooltip", "Task ID: this is a unique id Of a Task.");
	}
	else
	{
		return LOCTEXT("ChildIndexTooltip", "Child index: this shows the order in which child nodes are executed.");
	}
}

EVisibility SGraphNode_MissionObject::GetBlueprintIconVisibility() const
{
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	const bool bCanShowIcon = (MOGraphNode != nullptr && MOGraphNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

void SGraphNode_MissionObject::GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const
{
	UMOGraphNode* MONode = Cast<UMOGraphNode>(GraphNode);
	if (MONode == NULL)
	{
		return;
	}

	/*if (MONode->bHasBreakpoint)
	{
		FOverlayBrushInfo BreakpointOverlayInfo;
		BreakpointOverlayInfo.Brush = MONode->bIsBreakpointEnabled ?
			FEditorStyle::GetBrush(TEXT("BTEditor.DebuggerOverlay.Breakpoint.Enabled")) :
			FEditorStyle::GetBrush(TEXT("BTEditor.DebuggerOverlay.Breakpoint.Disabled"));

		if (BreakpointOverlayInfo.Brush)
		{
			BreakpointOverlayInfo.OverlayOffset -= BreakpointOverlayInfo.Brush->ImageSize / 2.f;
		}

		Brushes.Add(BreakpointOverlayInfo);
	}

	if (FMissionObjectDebugger::IsPlaySessionPaused())
	{
		if (MONode->bDebuggerMarkBreakpointTrigger || (MONode->bDebuggerMarkCurrentlyActive && MONode->IsA(UMOGraphNode_Task::StaticClass())))
		{
			FOverlayBrushInfo IPOverlayInfo;

			IPOverlayInfo.Brush = MONode->bDebuggerMarkBreakpointTrigger ? FEditorStyle::GetBrush(TEXT("BTEditor.DebuggerOverlay.BreakOnBreakpointPointer")) : 
				FEditorStyle::GetBrush(TEXT("BTEditor.DebuggerOverlay.ActiveNodePointer"));
			if (IPOverlayInfo.Brush)
			{
				float Overlap = 10.f;
				IPOverlayInfo.OverlayOffset.X = (WidgetSize.X/2.f) - (IPOverlayInfo.Brush->ImageSize.X/2.f);
				IPOverlayInfo.OverlayOffset.Y = (Overlap - IPOverlayInfo.Brush->ImageSize.Y);
			}

			IPOverlayInfo.AnimationEnvelope = FVector2D(0.f, 10.f);
			Brushes.Add(IPOverlayInfo);
		}

		if (TriggerOffsets.Num())
		{
			FOverlayBrushInfo IPOverlayInfo;

			IPOverlayInfo.Brush = FEditorStyle::GetBrush(MONode->bDebuggerMarkSearchTrigger ?
				TEXT("BTEditor.DebuggerOverlay.SearchTriggerPointer") :
				TEXT("BTEditor.DebuggerOverlay.FailedTriggerPointer") );

			if (IPOverlayInfo.Brush)
			{
				for (int32 i = 0; i < TriggerOffsets.Num(); i++)
				{
					IPOverlayInfo.OverlayOffset.X = -IPOverlayInfo.Brush->ImageSize.X;
					IPOverlayInfo.OverlayOffset.Y = TriggerOffsets[i].Position.Y + TriggerOffsets[i].Size.Y / 2 - IPOverlayInfo.Brush->ImageSize.Y / 2;

					IPOverlayInfo.AnimationEnvelope = FVector2D(10.f, 0.f);
					Brushes.Add(IPOverlayInfo);
				}
			}
		}
	}*/
}

TArray<FOverlayWidgetInfo> SGraphNode_MissionObject::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	check(NodeBody.IsValid());
	check(IndexOverlay.IsValid());

	FVector2D Origin(0.0f, 0.0f);

	// build overlays for decorator sub-nodes
	for(const auto& DecoratorWidget : DecoratorWidgets)
	{
		TArray<FOverlayWidgetInfo> OverlayWidgets = DecoratorWidget->GetOverlayWidgets(bSelected, WidgetSize);
		for(auto& OverlayWidget : OverlayWidgets)
		{
			OverlayWidget.OverlayOffset.Y += Origin.Y;
		}
		Widgets.Append(OverlayWidgets);
		Origin.Y += DecoratorWidget->GetDesiredSize().Y;
	}

	FOverlayWidgetInfo Overlay(IndexOverlay);
	Overlay.OverlayOffset = FVector2D(WidgetSize.X - (IndexOverlay->GetDesiredSize().X * 0.5f), Origin.Y);
	Widgets.Add(Overlay);

	Origin.Y += NodeBody->GetDesiredSize().Y;

	// build overlays for service sub-nodes
	for(const auto& ServiceWidget : ServicesWidgets)
	{
		TArray<FOverlayWidgetInfo> OverlayWidgets = ServiceWidget->GetOverlayWidgets(bSelected, WidgetSize);
		for(auto& OverlayWidget : OverlayWidgets)
		{
			OverlayWidget.OverlayOffset.Y += Origin.Y;
		}
		Widgets.Append(OverlayWidgets);
		Origin.Y += ServiceWidget->GetDesiredSize().Y;
	}

	return Widgets;
}

TSharedRef<SGraphNode> SGraphNode_MissionObject::GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, MouseEvent);
	return SubNode.IsValid() ? SubNode.ToSharedRef() : StaticCastSharedRef<SGraphNode>(AsShared());
}

void SGraphNode_MissionObject::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter)
{
	SGraphNodeMO::MoveTo(NewPosition, NodeFilter);

	// keep node order (defined by linked pins) up to date with actual positions
	// this function will keep spamming on every mouse move update
	UMOGraphNode* MOGraphNode = Cast<UMOGraphNode>(GraphNode);
	if (MOGraphNode && !MOGraphNode->IsSubNode())
	{
		UMOGraph* MOGraph = MOGraphNode->GetMOGraph();
		if (MOGraph)
		{
			for (int32 Idx = 0; Idx < MOGraphNode->Pins.Num(); Idx++)
			{
				UEdGraphPin* Pin = MOGraphNode->Pins[Idx];
				if (Pin && Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 1) 
				{
					UEdGraphPin* ParentPin = Pin->LinkedTo[0];
					if (ParentPin)
					{
						;//MOGraph->RebuildChildOrder(ParentPin->GetOwningNode());
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
