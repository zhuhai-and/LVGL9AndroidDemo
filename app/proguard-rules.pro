# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Preserve line numbers for debugging stack traces
-keepattributes SourceFile,LineNumberTable
-renamesourcefileattribute SourceFile

# Keep JNI native methods and their containing class
-keep class com.hzy.lvgl.demo.LVApp { *; }
-keepclassmembers class com.hzy.lvgl.demo.LVApp {
    native <methods>;
}

# Keep all classes with native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep ViewBinding generated classes
-keep class com.hzy.lvgl.demo.databinding.** { *; }

# Keep model classes used in serialization
-keep class com.hzy.lvgl.demo.model.** { *; }
