#include "mvFileIO.h"
#include <cstdlib>
#include <stdio.h>

char*
read_file(const char* file, unsigned& size, const char* mode)
{
    FILE* dataFile = fopen(file, mode);

    if (dataFile == nullptr)
    {
        //assert(false && "File not found.");
        return nullptr;
    }

    // obtain file size:
    fseek(dataFile, 0, SEEK_END);
    size = ftell(dataFile);
    fseek(dataFile, 0, SEEK_SET);

    // allocate memory to contain the whole file:
    char* data = new char[size+1];

    // copy the file into the buffer:
    size_t result = fread(data, sizeof(char), size, dataFile);
    if (result != size)
    {
        if (feof(dataFile))
            printf("Error reading test.bin: unexpected end of file\n");
        else if (ferror(dataFile)) {
            perror("Error reading test.bin");
        }
        //assert(false && "File not read.");
    }

    fclose(dataFile);

    data[size] = 0;
    return data;
}

