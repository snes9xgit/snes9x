#include "slang_preset.hpp"

int main(int argc, char **argv)
{
    SlangPreset preset;

    if (argc != 2)
    {
        printf("Usage %s shaderpreset\n", argv[0]);
        return -1;
    }

    bool success = preset.load_preset_file(argv[1]);
    if (!success)
    {
        printf("Failed to load %s\n", argv[1]);
        return -1;
    }
    
    preset.introspect();

    preset.print();
 
    return 0;
}