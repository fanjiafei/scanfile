#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <regex.h>
#include <libgen.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
vector<string> v_file;
int regex_match(const char *buffer, const char *pattern)
{
    int ret = 0;
    char errbuf[1024] = {0};
    regex_t reg;
    regmatch_t pm[1] = {0};
    ret = regcomp(&reg, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0) {
        regerror(ret, &reg, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s:regcom(%s)\n", errbuf, pattern);
        return -1;
    }
    if (regexec(&reg, buffer, 1, pm, 0) == 0) {
        regfree(&reg);
        return 0;                         //匹配成功
    }
    else {
        regfree(&reg);
        return -1;
    }
}
int scan_dirpath(char *path, char *pattern)    //递归扫描该目录下所有的文件和目录
{
    char file_path[512] = {0};
    char file[512] = {0};
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    struct stat buf;
    int i, j;
    /****************浏览目录***************************/
    if ((dir = opendir(path)) == NULL) {
        perror("opendir failed!");
        return -1;
    }
    while((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {//除去根文件目录
            strcpy(file_path, path);
            if (path[strlen(path) - 1] != '/')  strcat(file_path, "/");
            strcat(file_path, ptr->d_name);          //构建完整的文件名
            assert(stat(file_path, &buf) != -1);
            if(S_ISREG(buf.st_mode)) {        //判断的是文件
                for(i = 0; i < strlen(file_path); i++) {
                    if(file_path[i] == '/') {
                        memset(file, 0, strlen(file));
                        j = 0;
                        continue;
                    }
                    file[j++] = file_path[i];
                }
                if (regex_match(file, pattern) == 0) {  //正则匹配成功
                    v_file.push_back(file_path);
                }
            }
            else if(S_ISDIR(buf.st_mode)) {   //判断的是目录
                scan_dirpath(file_path, pattern);
            }
        }
    }
    return 0;
}

//判断文件夹是否存在
int is_dir_exist(const char * dir_path){
    if(dir_path==NULL){
        return -1;
    }
    if(opendir(dir_path)==NULL){
        return -1;
    }
    return 0;
}


int main()
{
    char path[512] = "/media/fan/DiskA/Github/IPC_Serial/src";//source
    char pattern[32] = ".*.cpp";
    char sysCmd[512+50];
    char backDir[] = "/media/fan/DiskA/back";//destination
    int ret;
    scan_dirpath(path, pattern);
    for (int i = 0; i < v_file.size(); i++) {
        cout<<v_file[i]<<endl;
        if(-1==is_dir_exist(backDir)){
            printf("dir %s is not exist try mkdir\n",backDir);
            sprintf(sysCmd,"mkdir %s",backDir);
            system(sysCmd);
        }
        sprintf(sysCmd,"cp %s %s",v_file[i].c_str(),backDir);
        ret = system(sysCmd);
        printf("ret:%d cmd:%s\n",ret,sysCmd);
        system("sync");
    }
    return 0;
}
