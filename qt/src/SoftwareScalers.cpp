#include <cstdint>

void S9xForceHires(void *buffer, int pitch, int &width, int &height)
{
    if (width <= 256)
    {
        for (int y = (height)-1; y >= 0; y--)
        {
            uint16_t *line = (uint16_t *)((uint8_t *)buffer + y * pitch);

            for (int x = (width * 2) - 1; x >= 0; x--)
            {
                *(line + x) = *(line + (x >> 1));
            }
        }

        width *= 2;
    }
}

static inline uint16_t average_565(uint16_t colora, uint16_t colorb)
{
    return (((colora) & (colorb)) + ((((colora) ^ (colorb)) & 0xF7DE) >> 1));
}
void S9xMergeHires(void *buffer, int pitch, int &width, int &height)
{
    if (width < 512)
        return;

    for (int y = 0; y < height; y++)
    {
        uint16_t *input = (uint16_t *)((uint8_t *)buffer + y * pitch);
        uint16_t *output = input;

        for (int x = 0; x < (width >> 1); x++)
        {
            *output++ = average_565(input[0], input[1]);
            input += 2;
        }
    }

    width >>= 1;
}
