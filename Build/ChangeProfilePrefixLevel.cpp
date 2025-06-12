#include "ChangeProfilePrefixLevel.h"
#include <codecvt>
#include <string>
#include <locale>

std::vector<API_Element> SelectionBeamColumnWithPrefix(GS::Array<API_Neig> selNeigs) 
{
    std::vector<API_Element> vectorElements;
    API_SelectionInfo* selectionInfo= new API_SelectionInfo();
    GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
    if (selectionInfo->typeID == API_SelEmpty) 
    {
        delete selectionInfo;
        return vectorElements;
    }
    for (int i = 0; i < selectionInfo->sel_nElem; i++) 
    {
        API_Element element;
        element.header.guid = selNeigs[i].guid;
        err = ACAPI_Element_Get(&element);
        if (err == NoError && (element.header.type.typeID == API_BeamID 
            || element.header.type.typeID == API_ColumnID))
        {
            vectorElements.push_back(element);
        }
    }
    delete selectionInfo;
    return vectorElements;
}

std::wstring stringToWstring(const char* utf8Bytes)
{
    //setup converter
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(utf8Bytes);
}

void DisposeAttributePrefix(API_Attribute* attrib)
{
    switch (attrib->header.typeID)
    {
    case API_MaterialID:
        delete attrib->material.texture.fileLoc;
        break;
    default:
        break;
    }
}

GS::ErrCode ChangeProfileWithPrefixIdLevel()
{
    GS::ErrCode err = NoError;
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> vectorElements = SelectionBeamColumnWithPrefix(selNeigs);
    API_Attribute attrib;
    UInt32 count, i;
    err = ACAPI_Attribute_GetNum(API_ProfileID, count);
    if (err != NoError) return err;
    std::vector<API_Attribute> allProfile;
    for (i = 1; i <= count; i++) 
    {
        BNZeroMemory(&attrib, sizeof(API_Attribute));
        attrib.header.typeID = API_ProfileID;
        attrib.header.index =ACAPI_CreateAttributeIndex(i);
        err = ACAPI_Attribute_Get(&attrib);
        if(err==NoError)allProfile.push_back(attrib);
        DisposeAttributePrefix(&attrib);
    }

    API_ElementMemo memo;
    BNZeroMemory(&memo, sizeof(API_ElementMemo));

    API_StoryInfo storyInfo;
    BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
    err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
    if (err != NoError) return err;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (API_Element element : vectorElements) 
    {
        err = ACAPI_Element_GetMemo(element.header.guid, &memo);
        if (err != NoError) 
        {
            ACAPI_DisposeElemMemoHdls(&memo);
            continue;
        }

        API_StoryType storyElement;
        auto pointerIterator = &(*storyInfo.data)[0];
        for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) 
        {
            if (pointerIterator != nullptr) 
            {
                if (id == element.header.floorInd) {
                    storyElement = *pointerIterator;
                    break;
                }
                ++pointerIterator;
            }
        }
        
        auto id = stringToWstring(convert.to_bytes(storyElement.uName).c_str()) + L"_" + stringToWstring((*memo.elemInfoString).ToCStr());
        for (API_Attribute att : allProfile)
        {
            if (element.header.type.typeID == API_BeamID && att.profile.beamType) 
            {
                if (id == stringToWstring(att.profile.head.name))
                {
                    for (USize idx = 0; idx < element.beam.nSegments; ++idx)
                    {
                        memo.beamSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
                        memo.beamSegments[idx].assemblySegmentData.profileAttr= att.profile.head.index;
                    }
                    API_Element maskElem;
                    ACAPI_ELEMENT_MASK_CLEAR(maskElem);
                    err = ACAPI_Element_Change(&element, &maskElem, &memo, APIMemoMask_BeamSegment, true);
                    ACAPI_DisposeElemMemoHdls(&memo);
                    break;
                }
            }
            else if (element.header.type.typeID == API_ColumnID && att.profile.coluType) 
            {
                if (id == stringToWstring(att.profile.head.name))
                {
                    for (USize idx = 0; idx < element.column.nSegments; ++idx) 
                    {
                        memo.columnSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
                        memo.columnSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
                    }
                    API_Element maskElem;
                    ACAPI_ELEMENT_MASK_CLEAR(maskElem);
                    err = ACAPI_Element_Change(&element, &maskElem, &memo, APIMemoMask_ColumnSegment, true);
                    ACAPI_DisposeElemMemoHdls(&memo);
                    break;
                }
            }
        }
        
    }

    return NoError;
}
