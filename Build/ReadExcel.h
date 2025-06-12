#pragma once
#include "DataProfile.h"
#include <vector>
#include <list>

std::vector<DataProfile> GetDataFromExcel();
std::vector<DataProfileH> GetDataFromExcelForH();
std::vector<DataProfileRecSteel> GetDataFromExcelForRecSteelColumn();
std::vector<DataProfile> GetDataFromExcelForBeamDefault();
std::vector<DataProfile> GetDataFromExcelForColumnDefault();

std::vector<DataProfileMashi> GetDataFromExcelForMashi();


std::vector<LayerName> GetLayerNames();
