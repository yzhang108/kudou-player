// tutorial02.c
// A pedagogical video player that will stream through every video frame as fast as it can.
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
// Use
//
// gcc -o tutorial02 tutorial02.c -lavformat -lavcodec -lz -lm `sdl-config --cflags --libs`
// to build (assuming libavformat and libavcodec are correctly installed,
// and assuming you have sdl-config. Please refer to SDL docs for your installation.)
//
// Run using
// tutorial02 myvideofile.mpg
//
// to play the video stream on your screen.


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

//#define SDL12

#include <stdio.h>

int dst_fix_fmt = PIX_FMT_RGB565;
//int dst_fix_fmt = PIX_FMT_YUV420P;


int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame;
  AVPacket        packet;
  int             frameFinished;
  AVFrame       *pFrameRGB;
  
  int           vpNumBytes;
  uint8_t       *vpBuffer;
  
#ifdef SDL12
  SDL_Overlay     *bmp;
  SDL_Surface     *screen;
#else
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
#endif
  SDL_Rect        rect;
  SDL_Event       event;
  struct SwsContext *img_convert_ctx = NULL;

  char filename[] = "/sdcard/test.mp4";
  // Register all formats and codecs
  av_register_all();

  //if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
  if(SDL_Init(SDL_INIT_VIDEO )) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }

  

  // Open video file
  if(avformat_open_input(&pFormatCtx, filename, NULL,  NULL)!=0)
    return -1; // Couldn't open file


  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information

  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, filename, 0);

  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream

  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;

  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }

  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1; // Could not open codec

  // Allocate video frame
  pFrame=avcodec_alloc_frame();
  pFrameRGB=avcodec_alloc_frame();

#ifdef SDL12
    #ifndef __DARWIN__
            screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
    #else
            screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
    #endif
      if(!screen) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
      }
         
    // Allocate a place to put our YUV image on that screen
    bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
				 pCodecCtx->height,
				 SDL_YV12_OVERLAY,
				 screen);
#else
    window = SDL_CreateWindow("MySDL",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  
                              pCodecCtx->width, pCodecCtx->height,
                              SDL_WINDOW_SHOWN);
    if(!window) {
        fprintf(stderr, "SDL: could not set video window - exiting\n");
        exit(1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Couldn't set create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    
    //texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,  pCodecCtx->width, pCodecCtx->height);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STATIC,  pCodecCtx->width, pCodecCtx->height);
    if (!texture) {
        fprintf(stderr, "Couldn't set create texture: %s\n", SDL_GetError());
        exit(1);
    }
    
    vpNumBytes = avpicture_get_size(dst_fix_fmt, pCodecCtx->width, pCodecCtx->height);
    vpBuffer = (uint8_t *) av_malloc(vpNumBytes*sizeof(uint8_t));
    avpicture_fill( (AVPicture*)pFrameRGB, vpBuffer,
                          dst_fix_fmt,
                          pCodecCtx->width, pCodecCtx->height);


#endif





  // Read frames and save first five frames to disk
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
    	avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

      // Did we get a video frame?
      if(frameFinished) {
#ifdef SDL12
			SDL_LockYUVOverlay(bmp);

			AVPicture pict;
			pict.data[0] = bmp->pixels[0];
			pict.data[1] = bmp->pixels[2];
			pict.data[2] = bmp->pixels[1];

			pict.linesize[0] = bmp->pitches[0];
			pict.linesize[1] = bmp->pitches[2];
			pict.linesize[2] = bmp->pitches[1];


         	img_convert_ctx = sws_getCachedContext(img_convert_ctx,pCodecCtx->width, pCodecCtx->height,  pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
         	if (img_convert_ctx == NULL) {
         		fprintf(stderr, "Cannot initialize the conversion context\n");
         		exit(1);
         	}
         	sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,0, pCodecCtx->height, pict.data, pict.linesize);

			SDL_UnlockYUVOverlay(bmp);

			rect.x = 0;
			rect.y = 0;
			rect.w = pCodecCtx->width;
			rect.h = pCodecCtx->height;
			SDL_DisplayYUVOverlay(bmp, &rect);
#else          
               //AVPicture pict = avcodec_alloc_frame();
               rect.x = 0;
	       rect.y = 0;
	       rect.w = pCodecCtx->width;
	       rect.h = pCodecCtx->height;
                        
               img_convert_ctx = sws_getCachedContext(img_convert_ctx,pCodecCtx->width, pCodecCtx->height,  pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,dst_fix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
         	if (img_convert_ctx == NULL) {
         		fprintf(stderr, "Cannot initialize the conversion context\n");
         		exit(1);
         	}
         	sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize); 
                //SDL_RenderClear(renderer);
                SDL_UpdateTexture(texture, NULL, pFrameRGB->data[0], pFrameRGB->linesize[0]);
                //printf("num:%d\n",  pFrameRGB->linesize[0]);
                SDL_RenderCopy(renderer, texture, NULL, &rect);
                SDL_RenderPresent(renderer);
                SDL_Delay(50);
              
                
#endif

      }
    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
    SDL_PollEvent(&event);
    switch(event.type) {
    case SDL_QUIT:
      SDL_Quit();
      exit(0);
      break;
    default:
      break;
    }

  }

  // Free the YUV frame
  av_free(pFrame);

  // Close the codec
  avcodec_close(pCodecCtx);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}
