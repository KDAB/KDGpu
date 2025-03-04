/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
plugins {
    id("com.android.application")
}

android {
    namespace = "com.kdab.dynamic_ubo"
    compileSdk = 34

    defaultConfig {
        ndk {
            abiFilters += mutableSetOf("arm64-v8a")
        }
        applicationId = "com.kdab.dynamic_ubo"
        minSdk = 32
        targetSdk = 32
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    buildFeatures {
        prefab = true
    }
    externalNativeBuild {
        cmake {
            path = file("../CMakeLists.txt")
            version = "3.22.1"
        }
    }
    ndkVersion = "26.1.10909125"
}

dependencies {

    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.11.0")
    implementation("androidx.games:games-activity:1.2.2")
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
}


// Task to copy files from outer project to android shader compilation folder
tasks.register<Copy>("copyShaders") {
    from("../../../../assets/shaders/examples/dynamic_ubo")
    into("src/main/shaders/examples/dynamic_ubo")
}

// Task to copy font files
tasks.register<Copy>("copyFonts") {
    from("../../../../assets/fonts")
    into("src/main/assets/fonts")
}

// Task to copy texture files
tasks.register<Copy>("copyTextures") {
    from("../../../../assets/textures")
    into("src/main/assets/textures")
}

// Ensure the copyShaders task runs before the build task
tasks.named("preBuild") {
    dependsOn("copyShaders", "copyFonts", "copyTextures")
}
