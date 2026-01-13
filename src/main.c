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
    int *hits[];
} Word;

int writeToFile(char *writePath) {
    FILE *fptr;
    fptr = fopen(writePath, "w");

    if (fptr == NULL) {
        fprintf(stderr, "Error: Could not save to %s, please use another location.\n", writePath);
        return 1;
    }

    // TODO: Use our hits property in the Word Structure to write everything to a CSV file
    fprintf(fptr, "This is a test");

    fclose(fptr);
}

int toLowercase(char *newWord, const char *oldWord) {
    // append every character in the current buffer to the lowerbuffer but as lowercase letters, then add the terminating character
    while (*oldWord) {
        *newWord++ = tolower((unsigned char)*oldWord++);
    }
    *newWord = '\0';
}

int singleSearch(char *filePath, char *input) {
    // Initialize File pointer then open file
    FILE *fptr;
    Word singleTarget;
    fptr = fopen(filePath, "r");

    // initialize the reading buffer
    char buffer[1024];
    char bufferLower[1024];

    // initialize the target word
    singleTarget.targetString = input;
    char wordLower[256];
    toLowercase(wordLower, singleTarget.targetString);
    singleTarget.line = 1;

    if (fptr == NULL) {
        fprintf(stderr, "File was not found, please make sure the path is correct.");
        return 1;
    }
    else{
        /* This is for testing purposes, since we are using this in our folder function
        and it will spam the log really bad*/
        printf("File was found! looking for the word: %s\n", singleTarget.targetString);
        
        while (fgets(buffer, sizeof(buffer), fptr)) {
            toLowercase(bufferLower, buffer);
            if (strstr(bufferLower, wordLower)) {
                printf("Found %s on line %d\n", singleTarget.targetString, singleTarget.line);
                // TODO: write the data we capture into a file, preferably a CSV
                writeToFile("./results.csv");
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
        if (strcmp(dirEntry->d_name, ".") == 0 ||
            strcmp(dirEntry->d_name, "..") == 0)
            continue;

        char path[1024];

        /* Catch in case a path is way too long for the tool to handle, 
        not sure yet if I should skip the path or terminate the app in the event this passes. */
        size_t sizeNeeded = strlen(folderPath) + 1 + strlen(dirEntry->d_name) + 1;
        if (sizeNeeded > sizeof(path)) {
            fprintf(stderr, "Path too long, skipping: %s/%s\n", folderPath, dirEntry->d_name);
            continue;
        }

        memset(path, 0xAA, sizeof(path));
        snprintf(path, sizeof(path), "%s/%s", folderPath, dirEntry->d_name);

        // Stat gathers file metadata which st_mode tells us the file type and permissions
        struct stat st;
        if (stat(path, &st) != 0) {
            perror(path);
            continue;
        }

        // Ignore Symbolic Links (pointer to another file or directory)
        if (S_ISLNK(st.st_mode)) {
            continue;
        }
        // Look for our word if it is a regular file
        if (S_ISREG(st.st_mode)) {
            printf("%s\n",path);
            //singleSearch(path, input);
        }
        // Recursive function
        else if (S_ISDIR(st.st_mode) && recursiveBool) {
            folderSearch(path, recursiveBool, input);
        }
    }
    closedir(dir);

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

    // TODO: write a method to handle manual save locations and a default save location. be sure
    // to let the end user know it's a feature in -h afterwards.
    char *defaultPath = "./results.csv";

    for (int i = 1; i < argc; i++){
        if (argv[i][0] == '-') {
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
        }
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
        folderSearch(argv[2], flagRecursive, argv[3]);
    }
    return 0;
}

