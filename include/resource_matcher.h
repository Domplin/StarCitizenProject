#pragma once
#include <string>
#include <vector>


struct ResourceMatch    
{
    int rsValue;
    int multiplier;
    std::string name;
};

class ResourceMatcher {
public: 
    std::vector<ResourceMatch> parse (const std::string& ocrText);

private:
    std::string matchByRS(int value, int& outMult);
    
    struct Resource {int rsValue; const char* name;};
    static const std::vector<Resource> RS_TABLE;
};