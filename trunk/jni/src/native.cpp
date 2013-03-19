#include <unistd.h>
#include <jni.h>
#include <android/log.h>

extern "C" {
#include "play.h"
}

#ifdef ANDROID

/* Include the SDL main definition header */

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/


// Library init
//extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
//{
//    return JNI_VERSION_1_4;
//}

// Start up the SDL app
extern "C" int Java_org_libsdl_app_SDLActivity_PlayerInit(JNIEnv* env,  jobject obj)
{
   return player_init();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerPrepare(JNIEnv* env,  jobject obj, jstring jfileName)
{
        jboolean isCopy;
        char localFileName[1024];
        const char *fileString     = env->GetStringUTFChars(jfileName, &isCopy);

        strncpy(localFileName, fileString, 1024);
        env->ReleaseStringUTFChars(jfileName, fileString);
        return player_prepare(localFileName);
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerMain(JNIEnv* env,  jobject obj)
{
   return player_main();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerExit(JNIEnv* env,  jobject obj)
{
   return player_exit();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerSeekTo(JNIEnv* env,  jobject obj, jint msec)
{
   int pos = msec;
   return seekTo(pos);
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerPause(JNIEnv* env,  jobject obj)
{
   return streamPause();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerIsPlay(JNIEnv* env,  jobject obj)
{
   return isPlay();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayerGetDuration(JNIEnv* env,  jobject obj)
{
   return getDuration();
}

extern "C" int Java_org_libsdl_app_SDLActivity_PlayergetCurrentPosition(JNIEnv* env,  jobject obj)
{
   return getCurrentPosition();
}

#endif /* ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
