# HiggsHook - Native Android Hooking Project

Project ini berisi source code C++ untuk melakukan **Native Hooking** pada game Android menggunakan library **Dobby**.

Tujuan utamanya adalah menyisipkan kode kita ke dalam proses game (`libgame.so`) untuk memantau dan memodifikasi fungsi internal.

## Struktur Project

- **jni/**: Folder utama source code.
    - `main.cpp`: Logika utama hooking (VTable Hook & Memory Scanner).
    - `Android.mk`: Konfigurasi build module.
    - `Application.mk`: Konfigurasi target ABI (arm64-v8a).
    - `Dobby/`: Library Dobby untuk melakukan inline hook.
- **build_native.bat**: Script Windows untuk compile otomatis menggunakan NDK.

## Alur Kerja (Workflow)

1.  **Persiapan Load**
    - Library `libhiggshook.so` diload oleh APK target (lewat injeksi Smali `System.loadLibrary`).

2.  **Eksekusi Hook (JNI_OnLoad)**
    - Saat library diload, fungsi `JNI_OnLoad` di `main.cpp` berjalan.
    - Script akan mencari alamat fungsi target menggunakan strategi:
        - **JNI VTable Intercept**: Membajak fungsi `RegisterNatives` dari `JNIEnv` untuk menangkap pendaftaran fungsi native game.

3.  **Payload**
    - Setelah target ditemukan, Dobby akan mengganti instruksi di alamat tersebut untuk melompat ke fungsi palsu kita (`my_NewGetWebsiteContent`).
    - Di dalam fungsi palsu, kita bisa mencatat log data sebelum meneruskannya ke fungsi asli.
