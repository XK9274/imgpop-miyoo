#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

Uint32 get_pixel(SDL_Surface* surface, int x, int y);
void put_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel);

void put_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16*)p = pixel;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32*)p = pixel;
            break;
    }
}

Uint32 get_pixel(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16*)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32*)p;
        default:
            return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s duration image_path x_position y_position\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Convert duration argument to milliseconds
    int duration = atoi(argv[1]) * 1000;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: Unable to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);

    // Open the framebuffer device
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        perror("Error: Unable to open framebuffer device");
        return EXIT_FAILURE;
    }

    // Get framebuffer information
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error: Unable to get variable screen information");
        close(fb_fd);
        return EXIT_FAILURE;
    }

    // Create an SDL surface from the framebuffer
    int width = vinfo.xres;
    int height = vinfo.yres;
    int bpp = vinfo.bits_per_pixel;
    int pitch = width * (bpp / 8);
    size_t map_size = pitch * height;
    void* fb0_map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    SDL_Surface* screen = SDL_CreateRGBSurfaceFrom(fb0_map, width, height, bpp, pitch, 0, 0, 0, 0);

    // Load the image
    SDL_Surface* image = IMG_Load(argv[2]);
    if (!image) {
        printf("Error: Unable to load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Set the surface alpha blending mode
    Uint32 alpha = SDL_MapRGBA(image->format, 0, 0, 0, 0);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, alpha);

    // Create a temporary surface to flip the image
    SDL_Surface* flipped_image = SDL_CreateRGBSurface(image->flags, image->w, image->h, bpp, image->format->Rmask, image->format->Gmask, image->format->Bmask, image->format->Amask);
	
	// Get x and y positions from command line arguments
    int x_position = atoi(argv[3]);
    int y_position = atoi(argv[4]);

    // Create a destination rectangle with x and y positions
    SDL_Rect dst_rect = {
        .x = x_position,
        .y = y_position,
        .w = image->w,
        .h = image->h
    };

    // Blit the image onto the framebuffer surface
    Uint32 start_time = SDL_GetTicks();
    Uint32 elapsed_time = 0;

    while (elapsed_time < duration) {
        SDL_BlitSurface(image, NULL, screen, &dst_rect);
        elapsed_time = SDL_GetTicks() - start_time;
    }

    // Clean up
    SDL_FreeSurface(image);
    munmap(fb0_map, map_size);
    close(fb_fd);

    return 0;
}