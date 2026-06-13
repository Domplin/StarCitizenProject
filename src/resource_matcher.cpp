#include "resource_matcher.h"
#include <cmath>
#include <cctype>





const std::vector<ResourceMatcher::Resource> ResourceMatcher::RS_TABLE = {
    {3170, "Quantainium"}, {3185, "Stileron"},    {3200, "Savrilium"},
    {3370, "Ouratite"},    {3385, "Riccite"},      {3400, "Lindinium"},
    {3540, "Beryl"},       {3555, "Taranite"},     {3570, "Borase"},
    {3585, "Gold"},        {3600, "Bexalite"},     {3825, "Laranite"},
    {3840, "Aslarite"},    {3855, "Titanium"},     {3870, "Tungsten"},
    {3885, "Agricium"},    {3900, "Torite"},        {4180, "Hephestanite"},
    {4195, "Tin"},         {4210, "Quartz"},        {4225, "Corundum"},
    {4240, "Copper"},      {4255, "Silicon"},       {4270, "Iron"},
    {4285, "Aluminium"},   {4300, "Ice"},
};

std::string ResourceMatcher::matchByRS(int value, int& outMult) {
    for(const auto& r : RS_TABLE){
        if(value % r.rsValue == 0){
            outMult = value / r.rsValue;
            return r.name;
        }
    }
    return "not a valid resource";
}



std::vector<ResourceMatch> ResourceMatcher::parse(const std::string& ocrText) {
    std::vector<ResourceMatch> results;
    std::string word;

    
    std::string text;
    for (char c : ocrText)
        if (c != ',') text += c;
    text += " ";

    for (char c : text) {
        if (isdigit(c)) {
            word += c;
        } else {
            if (!word.empty()) {
                int value = std::stoi(word);
                if (value >= 0 && value <= 1000000) {
                    int mult = 1;
                    std::string name = matchByRS(value, mult);
                    if (!name.empty())
                        results.push_back({ value, mult, name });
                }
                word.clear();
            }
        }
    }
    return results;
}