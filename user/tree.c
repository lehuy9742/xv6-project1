#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void tree(char *path, int depth){
    struct stat st;
    struct dirent de;
    int fd;
    char buffer[512];
    char *p;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "tree: not a directory or file %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) { 
        fprintf(2, "tree: not a directory %s\n", path);
        close(fd);
        return;
    }

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buffer){
        printf("tree: path too long\n");
        close(fd);
        return;
    }

    strcpy(buffer, path);
    p = buffer + strlen(buffer);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        if(strcmp(p, ".") == 0 || strcmp(p, "..") == 0){
            continue;
        }
        if(stat(buffer, &st) < 0){
            fprintf(2, "tree: the child path could not be checked %s\n", path);
            continue;
        }

        if(st.type == T_DIR){
            for(int i = 0; i < depth; ++i){
                printf(" ");
            }
            printf("%s", p);
            printf("/\n");

            tree(buffer, depth + 1);
        }else if(st.type == T_FILE){
            for(int i = 0; i < depth; ++i){
                printf(" ");
            }
            printf("%s\n", p);
        }
    }
    if(read(fd, &de, sizeof(de)) < 0){
        fprintf(2, "tree: read error at %s", path);
    }
    close(fd);
    return;
}

int main(int argc, char *argv[]){
    if(argc == 1){
        tree(".", 1);
    }else{
        for(int i = 1; i < argc; ++i){
            tree(argv[i], 1);
        }
    }


}




    