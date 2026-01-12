#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

typedef struct{
    int line;
    char *targetString;
    char *hits[];
} Word;

int toLowercase(char *newWord, const char *oldWord) {
    // append every character in the current buffer to the lowerbuffer but as lowercase letters, then add the terminating character
    while (*oldWord) {
        *newWord++ = tolower((unsigned char)*oldWord++);
    }
    *newWord = '\0';
}

int singleSearch(char *filePath, char *input) {
    FILE *fptr;
    Word singleTarget;
    fptr = fopen(filePath, "r");

    char buffer[1024];
    char bufferLower[1024];

    // initialize target word and make case insensitive
    singleTarget.targetString = input;
    char wordLower[256];
    toLowercase(wordLower, singleTarget.targetString);
    singleTarget.line = 1;

    if (fptr == NULL) {
        fprintf(stderr, "File was not found, please make sure the path is correct.");
        return 1;
    }
    else{
        printf("File was found! looking for the word: %s\n", singleTarget.targetString);
        

        while (fgets(buffer, sizeof(buffer), fptr)) {
            // Make the line buffer for the current line case insensitive
            toLowercase(bufferLower, buffer);

            if (strstr(bufferLower, wordLower)) {
                printf("Found %s on line %d\n", singleTarget.targetString, singleTarget.line);
            }
            singleTarget.line++;
        }
        fclose(fptr);
    }
    return 0;
}

int folderSearch(char *folderPath, int recursiveBool, char *input) {
    DIR *dir = opendir(folderPath);
    if (!dir) {
        perror(folderPath);
        return 1;
    }

    struct dirent *dirEntry;
    

    while ((dirEntry = readdir(dir)) != NULL) {
        // Skip over '.' and '..'
        if (strcmp(dirEntry->d_name, "..") == 0 || strcmp(dirEntry->d_name, ".") == 0) {
            continue;
        }

        char path[1024];

        // Catch in case a path is way too long for the tool to handle
        size_t sizeNeeded = strlen(folderPath) + 1 + strlen(dirEntry->d_name) + 1;
        if (sizeNeeded > sizeof(path)) {
            fprintf(stderr, "Path too long, skipping: %s/%s\n", folderPath, dirEntry->d_name);
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", folderPath, dirEntry->d_name);
    }
    /*
    
    char path[PATH_MAX];

    if (dir == NULL) {
        fprintf(stderr, "Error: Directory cannot be opened, either due to not being a valid directory or needs Admin permissions.");
        return 1;
    }

    // Reads all the files / folders of the current directory being searched
    
    while ((dirEntry = readdir(dir)) != NULL)
    {
        // Skip over '.' and '..'
        if (strcmp(dirEntry->d_name, "..")) {
            continue;
        }
        
        struct stat st;
        if (stat(path, &st) == -1) {
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            printf(path);
            singleSearch(path, input);
        }
        else if (S_ISREG(st.st_mode && recursiveBool)) {
            folderSearch(path, recursiveBool, input);
        }
    }
    closedir(dir);
    */
    return 0;
}

void printHelp(const char *application){
    printf(
        "Usage:\n"
        "%s -s <file> <word> \n"
        "%s -f [-r] <file> <word> \n"
        "\nOptions:\n"
        " -s Searches a single file\n"
        " -f Searches a directory\n"
        " -r Recursive search (only works with -f)\n"
        " -h shows this Help Message\n",
        application, application
    );
}

int main(int argc, char *argv[])
{
    int i = 1;
    int flagSingle = 0;
    int flagFolder = 0;
    int flagRecursive = 0;
    int flagHelp = 0;

    int argumentIndex = 1;

    while (i < argc && argv[i][0] == '-'){
        for (int j = 1; argv[i][j] != '\0'; j++) {
            switch (argv[i][j])
            {
                case 's': flagSingle = 1; break;
                case 'f': flagFolder = 1; break;
                case 'r': flagRecursive = 1;break;
                case 'h': flagHelp = 1; break;
                default:
                    fprintf(stderr, "Error: Improper syntax. Please use %s -h to see the available flags.", argv[0]);
                    return 1;
            }
        }
        i++;
    }
    
    // Input Validation
    if (flagHelp) {
        printHelp(argv[0]);
        return 0;
    }
    if (flagSingle + flagFolder != 1) {
        fprintf(stderr, "Error: must specify exactly one of -s or -f in the first argument.\n");
        return 1;
    }
    if (flagRecursive && !flagFolder) {
        fprintf(stderr, "Error: -r can only be used with -f\n");
        return 1;
    }
    
    // Executes the actual program
    if (flagSingle) {
        singleSearch(argv[2], argv[3]);
    }
    else if (flagFolder) {
        folderSearch(argv[2], flagFolder, argv[3]);
    }
    return 0;
}

