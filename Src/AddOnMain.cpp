#include "APIEnvir.h"
#include "ACAPinc.h"

#include "ResourceIds.hpp"
#include "DGModule.hpp"
#include "APIdefs_Elements.h"

#include "../../Build/WallDimension.h"

#include "../../Build/QtDimensionForm.h"
#include "../../Build/WalllWithLineDimenison.h"
#include "../../Build/BeamDimWithLine.h"
#include "../../Build/ColumnDimension.h"
#include "../../Build/ChangeIdRevit.h"
#include "../../Build/CreateCoppha.h"
#include "../../Build/CreateProfiles.h"
#include "../../Build/ModifyHotSpot.h"
#include "../../Build/ChangeProfileElement.h"
#include "../../Build/InputMashiAuto.h"
#include"../../Build/CreateLayerAuto.h"
#include"../../Build/ReadDataCad.h"
#include"../../Build/BeamCad.h"
#include"../../Build/WallCad.h"
#include"../../Build/ColumnCad.h"
#include "../../Build/CadSlab.h"
#include"../../Build/BeamCadForm.h"
#include "../../Build/CreateProfilesH.h"
#include "../../Build/ModifyHotSpotH.h"
#include "../../Build/ColumnCad.h"
#include "../../Build/CadSlab.h"
#include "../../Build/WallCad.h"
#include "../../Build/FitlerAndCutElement.h"
#include"../../Build/AlignElements.h"
#include "../../Build/ProfileRecSteelAtribute.h"
#include "../../Build/LayerListBox.h"
#include "../../Build/FormCheckBox.h"
#include "../../Build/CadZoneCreate.h"
#include "../../Build/ChangeProfilePrefixLevel.h"
#include "../../Build/SeperateWall.h"
#include "../../Build/CreaetMeshTopo.h"
#include "../../Build/EditMeshTopo.h"
#include "../../Build/ParametersSet.h"
#include "../../Build/AlignBeamToFloor.h"
#include "../../Build/WallCeiling.h"
#include "../../Build/SeparateMirror.h"
#include "../../Build/BaseWallBinding.h"
#include "../../Build/SelectEditElementBinding.h"
#include "../../Build/WallToFloorBinding.h"
#include "../../Build/PaintWallBinding.h"
#include "../../Build/OpeningWallBinding.h"
#include "../../Build/LevelExport.h"
#include "../../Build/LevelImport.h"
#include "../../Build/FireProtectBinding.h"
#include "../../Build/CurtainWallOpeningBinding.h"
#include "../../Build/FamilyByPointBinding.h"
#include "../../Build/PoinsToMeshBinding.h"
#include "../../Build/BeamToRoofBinding.h"
#include "../../Build/OffsetFromTopForm.h"
#include "../../Build/BeamBottomTopBinding.h"
#include "../../Build/BeamBottomBeamBinding.h"
#include "../../Build/RemoveIdSao.h"
//#include "../../Build/FromRollDown.h"

static const GSResID AddOnInfoID = ID_ADDON_INFO;
static const Int32 AddOnNameID = 1;
static const Int32 AddOnDescriptionID = 2;

static const short AddOnMenuID = ID_ADDON_MENU;
static const Int32 AddOnCommandID = 1;
static const Int32 AddOnCommandDImID = 2;
static const Int32 AddOnCommandDimWithLineID = 3;
static const Int32 AddOnCommandBeamWithLineID = 4;
static const Int32 AddOnComandBeamWithLineNotLayerID = 5;
static const Int32 AddOnCommandColumnGridNotLayerID = 6;


static const short PromptID = ID_PROMPT;

static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams)
{
	switch (menuParams->menuItemRef.menuResID)
	{
	case ID_ADDON_MENU:
		switch (menuParams->menuItemRef.itemIndex) {
		case AddOnCommandID:
		{
			ExampleDialog dialog;
			bool result = dialog.Invoke();
			if (result)
			{
				GS::UniString value = dialog.g_value;
				std::string mystr = value.ToCStr();
				SaveFile(mystr);
			}
			break;
		}
		case AddOnCommandDImID:
		{
			ACAPI_CallUndoableCommand("CreateDimWallCombine",
				[&]() -> GSErrCode {
					std::string mystr = readFile();
					double distance;
					try
					{
						distance = std::strtod(mystr.c_str(), NULL) / 1000;
					}
					catch (const std::exception&) { distance = 0.3; };
					CreateDimensionTotalWall();
					CreateDimensionCenterWall(distance);
					return CreateDimensionLayerWall(distance);
				});
			break;
		}

		case AddOnCommandDimWithLineID:
		{
			std::string mystr = readFile();
			double distance;
			try
			{
				distance = std::strtod(mystr.c_str(), NULL) / 1000;
			}
			catch (const std::exception&) { distance = 0.3; };
			ACAPI_CallUndoableCommand("CreateDimWallWithLine",
				[&]() -> GSErrCode {
					CreateDimensionTotalWallWithLine();
					return CreateDimensionWallWithLine(distance);
				});
			break;
		}

		case AddOnCommandBeamWithLineID:
		{
			std::string mystr = readFile();
			double distance;
			try
			{
				distance = std::strtod(mystr.c_str(), NULL) / 1000;
			}
			catch (const std::exception&) { distance = 0.3; };
			ACAPI_CallUndoableCommand("CreateDimBeamLayerWithLine",
				[&]() -> GSErrCode {
					CreateTotalDimBeam();
					CreateDimBeamToLine(distance);
					return CreateDimLayerBeam(distance);
				});
			break;
		}

		case AddOnComandBeamWithLineNotLayerID:
		{
			std::string mystr = readFile();
			double distance;
			try
			{
				distance = std::strtod(mystr.c_str(), NULL) / 1000;
			}
			catch (const std::exception&) { distance = 0.3; };
			ACAPI_CallUndoableCommand("CreateDimBeamNotLayerWithLine",
				[&]() -> GSErrCode {
					CreateTotalDimBeam();
					return CreateDimBeamToLine(distance);
				});
			break;
		}

		case AddOnCommandColumnGridNotLayerID:
		{
			std::string mystr = readFile();
			double distance;
			try
			{
				distance = std::strtod(mystr.c_str(), NULL) / 1000;
			}
			catch (const std::exception&) { distance = 0.3; };
			ACAPI_CallUndoableCommand("CreateDimColumnGrid",
				[&]() -> GSErrCode {
					return CreateDimensionWithGrid(distance);
				});
			break;
		}

		}
		break;

	case 32501:
		switch (menuParams->menuItemRef.itemIndex)
		{
		case 1:
			ACAPI_CallUndoableCommand("ChangeIdFromRevit",
				[&]() -> GSErrCode {
					return ChangeIdFromElement();
				});
			break;
		case 2:
			ACAPI_CallUndoableCommand("CreateCoppha",
				[&]() -> GSErrCode {
					return CreateCoppha();
				});
			break;
		case 3:
			ACAPI_CallUndoableCommand("CreateProfilesRC",
				[&]() -> GSErrCode {
					return CreateProfiles();
				});
			break;
		case 4:
			ACAPI_CallUndoableCommand("CreateProfilesH",
				[&]() -> GSErrCode {
					return CreateProfilesH();
				});
			break;
		case 5:
			ACAPI_CallUndoableCommand("CreateProfilesRecSteel",
				[&]() -> GSErrCode {
					return CreateProfileRecSteel();
				});
			break;
		case 6:
			ACAPI_CallUndoableCommand("ChangeProfileElements",
				[&]() -> GSErrCode {
					return ChangeProfileElement();
				});
			break;
		case 7:
			ACAPI_CallUndoableCommand("CreateLayerFromExcel",
				[&]() -> GSErrCode {
					return CreateLayerAuto();
				});
			break;
		case 8:
			return ShowFilterAndCut3d();
			break;
		case 9:
			ACAPI_CallUndoableCommand("Allign Element",
				[&]() -> GSErrCode {
					return AllignElements();
				});
			break;
		case 10:
			//Show_LayerDialog();
			Show_FontPopUpDialog();
			//Show_BasicElementIconsRollDownDialog();

			break;
		case 11:
			ACAPI_CallUndoableCommand("ChangeProfileWithLevel",
				[&]() -> GSErrCode
				{
					return ChangeProfileWithPrefixIdLevel();
				});
			break;
		case 12:
			SelectionEditElems();
			break;
		case 13:
			ExportLevelCsv();
			break;
		case 14:
			ACAPI_CallUndoableCommand("ImportLevelLink",
				[&]() -> GSErrCode
				{
					return ImportLevelFile();
				});
			break;
		case 15:
			ACAPI_CallUndoableCommand("CreateFireProtect",
				[&]() -> GSErrCode
				{
					return CreateFireProtect();
				});
			break;
		case 16:
			ACAPI_CallUndoableCommand("RemoveSaoOfId",
				[&]() -> GSErrCode
				{
					return RemoveSao();
				});
			break;
		case 17:
			ACAPI_CallUndoableCommand("AddSaoOfId",
				[&]() -> GSErrCode
				{
					return AddSao();
				});
			break;
		}
		break;

	case  32502:
		switch (menuParams->menuItemRef.itemIndex)
		{
		case 1:
		{
			BeamCadDialog dialog(ACAPI_GetOwnResModule());
			BeamObserver	observer(&dialog);
			GSErrCode err = NoError;
			err = GetInfromationCad(true, false, false, false, false);
			if (err != NoError) return err;
			bool result = dialog.Invoke();
			if (result) {
				ACAPI_CallUndoableCommand("CreateBeamCad",
					[&]() -> GSErrCode {
						return  CreateBeams();;
					});
			}
			break;
		}
		case 2:
		{
			BeamCadDialog dialog(ACAPI_GetOwnResModule());
			BeamObserver	observer(&dialog);
			GSErrCode err = NoError;
			err = GetInfromationCad(false, true, false, false, false);
			if (err != NoError) return err;
			bool result = dialog.Invoke();
			if (result) {
				ACAPI_CallUndoableCommand("CreateColumnCad",
					[&]() -> GSErrCode {
						return CreateCadColumns();
					});
			}
			break;
		}
		case 3:
		{
			BeamCadDialog dialog(ACAPI_GetOwnResModule());
			BeamObserver	observer(&dialog);
			GSErrCode err = NoError;
			err = GetInfromationCad(false, false, false, true, false);
			if (err != NoError) return err;
			bool result = dialog.Invoke();
			if (result) {
				ACAPI_CallUndoableCommand("CreateFloorFoundationCad",
					[&]() -> GSErrCode {
						return CreateCadSlabs();
					});
			}
			break;
		}
		case 4:
		{
			BeamCadDialog dialog(ACAPI_GetOwnResModule());
			BeamObserver	observer(&dialog);
			GSErrCode err = NoError;
			err = GetInfromationCad(false, false, true, false, false);
			if (err != NoError) return err;
			bool result = dialog.Invoke();
			if (result) {
				ACAPI_CallUndoableCommand("CreateWallCad",
					[&]() -> GSErrCode {
						return CreateCadWalls();
					});
			}
			break;
		}

		case 5:
		{
			BeamCadDialog dialog(ACAPI_GetOwnResModule());
			BeamObserver	observer(&dialog);
			GSErrCode err = NoError;
			err = GetInfromationCad(false, false, false, false, true);
			if (err != NoError) return err;
			bool result = dialog.Invoke();
			if (result) {
				ACAPI_CallUndoableCommand("CreateRoomCad",
					[&]() -> GSErrCode {
						return CreateCadZones();
					});
			}
			break;
		}

		case 6:
		{
			ACAPI_CallUndoableCommand("SeparateWallLayer",
				[&]() -> GSErrCode {
					return SeperateWallData();
				});
			break;
		}

		case 7:
		{
			ACAPI_CallUndoableCommand("MeshByPolyline",
				[&]() -> GSErrCode {
					return ModifyTopo();
				});
			break;
		}

		case 8:
		{
			ACAPI_CallUndoableCommand("EditMeshByPolyline",
				[&]() -> GSErrCode {
					return EditTopoByPolyline();
				});
			break;
		}

		case 9:
		{
			ACAPI_CallUndoableCommand("ParameterAreaLengthThickness",
				[&]() -> GSErrCode {
					return SetParameterToWallSlab();
				});
			break;
		}

		case 10:
		{
			ACAPI_CallUndoableCommand("AlignBealToFloor",
				[&]() -> GSErrCode {
					return AlignBeamFloor();
				});
			break;
		}
		case 11:
		{
			ACAPI_CallUndoableCommand("WallCeilingToCeling",
				[&]() -> GSErrCode {
					return CreateWallFromToCeiling();
				});
			break;
		}

		case 12:
		{
			ACAPI_CallUndoableCommand("MirrorWallSpe",
				[&]() -> GSErrCode {
					return SeparateMirrorWall();
				});
			break;
		}

		case 13:
		{
			ACAPI_CallUndoableCommand("CreateBaseWallByRoom",
				[&]() -> GSErrCode {
					return CreateWallBase();
				});
			break;
		}
		case 14:
		{
			ACAPI_CallUndoableCommand("WallToFloorCeilingBeam",
				[&]() -> GSErrCode {
					return AlignWallToFloorBeam();
				});
			break;
		}

		case 15:
		{
			ACAPI_CallUndoableCommand("CreateBaseWallByZone",
				[&]() -> GSErrCode {
					return CreateWallBaseByZone();
				});
			break;
		}

		case 16:
		{
			ACAPI_CallUndoableCommand("CreatePaintByHabaki",
				[&]() -> GSErrCode {
					return PaintWallByHabaki();
				});
			break;
		}
		case 17:
		{
			ACAPI_CallUndoableCommand("CreateOpeningByDoor",
				[&]() -> GSErrCode {
					return  CreateOpening();
				});
			break;
		}

		case 18:
		{
			ACAPI_CallUndoableCommand("CreateOpeningByCurtainWall",
				[&]() -> GSErrCode {
					return  CreateOpeningForCurtainWall();
				});
			break;
		}

		case 19:
		{
			ACAPI_CallUndoableCommand("CreateOpeningByWindow",
				[&]() -> GSErrCode {
					return  CreateOpeningWindow();
				});
			break;
		}

		case 20:
		{
			ACAPI_CallUndoableCommand("CreateFamilyByPoint",
				[&]() -> GSErrCode {
					return  PutFamilyByPoint();
				});
			break;
		}
		case 21:
		{
			ACAPI_CallUndoableCommand("AddPointsToMeshTopo",
				[&]() -> GSErrCode {
					return  AddPointToMesh();
				});
			break;
		}

		case 22:
		{
			ACAPI_CallUndoableCommand("BeamToRoof",
				[&]() -> GSErrCode {
					return  AlignBeamToRoof();
				});
			break;
		}

		case 23:
		{
			OffsetFromTopForm dialog;
			bool result = dialog.Invoke();
			if (result)
			{
				GS::UniString value = dialog.g_value;
				std::string mystr = value.ToCStr();
				FileTextSave(mystr);
			}
			break;
		}
		case 24:
		{
			ACAPI_CallUndoableCommand("OffsetBeamBottomTop",
				[&]() -> GSErrCode {
					std::string mystr = ReadFileText();
					double distance;
					try
					{
						distance = std::strtod(mystr.c_str(), NULL) / 1000;
					}
					catch (const std::exception&) { distance = 0.1; };
					return ChangeElevationBeam(distance);
				});
			break;
		}

		case 25:
		{
			ACAPI_CallUndoableCommand("BeamToBottomBeam",
				[&]() -> GSErrCode {
					return  AlignBeamToBottom();
				});
			break;
		}




		}
		break;
	}
	return NoError;
}

API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}

GSErrCode RegisterInterface (void)
{
#ifdef ServerMainVers_2700
	{
		ACAPI_MenuItem_RegisterMenu(32501, 32511, MenuCode_UserDef, MenuFlag_Default);
		ACAPI_MenuItem_RegisterMenu(32502, 32512, MenuCode_UserDef, MenuFlag_Default);
		return ACAPI_MenuItem_RegisterMenu(AddOnMenuID, PromptID, MenuCode_UserDef, MenuFlag_Default);
	}
	
#else
	return ACAPI_Register_Menu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
#endif
}

GSErrCode Initialize (void)
{
#ifdef ServerMainVers_2700
	{
		ACAPI_MenuItem_InstallMenuHandler(32501, MenuCommandHandler);
		ACAPI_MenuItem_InstallMenuHandler(32502, MenuCommandHandler);
		return ACAPI_MenuItem_InstallMenuHandler(AddOnMenuID, MenuCommandHandler);
	}
#else
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
#endif
}

GSErrCode FreeData (void)
{
	return NoError;
}
