#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

static int err_code;
int num = 0;
//global variable num set to keep track of file/directory count for -n

/*
 * here are some function signatures and macros that may be helpful.
 */

void handle_error(char* fullname, char* action);
bool test_file(char* pathandname);
bool is_dir(char* pathandname);
const char* ftype_to_str(mode_t mode);
void list_file(char* pathandname, char* name, bool list_long, bool numbered, struct stat sb, bool human);
void list_dir(char* dirname, bool list_long, bool list_all, bool recursive, bool numbered, bool human);
void lOutput(struct stat sb, char* name, bool human); // new function created to output -l details

/*
 * You can use the NOT_YET_IMPLEMENTED macro to error out when you reach parts
 * of the code you have not yet finished implementing.
 */
#define NOT_YET_IMPLEMENTED(msg)                  \
    do {                                          \
        printf("Not yet implemented: " msg "\n"); \
        exit(255);                                \
    } while (0)

/*
 * PRINT_ERROR: This can be used to print the cause of an error returned by a
 * system call. It can help with debugging and reporting error causes to
 * the user. Example usage:
 *     if ( error_condition ) {
 *        PRINT_ERROR();
 *     }
 */
#define PRINT_ERROR(progname, what_happened, pathandname)               \
    do {                                                                \
        printf("%s: %s %s: %s\n", progname, what_happened, pathandname, \
               strerror(errno));                                        \
    } while (0)

/* PRINT_PERM_CHAR:
 *
 * This will be useful for -l permission printing.  It prints the given
 * 'ch' if the permission exists, or "-" otherwise.
 * Example usage:
 *     PRINT_PERM_CHAR(sb.st_mode, S_IRUSR, "r");
 */
#define PRINT_PERM_CHAR(mode, mask, ch) printf("%s", (mode & mask) ? ch : "-");

/*
 * Get username for uid. Return 1 on failure, 0 otherwise.
 */
static int uname_for_uid(uid_t uid, char* buf, size_t buflen) {
    struct passwd* p = getpwuid(uid);
    if (p == NULL) {
        return 1;
    }
    strncpy(buf, p->pw_name, buflen);
    return 0;
}

/*
 * Get group name for gid. Return 1 on failure, 0 otherwise.
 */
static int group_for_gid(gid_t gid, char* buf, size_t buflen) {
    struct group* g = getgrgid(gid);
    if (g == NULL) {
        return 1;
    }
    strncpy(buf, g->gr_name, buflen);
    return 0;
}

/*
 * Format the supplied `struct timespec` in `ts` (e.g., from `stat.st_mtime`) as a
 * string in `char *out`. Returns the length of the formatted string (see, `man
 * 3 strftime`).
 */
static size_t date_string(struct timespec* ts, char* out, size_t len) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    struct tm* t = localtime(&ts->tv_sec);
    if (now.tv_sec < ts->tv_sec) {
        // Future time, treat with care.
        return strftime(out, len, "%b %e %Y", t);
    } else {
        time_t difference = now.tv_sec - ts->tv_sec;
        if (difference < 31556952ull) {
            return strftime(out, len, "%b %e %H:%M", t);
        } else {
            return strftime(out, len, "%b %e %Y", t);
        }
    }
}

/*
 * Print help message and exit.
 */
static void help() {
    /* TODO: add to this */
    printf("ls: List files\n");
    printf("\t--help: Print this help\n");
    exit(0);
}

/*
 * call this when there's been an error.
 * The function should:
 * - print a suitable error message (this is already implemented)
 * - set appropriate bits in err_code
 */
void handle_error(char* what_happened, char* fullname) {
    PRINT_ERROR("ls", what_happened, fullname);
    // err_code is set to 64 because if this function is called, there is an error
    err_code = 64;
    if (errno == EACCES) {
        err_code |= 16;
    }
    if (errno == ENOENT) {
        err_code |= 8;
    }
    if (errno != ENOENT && errno != EACCES) {
        err_code |= 32;
    }
    exit(err_code);


    // TODO: your code here: inspect errno and set err_code accordingly.
    return;
}

/*
 * test_file():
 * test whether stat() returns successfully and if not, handle error.
 * Use this to test for whether a file or dir exists
 */
bool test_file(char* pathandname) {
    struct stat sb;
    if (stat(pathandname, &sb)) {
        handle_error("cannot access", pathandname);
        return false;
    }
    return true;
}

/*
 * is_dir(): tests whether the argument refers to a directory.
 * precondition: test_file() returns true. that is, call this function
 * only if test_file(pathandname) returned true.
 */
bool is_dir(char* pathandname) {
    /* TODO: fillin */
    struct stat sb;
    if (test_file(pathandname)) {
        stat(pathandname, &sb);
        if (S_ISREG(sb.st_mode)) {
            return false;
        }
        else if (S_ISDIR(sb.st_mode)) {
            return true;
        }
    }
    // stat check if pathandname passed in is a directory or file
    return false;
}

/* convert the mode field in a struct stat to a file type, for -l printing */
const char* ftype_to_str(mode_t mode) {
    /* TODO: fillin */


    return "?";
}

void lOutput(struct stat sb, char* name, bool human) {
    mode_t mode = sb.st_mode;
    if (S_ISDIR(mode)){
        printf("d");
    }
    else if (S_ISREG(mode)){
        printf("-");
    }
    else {
        printf("?");
    }
    PRINT_PERM_CHAR(mode, S_IRUSR, "r");
    PRINT_PERM_CHAR(mode, S_IWUSR, "w");
    PRINT_PERM_CHAR(mode, S_IXUSR, "x");
    PRINT_PERM_CHAR(mode, S_IRGRP, "r");
    PRINT_PERM_CHAR(mode, S_IWGRP, "w");
    PRINT_PERM_CHAR(mode, S_IXGRP, "x");
    PRINT_PERM_CHAR(mode, S_IROTH, "r");
    PRINT_PERM_CHAR(mode, S_IWOTH, "w");
    PRINT_PERM_CHAR(mode, S_IXOTH, "x");
    printf(" %d", sb.st_nlink);
    char uname[50];
    if (uname_for_uid(sb.st_uid, uname, sizeof(uname)))
    {
       printf(" %d", sb.st_uid); 
       err_code = 96;
    }
    else {
        printf(" %s", uname);
    } 
    char group[50];
    if (group_for_gid(sb.st_gid, group, sizeof(group)))
    {
        printf(" %d", sb.st_gid);
        err_code = 96;
    }
    else {
        printf(" %s", group);
    }
    //logic for -h flag, rounds down but it was never specified how we should round
    if (!human || sb.st_size < 1000) {
        printf(" %ld", sb.st_size);
    }
    else {
        if (1000 <= sb.st_size && sb.st_size < 1000000){
                double number = sb.st_size / 1000;
                int rounded1 = (int)floor(number * 10 + 0.5);
                double rounded = (double)(rounded1 / 10);
                if (!(rounded1 % 10 == 0)){
                    printf(" %.1fK", rounded);
                }
                else {
                    printf(" %dK", rounded1 / 10);
                }
        }
        else if (1000000 <= sb.st_size && sb.st_size < 1000000000){
                double number = sb.st_size / 1000000;
                int rounded1 = (int)floor(number * 10 + 0.5);
                double rounded = (double)(rounded1 / 10);
                if (!(rounded1 % 10 == 0)){
                    printf(" %.1fM", rounded);
                }
                else {
                    printf(" %dM", rounded1 / 10);
                }
        }
        else if (1000000000 <= sb.st_size){
                double number = sb.st_size / 1000000000;
                int rounded1 = (int)floor(number * 10 + 0.5);
                double rounded = (double)(rounded1 / 10);
                if (!(rounded1 % 10 == 0)){
                    printf(" %.1fG", rounded);
                }
                else {
                    printf(" %dG", rounded1 / 10);
                }
        }
    }
    char mod[50];
    date_string(&sb.st_mtim, mod, sizeof(mod));
    printf(" %s ", mod);
}

/* list_file():
 * implement the logic for listing a single file.
 * This function takes:
 *   - pathandname: the directory name plus the file name.
 *   - name: just the name "component".
 *   - list_long: a flag indicated whether the printout should be in
 *   long mode.
 *
 *   The reason for this signature is convenience: some of the file-outputting
 *   logic requires the full pathandname (specifically, testing for a directory
 *   so you can print a '/' and outputting in long mode), and some of it
 *   requires only the 'name' part. So we pass in both. An alternative
 *   implementation would pass in pathandname and parse out 'name'.
 */
void list_file(char* pathandname, char* name, bool list_long, bool numbered, struct stat sb, bool human) {
    /* TODO: fill in*/
    if (list_long) {
        lOutput(sb, name, human);
        printf("%s\n", name);
    }
    else {
        if (!numbered){
            printf("%s\n", name);
        }
    }

}

/* list_dir():
 * implement the logic for listing a directory.
 * This function takes:
 *    - dirname: the name of the directory
 *    - list_long: should the directory be listed in long mode?
 *    - list_all: are we in "-a" mode?
 *    - recursive: are we supposed to list sub-directories?
 */

void list_dir(char* dirname, bool list_long, bool list_all, bool recursive, bool numbered, bool human) {
    /* TODO: fill in
     *   You'll probably want to make use of:
     *       opendir()
     *       readdir()
     *       list_file()
     *       snprintf() [to make the 'pathandname' argument to
     *          list_file(). that requires concatenating 'dirname' and
     *          the 'd_name' portion of the dirents]
     *       closedir()
     *   See the lab description for further hints
     */

        // checks if given arguement is a directory and all print options are blocked for -n
        if (is_dir(dirname)){
            DIR *dir = opendir(dirname);
            struct dirent *ent;
            struct stat sb;
            //opens directory
            if (recursive && !numbered) {
                printf("%s:\n", dirname);
                //only print if recursive to match output
            }
            //create pointer to char pointers, initialize it to store the size of 20 char pointers
            char **rec_arr = malloc(20 * sizeof(char*));
            for (int i = 0; i < 20; i++){
                rec_arr[i] = NULL;
                //initialize all subarrays to null
            }
            int track = 0;
            //keeps track of how many directory names will be stored in char pointer
            int col = 20;
            //keeps track of how many names we can have in our array
            while ((ent = readdir(dir)) != NULL) {
                char pathandname[100];
                int test = snprintf(pathandname, sizeof(pathandname), "%s/%s", dirname, ent->d_name);
                //return error if snprintf resulted in failure
                if (test == 0) {
                    handle_error("pathandname could not be created", dirname);
                }
                stat(pathandname, &sb);
                if (is_dir(pathandname) && recursive && (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0)) {
                    if (ent->d_name[0] == 46 && !list_all) {
                        //cant read hidden files unless list_all is true
                        continue;
                    }
                    else{
                        rec_arr[track] = strdup(pathandname);
                        track++;
                        //keeps track of all directory names in array
                        if (track > col) {
                            char **rec_arr = realloc(rec_arr, col * 2 * sizeof(char*));
                            col *= 2;
                            //resizes array if there is not enough space to keep all directory names
                    }
                    }
                }
                // conditions will print certain output if list_long is true and numbered is false, also if numbered is true I always pass list_long as false when calling list directory, which avoids print statements
                if (is_dir(pathandname) && list_all && (strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)){
                    num += 1;
                    if (list_long) {
                        lOutput(sb, ent->d_name, human);
                    }
                    if (!numbered){
                        printf("%s\n", ent->d_name);
                        }
                }

                else if (is_dir(pathandname) && list_all){
                    num++;
                    if (list_long) {
                        lOutput(sb, ent->d_name, human);
                    }
                    if (!numbered){
                        printf("%s/\n", ent->d_name);
                        }
                }

                else if (is_dir(pathandname) && (ent->d_name[0] != 46)){
                    num++;
                    if (list_long) {
                        lOutput(sb, ent->d_name, human);
                    }
                    if (!numbered){
                        printf("%s/\n", ent->d_name);
                        }
                }

                else if(test_file(pathandname) && !(is_dir(pathandname)) && list_all) {
                    num++;
                    list_file(pathandname, ent->d_name, list_long, numbered, sb, human);
                }

                else if(test_file(pathandname) && !(is_dir(pathandname)) && (ent->d_name[0] != 46)) {
                    num++;
                    list_file(pathandname, ent->d_name, list_long, numbered, sb, human);
                }

                else {
                    if (list_long && list_all) {
                        lOutput(sb, ent->d_name, human);
                        printf("%s\n", ent->d_name);
                    }
                    else if (list_all){
                        handle_error("not a file or directory", ent->d_name);
                        }
                    else if (!(ent->d_name[0] == 46)){
                        handle_error("not a file or directory", ent->d_name);
                        }
                }
                //free(pathandname);
            }
            // if recursive we recurse into directories and print their contents after all the current directories contents have been printed
            //we then free every sub array until we reach NULL, then free the while array and close directory
            if (recursive) {
                if (!numbered) {
                    printf("\n");
                }
                int k = 0;
                while (rec_arr[k] != NULL) {
                    list_dir(rec_arr[k], list_long, list_all, recursive, numbered, human);
                    free(rec_arr[k]);
                    k++;
                }
                free(rec_arr);
            }
            else{
                free(rec_arr);
            }
            closedir(dir);
        }
    }



int main(int argc, char* argv[]) {
    // This needs to be int since C does not specify whether char is signed or
    // unsigned.
    int opt;
    err_code = 0;
    bool list_long = false, list_all = false, recursive = false, numbered = false, human = false;
    // We make use of getopt_long for argument parsing, and this
    // (single-element) array is used as input to that function. The `struct
    // option` helps us parse arguments of the form `--FOO`. Refer to `man 3
    // getopt_long` for more information.
    struct option opts[] = {
        {.name = "help", .has_arg = 0, .flag = NULL, .val = '\a'}};

    // This loop is used for argument parsing. Refer to `man 3 getopt_long` to
    // better understand what is going on here.
    while ((opt = getopt_long(argc, argv, "1alRnh", opts, NULL)) != -1) {
        switch (opt) {
            case '\a':
                // Handle the case that the user passed in `--help`. (In the
                // long argument array above, we used '\a' to indicate this
                // case.)
                
                help();
                break;
            case '1':
                // Safe to ignore since this is default behavior for our version
                // of ls.
                break;
            case 'a':
                list_all = true;
                break;
                // TODO: you will need to add items here to handle the
                // cases that the user enters "-l" or "-R"
            case 'l':
                list_long = true;
                break;
            case 'R':
                recursive = true;
                break;
            case 'n':
                numbered = true;
                break;
            case 'h':
                human = true;
                break;
            default:
                printf("Unimplemented flag %d\n", opt);
                break;
        }
    }
    //after flag parsing, I check for number of arguments and if numbered is active, then make certain changes in what I print
    if (optind == argc && !numbered) {
            list_dir(".", list_long, list_all, recursive, numbered, human); 
        }
    else if (optind == argc && numbered) {
        list_dir(".", false, list_all, recursive, numbered, human);
        printf("%d\n", num);
    }
    else if ((optind + 1== argc) && !numbered) {
            list_dir(argv[optind], list_long, list_all, recursive, numbered, human); 
        }
    else{
        if (!numbered){
            for (int i = optind; i < argc; i++) {
                if (is_dir(argv[i])){
                    printf("%s:\n", argv[i]);
                    list_dir(argv[i], list_long, list_all, recursive, numbered, human);
                    printf("\n");
                }
                else {
                    printf("%s\n\n", argv[i]);
            }
            }
        }
        else {
            for (int i = optind; i < argc; i++) {
                if (is_dir(argv[i])){
                    list_dir(argv[i], false, list_all, recursive, numbered, human);
                }
            }
            printf("%d", num);
        }
    }
    exit(err_code);
}
