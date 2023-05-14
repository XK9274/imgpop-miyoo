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
    if (argc != 6) {
        printf("Usage: %s duration image_path x_position y_position resize_percentage\n", argv[0]);
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

    int resize_percentage = atoi(argv[5]);
    int target_width = width * resize_percentage / 100;
    int target_height = height * resize_percentage / 100;

    SDL_Surface* image = IMG_Load(argv[2]);
    if (!image) {
        printf("Error: Unable to load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    double width_ratio = (double)target_width / image->w;
    double height_ratio = (double)target_height / image->h;
    double resize_ratio = (width_ratio < height_ratio) ? width_ratio : height_ratio;

    int resized_width = image->w * resize_ratio;
    int resized_height = image->h * resize_ratio;

   SDL_Surface* resized_image = zoomSurface(image, resize_ratio, resize_ratio, 1);
    SDL_FreeSurface(image);
    image = resized_image;

    // Rotate the image
    double angle = 180.0;
    SDL_Surface* rotated_image = rotozoomSurface(image, angle, 1.0, 1);
    SDL_FreeSurface(image);
    image = rotated_image;

    int x_position = atoi(argv[3]);
    int y_position = atoi(argv[4]);

    SDL_Rect dst_rect = {
        .x = x_position,
        .y = y_position,
        .w = resized_width,
        .h = resized_height
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