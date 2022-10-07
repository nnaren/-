#include <vector>
#include <regex> 
#include <functional>
#include <filesystem>
#include <boost/regex.hpp>
#include "TextUtils.h"
using namespace std;
using namespace boost;
namespace fs = std::filesystem;
namespace boostfs = boost::filesystem;

typedef  pair<std::function<bool (const fs::path &)>, std::function<bool (const fs::path &)>> PathFilter;


boost::regex WildCard2Regex(const string & wildCard, const string & postFix = "")
{
    return boost::regex("(?i:" +  boost::regex_replace(wildCard, boost::regex("(\\*\\.\\*)|(\\*\\.(?=;|\\z))|(\\*)|(\\.)|(;\\z)|(;)|(\\?)"), "(?1.+)(?2[^.]+)(?3.*?)(?4\\\\.)(?5)(?6|)(?7.)", format_all) + ")" + postFix);    
}

boost::regex e_Include_Exclude_Filter("(?:\\`|(?<=;))\\s*(?:-([^;]+?)|\\+?([^-+][^;]*?))\\s*(;|\\z)");

std::function<bool (const fs::path &)> FileNameFilter(const string & filePattern) {
    string fileFilter = TrimAndEraseComment(filePattern);
    if (fileFilter.empty() ||fileFilter == "*.*") {
        return [](const fs::path & p) { return true; };
    }
    string excludeFile = regex_replace(fileFilter, e_Include_Exclude_Filter, "(?1$1$3)", format_all);
    string includeFile = regex_replace(fileFilter, e_Include_Exclude_Filter, "(?2$2$3)", format_all);
    // cout << "创建过滤器" << endl;
    // cout << excludeFile << endl;
    // cout << includeFile << endl;
    std::function<bool (const fs::path &)> includeFilter = includeFile.empty() ?
        std::function<bool (const fs::path &)> ([] (const fs::path & p ) {return true;}) :
        [includeFilePatterns = WildCard2Regex(includeFile)] 
            (const fs::path & p) {return regex_match(p.filename().string(), includeFilePatterns);};
    return excludeFile.empty() ? 
            includeFilter :
            std::function<bool (const fs::path &)> ([excludeFilePatterns = WildCard2Regex(excludeFile), includeFilter] (const fs::path &p)
            {return includeFilter(p) && !regex_match(p.filename().string(), excludeFilePatterns); });
}


PathFilter CreatePathFilter(const string & dirPattern, const string & filePattern) {
    string subDirFilter = TrimAndEraseComment(dirPattern);
    // cout << dirPattern << " " << filePattern << endl;
    string excludeDir = regex_replace(subDirFilter, e_Include_Exclude_Filter, "(?1$1$3)", format_all);
    string mustIncludeDir = regex_replace(subDirFilter, e_Include_Exclude_Filter, "(?2$2$3)", format_all);
    // cout << "创建目录过滤器" << endl;
    // cout << excludeDir << endl;
    std::function<bool (const fs::path &)> excludeDirFilter = 
        excludeDir.empty() 
        ? std::function<bool (const fs::path &)> ([] (const fs::path & p ) { return true; })
        : [subDirPatterns = WildCard2Regex(excludeDir)] (const fs::path & p) 
            { return !regex_match(p.filename().string(), subDirPatterns); };
    std::function<bool (const fs::path &)> includeFileFilter = 
        mustIncludeDir.empty() 
        ? FileNameFilter(filePattern)
        : [subDirPatterns = WildCard2Regex(mustIncludeDir, "(?:\\\\|\\z)"), FileNameFilter = FileNameFilter(filePattern)] 
            (const fs::path & p) { return FileNameFilter(p) && regex_search(p.parent_path().string(), subDirPatterns); };
    
    return {excludeDirFilter, includeFileFilter};
}