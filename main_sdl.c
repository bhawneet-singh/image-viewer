#include <stdlib.h>
#include <SDL2/SDL.h>
#include <pthread.h>

typedef struct 
{
    __uint16_t magic_no;
    __uint32_t f_size;
    __uint16_t app_s;
    __uint16_t app_s_two;
    __uint32_t offset;
}
__attribute__((packed))
bmp_file_header;

typedef struct
{
    __uint32_t h_size;
    __uint32_t width;
    __uint32_t height;
    __uint16_t c_planes;
    __uint16_t bpp;
    __uint32_t c_method;
    __uint32_t raw_size;
    __int32_t h_res;
    __int32_t v_res;
    __uint32_t color_p;
    __uint32_t imp_color;
}
__attribute__((packed))
bmp_info_header;

typedef struct
{
    __uint8_t blue;
    __uint8_t green;
    __uint8_t red;
}
__attribute__((packed))
rgb;

typedef struct
{
    bmp_info_header* binfo;
    rgb* buf;
}
argument;

int width = 1060;
int height = 720;

SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;

void* sepia(void* args);
void* grayscale(void* args);
void* reflect(void* args);
void* normal(void* args);

int main(int argc , char** argv) 
{
    SDL_Init(SDL_INIT_VIDEO);
    if(argc < 2)
    {
        printf("not valid cmdline args ! \n");
        return 1;
    }
    //opening the file
    FILE* input = fopen(argv[1],"rb");
    if(input == NULL)
    {
        printf("something went wrong while opeing the file \n");
        return 1;
    }

    //640*480
    bmp_file_header bfile;
    bmp_info_header binfo;
    //read fileheader and info header
    fread(&bfile,sizeof(bmp_file_header),1,input);
    fread(&binfo,sizeof(bmp_info_header),1,input);
    //sdl window of specific size according to image height
    float width_r = (float)width/binfo.height;
    float height_r = (float)height/binfo.width;
    float b_ratio = 0;
    if(width_r < height_r)
        b_ratio = width_r;
    else
        b_ratio = height_r;
    height = binfo.height * width_r;
    width = binfo.width * height_r;

    SDL_CreateWindowAndRenderer(height , width , 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    //buffer for data
    fseek(input,bfile.offset,0);
    rgb *buf = (rgb*) malloc(sizeof(rgb)*(binfo.width*binfo.height));
    int count = binfo.height;
    //reading into array 
    fread( buf , sizeof(rgb) , binfo.width * binfo.height , input);
    argument args;
    args.binfo = &binfo;
    args.buf = buf;
    pthread_t th_normal;
    pthread_t th_grayscale;
    pthread_t th_sepia;
    pthread_t th_reflect;
    pthread_create(&th_normal,NULL,&normal,(void*)&args);

    while (1) 
    {
        if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN)
        {
            int key = event.key.keysym.sym;
            if(key == 'q')
                break;
            if(key == 'g')
                pthread_create(&th_grayscale,NULL,&grayscale,(void*)&args);
            
            if(key =='n')
                pthread_create(&th_normal,NULL,&normal,(void*)&args);

            if(key =='s')
                pthread_create(&th_sepia,NULL,&sepia,(void*)&args);

            if(key =='r')
                pthread_create(&th_reflect,NULL,&reflect,(void*)&args);
        }
    }
    //quite sdl window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    //free heap
    free(buf);
    //close file
    fclose(input);
    return EXIT_SUCCESS;
}

void* grayscale(void* args)
{
    printf("grayscale ");
    bmp_info_header* binfo;
    rgb* buf;
    argument* arg = (argument*)args;
    binfo = arg->binfo;
    buf = arg->buf;
    for(int i = binfo->height - 1 , y = 0 ; i >= 0 ; i = i - ((float)binfo->height / height),y++)
        for(int j = 0 , x = 0; j < binfo->width ; j = j + (((float)binfo->width / width) - 1.5 ),x++)
        {
            __uint8_t avg = (((rgb*)buf + (int)(binfo->width * i) + j)->red + ((rgb*)buf + (int)(binfo->width) * i + j)->green + ((rgb*)buf + (int)(binfo->width) * i + j)->blue)/3;
            SDL_SetRenderDrawColor(renderer, avg , avg , avg , 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }

    SDL_RenderPresent(renderer);
    printf("applied !\n");
    return NULL;
}

void* normal(void* args)
{
    printf("normal ");
    bmp_info_header* binfo;
    rgb* buf;
    argument* arg = (argument*)args;
    binfo = arg->binfo;
    buf = arg->buf;
    for(int i = binfo->height - 1 , y = 0 ; i >= 0 ; i = i - ((float)binfo->height / height),y++)
        for(int j = 0 , x = 0; j < binfo->width ; j = j + (((float)binfo->width / width) - 1.5 ),x++)
        {
            SDL_SetRenderDrawColor(renderer,((rgb*)buf + (int)(binfo->width * i) + j)->red , ((rgb*)buf + (int)(binfo->width) * i + j)->green , ((rgb*)buf + (int)(binfo->width) * i + j)->blue, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }

    SDL_RenderPresent(renderer);
    printf("applied !\n");
    return NULL;
}

void* sepia(void* args)
{
    printf("sepia ");
    bmp_info_header* binfo;
    rgb* buf;
    argument* arg = (argument*)args;
    binfo = arg->binfo;
    buf = arg->buf;
    for(int i = binfo->height - 1 , y = 0 ; i >= 0 ; i = i - ((float)binfo->height / height),y++)
        for(int j = 0 , x = 0; j < binfo->width ; j = j + (((float)binfo->width / width) - 1.5 ),x++)
        {
            __uint8_t oRed = ((rgb*)buf + (int)(binfo->width * i) + j)->red ;
            __uint8_t oGreen = ((rgb*)buf + (int)(binfo->width) * i + j)->green ;
            __uint8_t oBlue = ((rgb*)buf + (int)(binfo->width) * i + j)->blue;
            __uint8_t red = 0.393 * oRed + 0.769 * oGreen + 0.189 * oBlue ;
            __uint8_t green = 0.349 * oRed + 0.686 * oGreen + 0.168 * oBlue ;
            __uint8_t blue = 0.272 * oRed + 0.534 * oGreen + 0.131 * oBlue ;
            SDL_SetRenderDrawColor(renderer,red,green,blue, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }

    SDL_RenderPresent(renderer);
    printf("applied !\n");
    return NULL;
}

void* reflect(void* args)
{
    printf("reflect ");
    bmp_info_header* binfo;
    rgb* buf;
    argument* arg = (argument*)args;
    binfo = arg->binfo;
    buf = arg->buf;
    for(int i = binfo->height - 1 , y = 0 ; i >= 0 ; i = i - ((float)binfo->height / height),y++)
        for(int j = binfo->width - 1 , x = 0; j >= 0 ; j = j - (((float)binfo->width / width) - 2 ),x++)
        {
            SDL_SetRenderDrawColor(renderer,((rgb*)buf + (int)(binfo->width * i) + j)->red , ((rgb*)buf + (int)(binfo->width) * i + j)->green , ((rgb*)buf + (int)(binfo->width) * i + j)->blue, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }

    SDL_RenderPresent(renderer);
    printf("applied !\n");
    return NULL;
}