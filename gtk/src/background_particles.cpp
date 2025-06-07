#include "background_particles.h"
#include <cstring>
#include <cmath>
#include <random>

static const double pi = 3.141592;

namespace Background
{

Particles::Particles(enum Mode mode)
{
    this->mode = Invalid;
    setmode(mode);
}

void Particles::setmode(enum Mode mode)
{
    if (mode == this->mode)
        return;
    std::random_device device;
    mt.seed(device());
    output.resize(256 * 224 * 2);
    gameimage.resize(256 * 224 * 2);

    uint16_t r, g, b;
    if (mode == Snow)
    {
        r = 0; g = 2; b = 6;
    }
    else 
    {
        r = 0; g = 0; b = 0;
    }

    for (int i = 0; i < 256 * 224; i++)
    {
        output[i] = (r << 11) | (g << 6) | (b);
    }
    particles.clear();
    
    this->mode = mode;
    std::uniform_real_distribution<double> real(-0.8, 0.8);
    wind = real(mt);

    for (int l = 0; l < 32; l++)
    {
        int mixr = r + (l / 31.0) * (31 - r);
        int mixg = g + (l / 31.0) * (31 - g);
        int mixb = b + (l / 31.0) * (31 - b);
        color_table[l] = (mixr << 11) | (mixg << 6) | mixb;
    }
}

void Particles::copyto(uint16_t *dst, int pitch)
{
    for (int y = 0; y < 224; y++)
    {
        memcpy((uint8_t *)dst + y * pitch, &output[y * 256], 512);
    }
}

void Particles::set_game_image(uint16_t *src, int pitch)
{
    for (int y = 0; y < 224; y++)
        memcpy(&gameimage[y * 256], &src[y * pitch / 2], 512);
}

void Particles::advance()
{
    if (mode == Snow)
        advance_snow();
    else
        advance_starfield();
}

inline void Particles::setpixel(int x, int y, uint16_t l)
{
    output[x + (y << 8)] = color_table[l];
}

void Particles::advance_starfield()
{
    rate = 0.5;

    std::uniform_real_distribution<double> realrand(0.0, 1.0);
    if (realrand(mt) <= rate)
    {
        Particle pt{};

        double angle = realrand(mt) * 2 * pi;
        pt.dx = cos(angle);
        pt.dy = sin(angle) * 0.7;
        pt.x = 128.5 + pt.dx * 5.0;
        pt.y = 112.5 + pt.dy * 5.0;
        pt.intensity = 0;

        particles.push_back(pt);
    }

    for (auto p = particles.begin(); p != particles.end(); p++)
    {
        setpixel(p->x - 0.5, p->y - 0.5, 0);
        setpixel(p->x + 0.5, p->y + 0.5, 0);
        setpixel(p->x + 0.5, p->y - 0.5, 0);
        setpixel(p->x - 0.5, p->y + 0.5, 0);

        p->x += p->dx;
        p->y += p->dy;

        if ((int)p->x < 1 || (int)p->x > 255 || (int)p->y < 1 || (int)p->y > 223)
        {
            p = particles.erase(p);
            continue;
        }

        p->intensity = (p->intensity + 1);
        if (p->intensity > 63)
            p->intensity = 63;

        uint16_t lum = p->intensity >> 1;

        // primitive bilinear filter effect
        double dx = p->x - 0.5;
        dx = dx - floor(dx);
        double dy = p->y - 0.5;
        dy = dy - floor(dy);

        uint16_t cul = (1.0 - dx) * (1.0 - dy) * lum;
        uint16_t cur = (dx) * (1.0 - dy) * lum;
        uint16_t cll = (1.0 - dx) * (dy)*lum;
        uint16_t clr = (dx) * (dy)*lum;

        setpixel(p->x - 0.5, p->y - 0.5, cul);
        setpixel(p->x + 0.5, p->y - 0.5, cur);
        setpixel(p->x - 0.5, p->y + 0.5, cll);
        setpixel(p->x + 0.5, p->y + 0.5, clr);
    }
}

void Particles::advance_snow()
{
    rate = 1.0;

    std::uniform_real_distribution<double> realrand;

    realrand = std::uniform_real_distribution<double>(-0.01, std::nextafter(0.01, 1.0));
    wind += realrand(mt);

    if (wind < -0.8)
        wind = -0.8;
    if (wind > 0.8)
        wind = 0.8;

    for (double new_snowflakes = rate; new_snowflakes >= 1.0; new_snowflakes -= 1.0)
    {
        Particle pt{};

        realrand = std::uniform_real_distribution<double>(0.0, 1.0);
        pt.x = ((wind + 0.8) / 1.6) * -192.0 + realrand(mt) * 384.0;
        pt.y = 0.0;
        pt.dx = 0.0;
        pt.dy = 0.6 + (realrand(mt) * 0.8);
        std::uniform_int_distribution<int> intrand(8, 31);
        pt.intensity = intrand(mt);

        particles.push_back(pt);
    }

    for (auto p = particles.begin(); p != particles.end(); p++)
    {
        if (p->x > 0.0 && p->x < 256.0)
            setpixel(p->x, p->y, 0);

        p->x += wind;
        p->y += p->dy;

        if ((int)p->y > 223)
        {
            p = particles.erase(p);
        }
        else if (p->x > 0.0 && p->x < 256.0)
            setpixel(p->x, p->y, p->intensity);
    }
}

} // namespace Background