# 题目1：多线程读取文件

## **基础题：**

完成文件名过滤功能，程序输入2参数：允许的后缀（如*.cpp;*.java;注意最后可能多一个; ） 和 要处理的目录。输出：目录下所有满足这些后缀的文件名。

注意：

- (?1xxx)是boost::regex的扩展功能且要加对应flag，std::regex能力太弱。
- 写正则必须小步，写一点验证一点，先工具验证，再程序验证

## **附加题：（有余力完成）**

- 可以完善后缀过滤，比如支持*.*、*、*.、*.txt.log*、*.log*.txt、*.t?t
- 可以完善接口，增加目录过滤功能。排除某些目录比如.git，或者只处理指定目录，比如只处理src和inc目录。

## 设计
### std::filesystem : 遍历目录、文件
### 正则表达式 过滤文件和目录
### 读取文件： 
* ifstream/istreambuf_iterator/fopen/fwrite 
* linux os api : open/read/write 文件映射：mmap
* windows os api : CreateFile/ReadFile/WriteFile 文件映射：CreateFileMapping

## 题目1：多线程读取文件
## 接口
命令行方式：findinfiles [-cfg cfgfilename] [-filter filterfilename] [-out outfilename] [-outfmt simple|full]
(fileordirname|-prj prjfilename)+
如果指定outfinename，则保护搜索结果到这个文件，并且自动退出；如果不指定outfilename，那就是只是从命令行启动程序，搜索后不会自动退出的。
如果不指定outfmt，默认是full，还会输出匹配的位置、条件和统计信息；simple只输出匹配的内容fileordirname和-prj prjfilename都filter文件每行一个正则表达式，前面有+的是过滤条件，前面有-的是反过滤条件

* 对于xls/xlsx的搜索，是以cell为单位的。
如果要cell里的完全匹配，需要启动正则“regex when search in files”，且在匹配内容前加\A，后加\z。

* 对于doc/docx/ppt/pptx/pdf，是使用doctotext这个软件转换成纯文本后搜索的，对匹配的“行”和“行号”不要太指望
并且，需要手工把doctotext的相关dll放到本程序所在目录下， 下载链接：[doctotext的相关dll](http://silvercoders.com/download/doctotext-4.0.1512-win32.tar.bz2)

### 本软件对正则的自定义控制功能介绍
过滤表中，纯空行、注释都会被去除，但是前后空格会保留
"(?>\\.|(?<m>int)|[^\\"])*"(?(<m>)|(?!))可匹配字符串里出现的int，但是务必注意这个?>，而不是?:
    但是仍然要适应skip扩展来处理引号的配对问题。
    利用*、+的子捕获组保存着最后一次的成功捕获状态，使用本软件的(?<match\d*>XXX)扩展，可以如此查找字符串出现的int：
"(?>\\.|(?<match1>int)|[^\\"])*"
    否则，需要使用本软件的(?(<skip\d*>XXX)扩展：
(?>"[\\"]*?int[^\\"])*")
(?<skip>"(?:\\.|(?>[^\\"]+))*")

例子：
匹配要求：    * .java; * .cpp; * .py
正则表达式： .*\.java|.*\.cpp|.*\.py
(?1xxx)(?2xxx)
(\*)(\.)(;)
(?1\.\*)(?2\.)(?3|)

正则：match、find、search

思路：
一个或者多个线程只读取文件，多线程只分析：
一个硬盘，一个线程读取文件：

### 生产者/消费者
2个条件变量：相互触发
1个mutex：保护制品库
1个automic变量：中断机制



常规字符 ：Char
.        ：Any
\表达式  ：Space(\s)、Word(\w)、Digit(\d)
序列     ：Seq
常规字符序列：Str，Str("ab")相当于Seq(Char('a'), Char('b'))

```
    bool match_ab*(const string & str)
{
    auto matcher = Seq(Str("ab"), Any);
    return matcher(str);
}

/* match: search for regexp anywhere in text */
int match(char *regexp, char *text)
{
	if (regexp[0] == '^')
	    return matchhere(regexp+1, text);
	do { /* must look even if string is empty */
	    if (matchhere(regexp, text))
	        return 1;
	} while (*text++ != '\0');
	return 0;
}
/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
        return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp+2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
	    return *text == '\0';
	if (*text!='\0' && (regexp[0]=='.' || regexp[0]==*text))
        return matchhere(regexp+1, text+1);
	return 0;
}

/* matchstar: search for c*regexp at beginning of text */ 
int matchstar(int c, char *regexp, char *text) 
{ 
	do { /* a * matches zero or more instances */ 
	    if (matchhere(regexp, text)) 
            return 1; 
	} while (*text != '\0' && (*text++ == c || c == '.')); 
	return 0; 
} 
```