import os
import sys
import subprocess
import shutil
import zipfile

def main():
    temp_dir = os.environ.get("TEMP", "C:/Users/Thedr/AppData/Local/Temp")
    aapt2_exe = os.path.join(temp_dir, "aapt2_bin", "aapt2.exe")
    android_jar = os.path.join(temp_dir, "android.jar")
    r8_jar = os.path.join(temp_dir, "r8.jar")
    uber_signer_jar = os.path.join(temp_dir, "uber-apk-signer.jar")
    java_exe = "C:/Program Files/Eclipse Adoptium/jdk-21.0.13.9-hotspot/bin/java.exe"
    javac_exe = "C:/Program Files/Eclipse Adoptium/jdk-21.0.13.9-hotspot/bin/javac.exe"

    work_dir = os.path.join(temp_dir, "apk_packaging")
    if os.path.exists(work_dir):
        shutil.rmtree(work_dir)
    os.makedirs(work_dir, exist_ok=True)

    compiled_res_dir = os.path.join(work_dir, "compiled_res")
    os.makedirs(compiled_res_dir, exist_ok=True)

    print("[1/6] Compiling resources with aapt2...")
    res_root = os.path.abspath("android/app/src/main/res")
    flat_files = []
    for root, dirs, files in os.walk(res_root):
        for f in files:
            full_path = os.path.join(root, f)
            cmd = [aapt2_exe, "compile", full_path, "-o", compiled_res_dir]
            subprocess.run(cmd, check=True)

    for f in os.listdir(compiled_res_dir):
        if f.endswith(".flat"):
            flat_files.append(os.path.join(compiled_res_dir, f))

    print(f"[2/6] Linking {len(flat_files)} resources into base APK...")
    manifest_path = os.path.abspath("android/app/src/main/AndroidManifest.xml")
    base_apk = os.path.join(work_dir, "base.apk")
    link_cmd = [
        aapt2_exe, "link",
        "-I", android_jar,
        "--manifest", manifest_path,
        "-o", base_apk,
        "--auto-add-overlay"
    ] + flat_files
    subprocess.run(link_cmd, check=True)

    print("[3/6] Compiling Java Activity to DEX with D8...")
    java_src = os.path.abspath("android/app/src/main/java/com/thedreadlorddeath/flappybird/MainActivity.java")
    class_out_dir = os.path.join(work_dir, "classes")
    os.makedirs(class_out_dir, exist_ok=True)
    subprocess.run([javac_exe, "--release", "8", "-cp", android_jar, "-d", class_out_dir, java_src], check=True)

    dex_out_dir = os.path.join(work_dir, "dex")
    os.makedirs(dex_out_dir, exist_ok=True)
    class_file = os.path.join(class_out_dir, "com", "thedreadlorddeath", "flappybird", "MainActivity.class")
    subprocess.run([java_exe, "-cp", r8_jar, "com.android.tools.r8.D8", "--lib", android_jar, "--output", dex_out_dir, class_file], check=True)

    print("[4/6] Staging complete APK filesystem...")
    stage_dir = os.path.join(work_dir, "stage")
    os.makedirs(stage_dir, exist_ok=True)

    # Unpack base.apk into stage
    with zipfile.ZipFile(base_apk, 'r') as z:
        z.extractall(stage_dir)

    # Copy classes.dex
    shutil.copy2(os.path.join(dex_out_dir, "classes.dex"), os.path.join(stage_dir, "classes.dex"))

    # Copy native libraries
    extlibs_android = os.path.abspath("build/_deps/sfml-src/extlibs/libs-android")
    for abi in ["arm64-v8a", "armeabi-v7a", "x86_64"]:
        abi_dir = os.path.join(stage_dir, "lib", abi)
        os.makedirs(abi_dir, exist_ok=True)
        src_so = os.path.join(extlibs_android, abi, "libopenal.so")
        if os.path.exists(src_so):
            shutil.copy2(src_so, os.path.join(abi_dir, "libopenal.so"))

    print("[5/6] Writing unaligned APK with forward-slash paths...")
    unaligned_apk = os.path.join(work_dir, "unaligned.apk")
    with zipfile.ZipFile(unaligned_apk, 'w', zipfile.ZIP_DEFLATED) as z:
        for root, dirs, files in os.walk(stage_dir):
            for file in files:
                abs_path = os.path.join(root, file)
                rel_path = os.path.relpath(abs_path, stage_dir)
                forward_slash_path = rel_path.replace("\\", "/")
                z.write(abs_path, forward_slash_path)

    print("[6/6] Zipaligning and signing with uber-apk-signer (v1, v2, v3)...")
    out_apk_dir = os.path.abspath("apk")
    os.makedirs(out_apk_dir, exist_ok=True)
    subprocess.run([
        java_exe, "-jar", uber_signer_jar,
        "-a", unaligned_apk,
        "-o", out_apk_dir,
        "--allowResign"
    ], check=True)

    # Find the resulting signed apk and rename to FlappyBird-SunsetArcade.apk
    for f in os.listdir(out_apk_dir):
        if f.endswith("-aligned-debugSigned.apk"):
            src_path = os.path.join(out_apk_dir, f)
            final_path = os.path.join(out_apk_dir, "FlappyBird-SunsetArcade.apk")
            if os.path.exists(final_path):
                os.remove(final_path)
            os.rename(src_path, final_path)
            print(f"Success! Final signed APK: {final_path} ({os.path.getsize(final_path)} bytes)")

        if f.endswith(".idsig"):
            os.remove(os.path.join(out_apk_dir, f))

    print("APK build and signing completed with 0 errors!")

if __name__ == "__main__":
    main()
