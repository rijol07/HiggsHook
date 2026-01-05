# HiggsHook - Native Android Hooking Project

This project implements a native hooking library (`libhiggshook.so`) for Android, designed to intercept internal function calls within `libgame.so`. It uses the **Dobby** hooking framework and multiple strategies to locate hidden functions.

## Project Structure

- **jni/**: C++ Source Code (`main.cpp`) and Build Config (`Android.mk`, `Application.mk`).
- **libs/**: Compiled Output (`.so` files). *Ignored by git.*
- **obj/**: Intermediate Object files. *Ignored by git.*
- **build_native.bat**: Windows Batch script to automate NDK compilation.

## Hooking Strategies Implemented

The `main.cpp` currently implements three strategies (toggled by uncommenting/logic):

1.  **Dlsym Lookup**: Attempts to find the function by its exported symbol name.
2.  **Memory Pattern/Offset Scanner**: Scans `libgame.so` memory for pointers to the function name string to find the function table.
3.  **JNI VTable Intercept (Current)**: Hooks `JNIEnv->RegisterNatives` directly from the JNI Environment VTable. This intercepts any time the game tries to register native functions with Java.

## Current Status & Analysis (For Master)

**Problem:**
Despite successfully injecting the library and hooking `RegisterNatives`, the target function `NewGetWebsiteContent` is NOT appearing in the logs, while other functions (like `Cocos2dxDownloader`) are captured.

**Hypothesis: Race Condition (The "Too Late" Problem)**
We inject our library via `System.loadLibrary` inside the `onCreate` method of the main Activity.
However, it is highly likely that `libgame.so` is loaded **earlier**, specifically in the `<clinit>` (Static Initializer) block of the Java class.
Because `<clinit>` runs before `onCreate`, the game finishes registering its native functions *before* our hook is even loaded.

**Recommended Solution:**
1.  **Earlier Injection:** Move `System.loadLibrary("higgshook")` into the `static {}` block of the target class (smali modification).
2.  **Zygote/Libc Hooking:** If `<clinit>` is also too late (e.g., if loaded by a deeper dependency), we might need to hook `dlopen` in `libc.so` or use a wrapper library (like wrapping `libhoudini.so` or `libart.so`) to catch `libgame.so` exactly when it loads.
3.  **Manual Address Resolution:** Since patching injection points is tedious, a hardcoded offset (if found via Ghidra/IDA Static Analysis on the specific version) coupled with a base-address calculation at runtime is the most robust fallback.
