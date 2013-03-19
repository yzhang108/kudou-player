#ifndef PLAYER_H
#define PLAYER_H
int player_init();
int player_prepare(const char *url);
int player_main();
int player_exit();
int getDuration();
int getCurrentPosition();
int seekTo(int msec);
int streamPause();
int isPlay();
#endif

