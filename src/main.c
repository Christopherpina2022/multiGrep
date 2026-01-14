#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#define MAX_HIT_LENGTH 2048
typedef struct{
    int matchCounter;
    char *writePath;
    char *targetString;
} Word;

int writeToFile(Word *file, char* filePath, int lineNumber) {
    FILE *fptr;
    fptr = fopen(file->writePath, "a");

    // Create headers if first line
    if (file->matchCounter == 0) {
        fprintf(fptr, "filepath,lineNumber\n");
    }
    fprintf(fptr, "%s,%d\n", filePath, lineNumber);
    file->matchCounter++;
    fclose(fptr);
}

int toLowercase(char *newWord, const char *oldWord) {
    // append every character in the current buffer to the lowerbuffer but as lowercase letters, then add the terminating character
    while (*oldWord) {
        *newWord++ = tolower((unsigned char)*oldWord++);
    }
    *newWord = '\0';
}

int singleSearch(char *filePath, Word *target) {
    // Initialize File pointer then open file
    FILE *fptr;
    fptr = fopen(filePath, "r");
    int i = 1;

    // initialize the reading buffer
    char buffer[1024];
    char bufferLower[1024];
    char wordLower[256];
    toLowercase(wordLower, target->targetString);
    
    if (fptr == NULL) {
        fprintf(stderr, "File was not found, please make sure the path is correct.");
        return 1;
    }
    else {
        // write the data we capture into a CSV file
        while (fgets(buffer, sizeof(buffer), fptr)) {
            toLowercase(bufferLower, buffer);
            if (strstr(bufferLower, wordLower)) {
                writeToFile(target, filePath, i);
            }
            i++;
        }
        fclose(fptr);
    }
    return 0;
}

int folderSearch(char *folderPath, int recursiveBool, Word *word) {
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

        // Skip catch in case a path is way too long for the tool to handle
        size_t sizeNeeded = strlen(folderPath) + 1 + strlen(dirEntry->d_name) + 1;
        if (sizeNeeded > sizeof(path)) {
            fprintf(stderr, "Path too long, skipping: %s/%s\n", folderPath, dirEntry->d_name);
            continue;
        }

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
            singleSearch(path, word);
        }
        // Recursive function
        else if (S_ISDIR(st.st_mode) && recursiveBool) {
            folderSearch(path, recursiveBool,word);
        }
    }
    closedir(dir);

    return 0;
}

void printHelp(const char *application){
    printf(
        "\nUsage:\n"
        " %s -s <file> <word>\n"
        " %s -f[r] <folder> <word> \n"
        " %s -s <file> <word> <result path>\n"
        " %s -f[r] <folder> <result path>\n"
        "\nOptions:\n"
        " -s Searches a single file\n"
        " -f Searches a directory\n"
        " -r Recursive search (only works with -f)\n"
        " -h shows this help message\n"
        "\nAdding a result path will change the default (results.csv) that will write in the current directory.",
        application, application, application, application
    );
}

int main(int argc, char *argv[])
{
    Word word = {0};
    int i = 1;
    int flagSingle = 0;
    int flagFolder = 0;
    int flagRecursive = 0;
    int flagHelp = 0;


    // TODO: write a method to handle manual save locations and a default save location.
    char *defaultPath = "./results.csv";
    word.writePath = defaultPath;

    // Create or wipe the contents of the previous result file before writing to the file
    FILE *csv = fopen(word.writePath, "w");
    if (!csv) {
        fprintf(stderr, "Error: File could not be found. Please make sure you entered a valid csv file path.");
        return 1;
    }

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
    
    // Input validation
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

    // Load Struct information after input validation
    word.targetString = argv[3];
    
    // Executes the actual program, targetString is loaded into struct before
    if (flagSingle) {
        singleSearch(argv[2],&word);
    }
    else if (flagFolder) {
        folderSearch(argv[2], flagRecursive,&word);
    }

    printf("\nThe query has returned %d results.\nthe files will be saved in: %s", word.matchCounter, word.writePath);
    return 0;
}