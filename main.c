#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s duration image_path x_position y_position\n", argv[0]);
        return EXIT_FAILURE;
    }

    int duration = atoi(argv[1]) * 1000;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: Unable to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);

    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        perror("Error: Unable to open framebuffer device");
        return EXIT_FAILURE;
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error: Unable to get variable screen information");
        close(fb_fd);
        return EXIT_FAILURE;
    }

    int width = vinfo.xres;
    int height = vinfo.yres;
    int bpp = vinfo.bits_per_pixel;
    int pitch = width * (bpp / 8);
    size_t map_size = pitch * height;
    void* fb0_map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    SDL_Surface* screen = SDL_CreateRGBSurfaceFrom(fb0_map, width, height, bpp, pitch, 0, 0, 0, 0);

    SDL_Surface* image = IMG_Load(argv[2]);
    if (!image) {
        printf("Error: Unable to load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    double angle = 180.0;
    double zoom = 1.0;
    int smooth = 1;
    SDL_Surface* rotated_image = rotozoomSurface(image, angle, zoom, smooth);

    SDL_FreeSurface(image);
    image = rotated_image;

    int x_position = atoi(argv[3]);
    int y_position = atoi(argv[4]);

    SDL_Rect dst_rect = {
        .x = x_position,
        .y = y_position,
        .w = image->w,
        .h = image->h
    };

    Uint32 start_time = SDL_GetTicks();
    Uint32 elapsed_time = 0;

    while (elapsed_time < duration) {
        SDL_BlitSurface(image, NULL, screen, &dst_rect);
        SDL_Flip(screen);
        elapsed_time = SDL_GetTicks() - start_time;
        SDL_Delay(500);
    }

    SDL_FreeSurface(image);
    munmap(fb0_map, map_size);
    close(fb_fd);

    return 0;
}
