boost::regex WildCard2Regex(const string & wildCard, const string & postFix = "")
{
    return boost::regex("(?i:" +  boost::regex_replace(wildCard, regex("(\\*\\.\\*)|(\\*\\.(?=;|\\z))|(\\*)|(\\.)|(;\\z)|(;)|(\\?)"), "(?1.+)(?2[^.]+)(?3.*?)(?4\\\\.)(?5)(?6|)(?7.)", format_all) + ")" + postFix);    
}
void ForWantedFiles(const std::function<void (const string &)> & functor, const vector<string> & dirNames, const string & filePattern = "*.*", const string & excludeDirPattern = "", const string & onlyDirPattern = "*.*")
{
    boost::regex e_ExcludeDirNamePattern = WildCard2Regex(excludeDirPattern);
    boost::regex e_OnlyDirNamePattern = WildCard2Regex(onlyDirPattern, "(?:\\\\|\\z)");    
    boost::regex e_FileNamePattern = WildCard2Regex(filePattern);
        
    for (auto & dirName : dirNames)
    {
        for ( fs::recursive_directory_iterator dir_itr(dirName); dir_itr != fs::recursive_directory_iterator(); ++dir_itr )
        {
            if (fs::is_directory(dir_itr->status()) && boost::regex_match(dir_itr->path().filename().string(), e_ExcludeDirNamePattern))
            {
                dir_itr.no_push();
            } 
            else if (fs::is_regular_file(dir_itr->status()) && 
                     boost::regex_match(dir_itr->path().filename().string(), e_FileNamePattern) &&
                     boost::regex_search(dir_itr->path().parent_path().string(), e_OnlyDirNamePattern))
            {
                functor(dir_itr->path().string());
            }
        } 
    }
}