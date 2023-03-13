// Copyright Epic Games, Inc. All Rights Reserved.

#include "ADTools.h"
#include "ADToolsStyle.h"
#include "ADToolsCommands.h"
#include "EditorStyleSet.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Internationalization/Culture.h"
#include "Kismet/KismetInternationalizationLibrary.h"

static const FName ADToolsTabName("ADTools");

#define LOCTEXT_NAMESPACE "FADToolsModule"

void FADToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FADToolsStyle::Initialize();
	FADToolsStyle::ReloadTextures();

	FADToolsCommands::Register();
	BindCommands();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FADToolsModule::RegisterMenus));
}

void FADToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FADToolsStyle::Shutdown();

	FADToolsCommands::Unregister();
}

void FADToolsModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FADToolsModule::PluginButtonClicked()")),
							FText::FromString(TEXT("ADTools.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}



void FADToolsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	auto OnGetToolbarButtonBrushLambda = [this]() -> const FSlateIcon
	{
		//根据状态,获取Icon
		return FSlateIcon(FADToolsStyle::GetStyleSetName(), "UnworldEditor.Unworld");
	};
	
	{
		//Window 菜单中添加按 ADTools
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FADToolsCommands::Get().PluginAction, ADToolsCommands);
		}
	}

	{
		//工具栏中添加按键.
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{

				Section.AddDynamicEntry("ADTools", FNewToolMenuSectionDelegate::CreateLambda([&](FToolMenuSection& InDynamicSection)
				{
					InDynamicSection.AddEntry(FToolMenuEntry::InitToolBarButton(
								"ADTools",
								FUIAction(FExecuteAction::CreateRaw(this,&FADToolsModule::OnToorBarButtonClick)),
								LOCTEXT("ADTools", "ADTools"),
								LOCTEXT("ADTools Tip", "ADTools Tip."),
								TAttribute<FSlateIcon>::Create(OnGetToolbarButtonBrushLambda)
					));


					//构建下拉框.
					InDynamicSection.AddEntry(FToolMenuEntry::InitComboButton(
						"ADTools ComboMenu",
						FUIAction(),
						FOnGetContent::CreateRaw(this,&FADToolsModule::GenerateComboMenu, ADToolsCommands),
						LOCTEXT("LaunchCombo_Label", "ADTools Options"),
						LOCTEXT("ComboToolTip", "Options for ADTools"),
						#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "ADTools.EditorTools"),
						#else
						FSlateIcon(FEditorStyle::GetStyleSetName(), "ADTools.EditorTools"),
						#endif
						true
					));
					
				}));
				
				//FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FADToolsCommands::Get().PluginAction));
				//Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FADToolsModule::BindCommands()
{
	check(!ADToolsCommands.IsValid());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	ADToolsCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	
	const FADToolsCommands& Commands = FADToolsCommands::Get();
	FUICommandList& ActionList = *ADToolsCommands;




	ActionList.MapAction(
			FADToolsCommands::Get().PluginAction,
			FExecuteAction::CreateRaw(this, &FADToolsModule::PluginButtonClicked),
			FCanExecuteAction());

	
	ActionList.MapAction(
		Commands.LangSwitcher,
		FExecuteAction::CreateRaw(this,&FADToolsModule::LangSwitcher),
		FCanExecuteAction());


	//重启按键
	ActionList.MapAction(
		Commands.ResetEditor,
		FExecuteAction::CreateRaw(this, &FADToolsModule::RestartEditor),
		FCanExecuteAction());

	//AddPIEPreviewDeviceActions(Commands, ActionList);
	
}

void FADToolsModule::LangSwitcher() const
{
	const bool isCN = FInternationalization::Get().GetCurrentLanguage()->GetName()=="zh-hans";
	
	if(isCN)
	{
		UKismetInternationalizationLibrary::SetCurrentLanguage("en",true);
	}
	else
	{
		UKismetInternationalizationLibrary::SetCurrentLanguage("zh-hans",true);
	}

	
}

void FADToolsModule::RestartEditor()
{
	FUnrealEdMisc::Get().RestartEditor((true));
}

TSharedRef<SWidget> FADToolsModule::GenerateComboMenu(TSharedPtr<FUICommandList> InCommands)
{
	//注册UWEditorButton
	UToolMenus::Get()->RegisterMenu("LevelEditor.LevelEditorToolBar.ADToolsButton");

	//构建 MenuBuilder
	FMenuBuilder MenuBuilder(true, InCommands);

	MenuBuilder.BeginSection("ADToolsMenu", TAttribute<FText>(FText::FromString("ADToolsMenu")));

	
	MenuBuilder.AddMenuEntry(FADToolsCommands::Get().LangSwitcher);
	MenuBuilder.AddMenuSeparator();
	MenuBuilder.AddMenuEntry(FADToolsCommands::Get().ResetEditor);

	MenuBuilder.EndSection();
	
	return MenuBuilder.MakeWidget();
}

void FADToolsModule::OnToorBarButtonClick() const
{
	// Put your "OnButtonClicked" stuff here
	const FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FADToolsModule::PluginButtonClicked()")),
							FText::FromString(TEXT("ADTools.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FADToolsModule, ADTools)