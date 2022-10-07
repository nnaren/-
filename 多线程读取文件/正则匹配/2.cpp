int main(int argc, char *argv[])
{
    string excludeDirPattern;
    string onlyDirPattern = "*.*";    
    string filePattern = "*.*";
    vector<string> dirNames;
    
    xpressive::sregex re_program_options(xpressive::icase("--exclude=")  >> (+_)[xpressive::ref(excludeDirPattern)   = _] | 
                                         xpressive::icase("--in=")       >> (+_)[xpressive::ref(onlyDirPattern)   = _] |     
                                         xpressive::icase("--ext=")     >> (+_)[xpressive::ref(filePattern)  = _] | 
                                         (~(xpressive::set='-') >> *_)[xpressive::ref(dirNames)->*push_back(as<string>(_))]); 
    for_each(irange(1, argc), [argv, &re_program_options](auto i){string s(argv[i]); return xpressive::regex_match(s, re_program_options);});
    if (dirNames.empty())
    {
        cout << "useage: find [--exclude=xxx] [--in=xxx] [--ext=xxx] dirname dirname ..." << endl;
        return 1;  
    }
        
    ForWantedFiles([](const auto & fileName){cout << fileName << "\n";}, dirNames, filePattern, excludeDirPattern, onlyDirPattern);
     
    return EXIT_SUCCESS;
}