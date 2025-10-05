# Keep JNI bridge methods
-keep class com.example.edgeviewer.NativeBridge { *; }

# Keep native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Reduce noise
-dontwarn java.nio.**
-dontwarn javax.annotation.**

