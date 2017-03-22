#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include "ftree.h"
#include "hash.h"

/*
1) use fork() and wait() to improve performance
2) copy of directory/file permissions
3) overwrite or skip existing files based on hash checking
4) cleanup -- remove not needed functions / comments
*/



/* Function for copying a file tree rooted at src to dest
 * Returns < 0 on error. The magnitude of the return value
 * is the number of processes involved in the copy and is
 * at least 1.
 */

/*
 * Copy the source folder to the destination
 */
int update_permission(const char *src, const char *dest);
bool same_size(const char *src, const char *dest);
char * get_fullname(const char *dir, const char *name);
int copy_file(const char *src, const char *dest);
bool is_directory(const char *path);


int copy_ftree(const char *src, const char *dest) { 
    int count = 1;

    if (!is_directory(src)) {
        printf("Please make sure %s is a valid directory\n", src);
        exit(1);
    }
    if (!is_directory(dest)) {
        printf("Please make sure %s is a valid directory\n", dest);
        exit(1);
    }
    

    DIR *dir = opendir(src);

    if (dir == NULL) {
        printf ("Cannot open directory '%s'\n", src);
        return -1;
    }

    struct dirent *dp;

    while ((dp = readdir(dir)) != NULL) {
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") || dp->d_type == DT_LNK) {
            continue;
        }
        char *src_filename = get_fullname(src, dp->d_name);
        char *dest_filename = get_fullname(dest, dp->d_name);
        
        if (dp->d_type == DT_REG) {

            if (same_size(src_filename, dest_filename)) {
            //same size
                FILE *fp = fopen(src_filename, "r");
                FILE *fp2 = fopen(dest_filename, "r");
                char *hash1 = hash(fp);
                char *hash2 = hash(fp2);
                fclose(fp);
                fclose(fp2);

                if (hash1 == hash2) {
                //check hash, same hash
                    if ((update_permission(src_filename, dest_filename)) == -1){
                        printf("chmod fails for %s.", dest_filename);
                        exit(1);
                    }
                } else {
                    copy_file(src_filename, dest_filename);
                    if ((update_permission(src_filename, dest_filename)) == -1){
                        printf("chmod fails for %s.", dest_filename);
                        exit(1);
                    }
                    free(src_filename);
                    free(dest_filename);
                }

            } else {
                copy_file(src_filename, dest_filename);
                if ((update_permission(src_filename, dest_filename)) == -1){
                        printf("chmod fails for %s.", dest_filename);
                        exit(1);
                    }
                free(src_filename);
                free(dest_filename);
            }
        }
        
        if (dp->d_type == DT_DIR) {
            //use fork
            int r;
            if ((r=fork() >0)) {
            //parent process
                if (!is_directory(dest_filename)) {
                    mkdir(dest_filename, 0777);
                } else {
                    if ((update_permission(src_filename, dest_filename)) == -1){
                        printf("chmod fails");
                        exit(1);
                    }
                }
                int status;
                if (wait(&status) == -1) {
                    perror("wait");
                }
                
                count += WEXITSTATUS(status);

            } else if (r==0) {
            //child process
                exit(copy_ftree(src_filename, dest_filename));

            } else {
                perror("fork");
                exit(1);
            }
        }
    }

    closedir(dir);

    return count;
}


/*
 * Update permission of dest_file
 */
int update_permission(const char *src, const char *dest) {
    struct stat file_info1;
    if (lstat(src, &file_info1) != 0) {
        perror("lstat");
        return -1;
    }
    chmod(dest, (file_info1.st_mode & 0777));

    return 0;
}


/*
 * Check if the src and dest files are the same size
 */

bool same_size(const char *src, const char *dest) {
    struct stat file_info1;
    struct stat file_info2;
    if (lstat(src, &file_info1) != 0) {
        return 0;
    }
    if (lstat(dest, &file_info2) != 0) {
        return 0;
    }

    return (file_info1.st_size == file_info2.st_size);

}

/*
 * Assemble full file name
 */

char * get_fullname(const char *dir, const char *name) {
    char *fullname = malloc(1024);
    strcpy(fullname, dir);
    strcat(fullname, "/");
    strcat(fullname, name);
    return fullname;
}

/*
 * Copy a file
 */

int copy_file(const char *src, const char *dest) {
    FILE *fr, *fw;
    char buffer[BUFFSIZE];
    
    if((fr = fopen(src,"rb")) == NULL){
        printf("cannot open this file for reading: %s.\n", src);
        return -1;
    }
    
    if((fw = fopen(dest, "wb")) == NULL){
        printf("cannot open this file for writing: %s.\n", dest);
        return -1;
    }
    
    
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, BUFFSIZE, fr)) != 0) {
        size_t written = fwrite(buffer, 1, bytes, fw);
        if( written != bytes) {
            printf("eroor in writing.\n");
            break;
        }
    }
    
    fclose(fr);
    fclose(fw);
    
    return 0;
}

/*
 * Check if this is a directory
 */

bool is_directory(const char *path) {
    struct stat path_info;
    
    return lstat(path, &path_info) == 0 && S_ISDIR(path_info.st_mode);
    
}