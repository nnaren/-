#include "TextUtils.h"
#include <string>
#include <functional>
using namespace std;
using namespace boost;





// void ForWantedFiles(const std::function<void (const string &)> & functor, const vector<string> & dirNames, const string & filePattern = "*.*", const string & excludeDirPattern = "", const string & onlyDirPattern = "*.*")
// {
//     boost::regex e_ExcludeDirNamePattern = WildCard2Regex(excludeDirPattern);
//     boost::regex e_OnlyDirNamePattern = WildCard2Regex(onlyDirPattern, "(?:\\\\|\\z)");    
//     boost::regex e_FileNamePattern = WildCard2Regex(filePattern);
        
//     for (auto & dirName : dirNames)
//     {
//         for ( fs::recursive_directory_iterator dir_itr(dirName); dir_itr != fs::recursive_directory_iterator(); ++dir_itr )
//         {
//             if (fs::is_directory(dir_itr->status()) && boost::regex_match(dir_itr->path().filename().string(), e_ExcludeDirNamePattern))
//             {
//                 dir_itr.no_push();
//             } 
//             else if (fs::is_regular_file(dir_itr->status()) && 
//                      boost::regex_match(dir_itr->path().filename().string(), e_FileNamePattern) &&
//                      boost::regex_search(dir_itr->path().parent_path().string(), e_OnlyDirNamePattern))
//             {
//                 functor(dir_itr->path().string());
//             }
//         } 
//     }
// }

boost::regex WildCard2Regex(const string& wildCard, const string & postfix = "") {
    return boost::regex("(?i:" + boost::regex_replace(wildCard, regex("(\\*\\.\\*)|(\\*\\.(?=;|\\z))|(\\*)|(\\.)|(;\\z)|(;)|(\\?)"), "(?1.+)(?2[^.]+)(?3.*?)(?4\\\\.)(?5)(?6|)(?7.)", format_all) + ")" + postFix);    
}

regex e_Include_Exclude_Filter("(?:\\`|(?<=;))\\s*(?:-([^;]+?)|\\+?([^-+][^;]*?))\\s*(;|\\z)");

string GetIncludeExts(const string & text) {
    string fileFilter = TrimAndEraseComment(text);
    string includeFile = regex_replace(fileFilter, e_Include_Exclude_Filter, "(?2$2@3)", format_all); // 去掉-的部分，和前导+

    return includeFile.empty() ? "*" : regex_replace(includeFile, regex("\\*\\.\\*(>=;|\\z)"), "*"); // 受控件限制，"." 要改成*
}

std::function<bool (const fs::path &)> FileNameFilter(const string & filePattern) {
    string fileFilter = TrimAndEraseComment(filePattern);
    if (fileFilter.empty() ||fileFilter == "*.*") {
        return [](const fs::path & p) { return true; };
    }
    string excludeFile = regex_replace(fileFilter, e_Include_Exclude_Filter, "(?1$1$3)", format_all);
    string includeFile = regex_replace(fileFilter, e_Include_Exclude_Filter, "(?1$2$3)", format_all);

    std::function<bool (const fs::path &)> includeFilter = includeFile.empty() ?
        std::function<bool (const fs::path &) ([] (const fs::path & p ) {return true;}) :
        [includeFilePatterns = WildCard2Regex(includeFile)] 
            (const fs::path & p) {return regex_match(p.filename().string(), includeFilePatterns);};
    return excludeFile.empty() ? 
            includeFilter :
            std::function<bool (const fs::path &)>([excludeFilePatterns = WildCard2Regex(exclueFile), includeFilter] (const fs::path &p)
            {return includeFilter(p) && !regex_match(p.filename().string(), excludeFilePatterns); });
}

PathFilter CreatePathFilter(const string & dirPattern, const string & filePattern) {
    string subDirFilter = TrimAndEraseComment(dirPattern);

    string excludeDir = regex_replace(subDirFilter, e_Include_Exclude_Filter, "(?1$1$3)", format_all);
    string mustIncludeDir = regex_replace(subDirFilter, e_Include_Exclude_Filter, "(?2$2$3)", format_all);

    std::function<bool (const fs::path &)> excludeDirFilter = 
        excludeDir.empty() 
        ? std::function<bool (const fs::path &) ([] (const fs::path & p ) { return true; })
        : [subDirPatterns = WildCard2Regex(excludeDir)] (cosnt fs::path & p) 
            { return !regex_match(p.filename().string(), subDirPatterns); };
    std::function<bool (const fs::path &)> includeFileFilter = 
        mustIncludeDir.empty() 
        ? FileNameFilter(filePattern) :
        : [subDirPatterns = WildCard2Regex(mustIncludeDir, "(?:\\\\|\\z)"), FileNameFilter = FileNameFilter(filePattern)] 
            (cosnt fs::path & p) { return FileNameFilter(p) && regex_search(p.parent_path().string(), subDirPatterns); };
    
    return {excludeDirFilter, includeFileFilter};
}