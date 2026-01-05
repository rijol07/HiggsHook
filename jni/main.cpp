
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

// Logger
#define TAG "HiggsHook"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Dobby Header (Pastikan folder Dobby ada di jni/Dobby)
#include "Dobby/include/dobby.h" 

// --- TARGET FUNCTION POINTERS ---
void* (*orig_NewGetWebsiteContent)(void*, void*, void*, void*, void*, int, int, bool);

// --- HOOK REPLACEMENT ---
void* my_NewGetWebsiteContent(void* thiz, void* host, void* resp, void* size, void* output, int output_len, int a7, bool a8) {
    LOGD("[!!!] CAPTURED NewGetWebsiteContent!");
    
    // Dump Output (Plaintext Request)
    if (output && output_len > 0) {
        char* buf = (char*)output;
        // Limit log length
        int len = output_len > 1024 ? 1024 : output_len;
        
        // Simple hex dump or string dump
        // Caution: Binary data might break logcat, printing header only
        LOGD("    Data Ptr: %p, Len: %d", buf, output_len);
        
        // Find command code at offset +30
        if (output_len > 34) {
             int cmd;
             memcpy(&cmd, buf + 30, 4);
             LOGD("    Command Code: 0x%X", cmd);
        }
    }

    // Call Original
    return orig_NewGetWebsiteContent(thiz, host, resp, size, output, output_len, a7, a8);
}

// --- JNI VTABLE HOOK STRATEGY ---

jint (*orig_RegisterNatives)(JNIEnv*, jclass, const JNINativeMethod*, jint);
jint my_RegisterNatives(JNIEnv* env, jclass clazz, const JNINativeMethod* methods, jint nMethods) {
    if (!methods || nMethods <= 0) return orig_RegisterNatives(env, clazz, methods, nMethods);

    // Debug: Print Class Name being registered
    jmethodID mid = env->GetMethodID(env->GetObjectClass(clazz), "getName", "()Ljava/lang/String;");
    if (mid) {
        jstring nameStr = (jstring)env->CallObjectMethod(clazz, mid);
        const char* name = env->GetStringUTFChars(nameStr, NULL);
        LOGD("[REG] Registering NATIVES for class: %s (Count: %d)", name, nMethods);
        
        // Check if this is the class we want (Optional optimization, but let's check everything)
        // if (strstr(name, "AndroidApi") || strstr(name, "Cocos2dxHttpURLConnection")) ...
        
        env->ReleaseStringUTFChars(nameStr, name);
    }

    // Scan the methods being registered
    for (int i = 0; i < nMethods; i++) {
        const char* mName = methods[i].name;
        const char* mSig = methods[i].signature;
        void* mPtr = methods[i].fnPtr;
        
        // Log all registrations to be sure
        LOGD("    [%d] %s %s -> %p", i, mName, mSig, mPtr);

        if (strstr(mName, "NewGetWebsiteContent")) {
            LOGD("[!!!] BINGO! Found NewGetWebsiteContent!");
            LOGD("      Function Pointer: %p", mPtr);
            
            // Install the PAYLOAD hook
            DobbyHook(mPtr, (void*)my_NewGetWebsiteContent, (void**)&orig_NewGetWebsiteContent);
            LOGD("[!!!] Payload Hook Installed Successfully!");
        }
    }

    return orig_RegisterNatives(env, clazz, methods, nMethods);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGD("===========================================");
    LOGD("       HIGGS DOMINO HOOK LOADED!           ");
    LOGD("       Mode: JNI VTable Hook (Ultimate)    ");
    LOGD("===========================================");

    JNIEnv* env = NULL;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("[-] Failed to get JNIEnv");
        return JNI_VERSION_1_6;
    }

    // THE ULTIMATE TRICK:
    // We don't need dlsym. We have the JNIEnv pointer!
    // The RegisterNatives function is at a fixed offset in the function table.
    
    // env points to a pointer to JNINativeInterface
    const struct JNINativeInterface* functions = env->functions;
    
    // functions->RegisterNatives IS the address of the function we want to hook!
    void* registerNativesAddr = (void*)functions->RegisterNatives;
    
    LOGD("[+] JNIEnv->RegisterNatives Address: %p", registerNativesAddr);
    
    if (registerNativesAddr) {
        // Install the TRAP hook
        DobbyHook(registerNativesAddr, (void*)my_RegisterNatives, (void**)&orig_RegisterNatives);
        LOGD("[+] VTable Hook Installed on RegisterNatives!");
    } else {
        LOGE("[-] Failed to get RegisterNatives address from VTable.");
    }

    return JNI_VERSION_1_6;
}
