# Real-Time Edge Detection Viewer (Android + OpenCV C++ + OpenGL ES + Web)

This repository contains a minimal, modular setup to build an Android app that captures camera frames, processes them in native C++ using OpenCV (via JNI), and renders the result using OpenGL ES. It also includes a tiny TypeScript web viewer to display a sample processed frame and basic stats.

## Tech Stack
- Android (Kotlin)
- NDK + JNI (C++)
- OpenCV (C++)
- OpenGL ES 2.0+
- TypeScript (web viewer)

## Project Structure

```
.
├─ android/                 # Android Gradle project
│  ├─ app/
│  │  ├─ src/main/java/...  # Kotlin sources
│  │  ├─ src/main/cpp/      # Thin JNI bridge that links to ../jni and ../gl
│  │  └─ src/main/res/      # Android resources
│  ├─ build.gradle          # (Project-level)
│  └─ settings.gradle
├─ jni/                     # Native C++ sources using OpenCV (image processing)
│  ├─ CMakeLists.txt
│  └─ src/
│     ├─ opencv_pipeline.cpp
│     └─ opencv_pipeline.hpp
├─ gl/                      # OpenGL ES renderer (C++)
│  ├─ CMakeLists.txt
│  └─ src/
│     ├─ gl_renderer.cpp
│     └─ gl_renderer.hpp
├─ web/                     # TypeScript web viewer
│  ├─ package.json
│  ├─ tsconfig.json
│  ├─ public/index.html
│  └─ src/index.ts
└─ .gitignore
```

## Architecture (High Level)
- Kotlin layer (Android): camera access (CameraX or Camera2) and UI wiring. Frames are provided to the native layer via JNI.
- C++ layer (JNI): receives frames, uses OpenCV to apply a filter (Canny/Grayscale). Returns processed pixels or uploads to a GL texture.
- C++ OpenGL ES: renders processed frames to a Surface using a simple textured quad.
- TypeScript viewer: standalone page to display a static/sample processed frame (e.g., Base64 PNG) and overlay basic stats.

## Build Instructions (Initial)
Android:
1. Open the `android/` folder in Android Studio (latest stable).
2. Ensure NDK and CMake are installed via SDK Manager.
3. Sync Gradle and run the app on a device.

Web:
1. `cd web`
2. `npm install`
3. `npm run build` (outputs to `dist/`) or `npm run dev` for a simple static server.

## OpenCV Setup (Optional, Enables Native Canny)
This project can build and run without OpenCV (it will fall back to grayscale in native code). To enable native OpenCV Canny:

1. Install OpenCV for Android (OpenCV SDK, e.g., 4.x). Note the `OpenCV_DIR` path that contains `OpenCVConfig.cmake`.
2. In your local build configuration, set `OpenCV_DIR` when configuring CMake. For Android Studio, you can set an environment variable or pass via Gradle CMake arguments.
   - Example (environment): set `OpenCV_DIR` to `.../OpenCV-android-sdk/sdk/native/jni`.
3. Rebuild. The build will define `EDGEVIEWER_USE_OPENCV` and link `core` and `imgproc`.

Notes:
- If `OpenCV_DIR` is not provided, the build uses a CPU grayscale fallback for Canny.
- Ensure ABI compatibility (e.g., `arm64-v8a`, `armeabi-v7a`) between your OpenCV prebuilt libs and app ABIs.

## Run-time Notes
- Tap the status text to capture one frame, process to grayscale via JNI, and save a PNG to `Pictures/EdgeViewer`.
- Use the Mode button to toggle between GRAY and CANNY. The renderer shows FPS. When OpenCV is configured, CANNY uses native OpenCV; otherwise it falls back gracefully.

## Roadmap toward Assessment Requirements
This repo will evolve over ~50 commits to keep history granular and meaningful.

Planned milestones:
1. Scaffold repo and minimal Android+NDK+Web skeleton (this commit)
2. Wire JNI bridge and CMake targets for `jni/` and `gl/`
3. Add simple OpenGL renderer with a solid-color texture
4. Integrate OpenCV minimal pipeline (grayscale)
5. Add Canny edge detection and buffer hand-off
6. Camera feed via TextureView/Camera2; push frames to JNI
7. Render processed frames to GL texture at 10–15 FPS+
8. Add toggle: raw vs processed
9. Add FPS stats overlay and logging
10. Export a sample processed frame for web viewer
11. Final polish and documentation

## Evaluation Mapping
- Native-C++ integration (JNI): JNI bridge, safe buffer handling, documented interfaces.
- OpenCV usage: minimal copies, correct color models, Canny/grayscale pipelines.
- OpenGL rendering: textured quad, shader program, efficient updates.
- TypeScript web viewer: clean `tsconfig`, DOM updates, basic FPS/resolution text.
- Structure & docs: modular directories, CMake targets, clear README and commit history.

## License
MIT


