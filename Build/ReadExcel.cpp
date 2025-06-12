#include "ReadExcel.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;


std::vector<DataProfile> GetDataFromExcel()
{
    std::vector<DataProfile> vectorDataResult= std::vector<DataProfile>();
    std::fstream fin;
    fin.open("C:/Tobic/CreateProfileAutoArchicadColumn.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s,word,','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfile data = DataProfile();
        data.Name = content[i][0];
        data.Width = std::stold(content[i][1]) / 1000;
        data.Height = std::stold(content[i][2]) / 1000;
        data.IsColumn = true;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }


    std::fstream finBeam;
    finBeam.open("C:/Tobic/CreateProfileAutoArchicadBeam.csv", std::ios::in);
    vector<vector<string>> contentBeam;
    std::vector<std::string> rowBeam;
    std::string lineBeam, wordBeam;
    while (getline(finBeam, lineBeam))
    {
        rowBeam.clear();
        stringstream s(lineBeam);
        while (getline(s, wordBeam, ','))
        {
            rowBeam.push_back(wordBeam);
        }
        contentBeam.push_back(rowBeam);
    }

    for (int i = 1; i < contentBeam.size(); i++)
    {
        DataProfile data = DataProfile();
        data.Name = contentBeam[i][0];
        data.Width = std::stold(contentBeam[i][1]) / 1000;
        data.Height = std::stold(contentBeam[i][2]) / 1000;
        data.IsColumn = false;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }

    return vectorDataResult;

}


std::vector<DataProfileH> GetDataFromExcelForH()
{
    std::vector<DataProfileH> vectorDataResult = std::vector<DataProfileH>();
    std::fstream fin;
    fin.open("C:/Tobic/H_CreateProfileAutoArchicadColumn.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfileH data = DataProfileH();
        data.Name = content[i][0];
        data.Width = std::stold(content[i][2]) / 1000;
        data.Height = std::stold(content[i][1]) / 1000;
        data.tw = std::stold(content[i][3]) / 1000;
        data.tf = std::stold(content[i][4]) / 1000;
        data.IsColumn = true;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }


    std::fstream finBeam;
    finBeam.open("C:/Tobic/H_CreateProfileAutoArchicadBeam.csv", std::ios::in);
    vector<vector<string>> contentBeam;
    std::vector<std::string> rowBeam;
    std::string lineBeam, wordBeam;
    while (getline(finBeam, lineBeam))
    {
        rowBeam.clear();
        stringstream s(lineBeam);
        while (getline(s, wordBeam, ','))
        {
            rowBeam.push_back(wordBeam);
        }
        contentBeam.push_back(rowBeam);
    }

    for (int i = 1; i < contentBeam.size(); i++)
    {
        DataProfileH data = DataProfileH();
        data.Name = contentBeam[i][0];
        data.Width = std::stold(contentBeam[i][2]) / 1000;
        data.Height = std::stold(contentBeam[i][1]) / 1000;
        data.tw = std::stold(contentBeam[i][3]) / 1000;
        data.tf = std::stold(contentBeam[i][4]) / 1000;
        data.IsColumn = false;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }

    return vectorDataResult;

}

std::vector<DataProfileRecSteel> GetDataFromExcelForRecSteelColumn()
{
    std::vector<DataProfileRecSteel> vectorDataResult = std::vector<DataProfileRecSteel>();
    std::fstream fin;
    fin.open("C:/Tobic/RecSteel_CreateProfileAutoArchicadColumn.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfileRecSteel data = DataProfileRecSteel();
        data.Name = content[i][0];
        data.Width = std::stold(content[i][1]) / 1000;
        data.Height = std::stold(content[i][2]) / 1000;
        data.Thickness = std::stold(content[i][3]) / 1000;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }

    return vectorDataResult;

}

std::vector<DataProfile> GetDataFromExcelForBeamDefault()
{
    std::vector<DataProfile> vectorDataResult = std::vector<DataProfile>();
    std::fstream fin;
    fin.open("C:/TobicCadArchi/BeamTypeInput.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfile data = DataProfile();
        data.Name = content[i][0];
        data.Width = std::stold(content[i][1]) / 1000;
        data.Height = std::stold(content[i][2]) / 1000;
        data.IsColumn = false;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }

    return vectorDataResult;

}

std::vector<DataProfile> GetDataFromExcelForColumnDefault()
{
    std::vector<DataProfile> vectorDataResult = std::vector<DataProfile>();
    std::fstream fin;
    fin.open("C:/TobicCadArchi/ColumnTypeInput.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfile data = DataProfile();
        data.Name = content[i][0];
        data.Width = std::stold(content[i][1]) / 1000;
        data.Height = std::stold(content[i][2]) / 1000;
        data.IsColumn = true;
        if (data.Width > 0) vectorDataResult.push_back(data);
    }

    return vectorDataResult;
}

std::vector<DataProfileMashi> GetDataFromExcelForMashi()
{
    std::vector<DataProfileMashi> vectorDataResult = std::vector<DataProfileMashi>();
    std::fstream fin;
    fin.open("C:/Tobic/CreateProfileAutoArchicadColumn.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        DataProfileMashi data = DataProfileMashi();
        data.Name = content[i][0];
        data.Top = std::stold(content[i][3]) / 1000;
        data.Bottom = std::stold(content[i][4]) / 1000;
        data.Left= std::stold(content[i][5]) / 1000;
        data.Right = std::stold(content[i][6]) / 1000;
       vectorDataResult.push_back(data);
    }


    std::fstream finBeam;
    finBeam.open("C:/Tobic/CreateProfileAutoArchicadBeam.csv", std::ios::in);
    vector<vector<string>> contentBeam;
    std::vector<std::string> rowBeam;
    std::string lineBeam, wordBeam;
    while (getline(finBeam, lineBeam))
    {
        rowBeam.clear();
        stringstream s(lineBeam);
        while (getline(s, wordBeam, ','))
        {
            rowBeam.push_back(wordBeam);
        }
        contentBeam.push_back(rowBeam);
    }

    for (int i = 1; i < contentBeam.size(); i++)
    {
        DataProfileMashi data = DataProfileMashi();
        data.Name = contentBeam[i][0];
        data.Top = std::stold(contentBeam[i][3]) / 1000;
        data.Bottom = std::stold(contentBeam[i][4]) / 1000;
        data.Left = std::stold(contentBeam[i][5]) / 1000;
        data.Right = std::stold(contentBeam[i][6]) / 1000;
        vectorDataResult.push_back(data);
    }

    return vectorDataResult;

}


std::vector<LayerName> GetLayerNames()
{
    std::vector<LayerName> vectorDataResult = std::vector<LayerName>();
    std::fstream fin;
    fin.open("C:/Tobic/CreateLayers.csv", std::ios::in);
    vector<vector<string>> content;
    std::vector<std::string> row;
    std::string line, word;
    while (getline(fin, line))
    {
        row.clear();
        stringstream s(line);
        while (getline(s, word, ','))
        {
            row.push_back(word);
        }
        content.push_back(row);
    }

    for (int i = 1; i < content.size(); i++)
    {
        LayerName data = LayerName();
        data.Name = content[i][0];
        if (!data.Name.empty() && data.Name != "")
        {
            vectorDataResult.push_back(data);
        }
    }
    return vectorDataResult;

}




