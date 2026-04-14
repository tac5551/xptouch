import json
import subprocess
import os
import hashlib
import shutil
import sys
Import("env")

env = DefaultEnvironment()
PIOENV = env.subst("$PIOENV")
BIN_CHANNEL = "s3_2.8"
CHIP_FAMILY = "ESP32-S3"
FLASH_SIZE = "16MB"
BOOTLOADER_OFFSET = "0x0"


def calculate_md5_and_size(file_path):
    md5_hash = hashlib.md5()
    try:
        with open(file_path, "rb") as file:
            chunk_size = 4096
            file_size = 0
            while True:
                chunk = file.read(chunk_size)
                if not chunk:
                    break
                md5_hash.update(chunk)
                file_size += len(chunk)
        md5_hex = md5_hash.hexdigest()
        return md5_hex, file_size
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return None, None


def build_path(*parts):
    return os.path.join(*parts)


def bin_channel_path(*parts):
    return build_path("..", "xptouch-bin", BIN_CHANNEL, *parts)


def build_output_path(*parts):
    return build_path(".", ".pio", "build", PIOENV, *parts)


def ensure_output_dirs():
    os.makedirs(bin_channel_path("ota"), exist_ok=True)
    os.makedirs(bin_channel_path("webusb"), exist_ok=True)
    os.makedirs(bin_channel_path("fw"), exist_ok=True)


def delete_bin_files(directory):
    try:
        for filename in os.listdir(directory):
            if filename.endswith(".bin"):
                file_path = os.path.join(directory, filename)
                os.remove(file_path)
                print(f"Deleted: {file_path}")
        print("All .bin files deleted successfully.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")


def post_build_create_ota_json(version_value):
    md5, _size = calculate_md5_and_size(bin_channel_path("ota", f"xptouch.{version_value}.bin"))
    ota = {
        "version": version_value,
        "url": f"https://tac-lab.tech/xptouch-bin/{BIN_CHANNEL}/ota/xptouch.{version_value}.bin",
        "md5": md5,
    }
    ota_serialized = json.dumps(ota, indent=2)
    with open(bin_channel_path("ota", "ota.json"), "w") as ota_file:
        ota_file.write(ota_serialized)


def post_build_manifest(version_value):
    webusb_manifest_fw_path = f"xptouch.web.{version_value}.bin"
    webusb_manifest = {
        "name": "xptouch",
        "version": version_value,
        "builds": [
            {
                "chipFamily": CHIP_FAMILY,
                "parts": [{"path": webusb_manifest_fw_path, "offset": 0}],
            }
        ],
    }
    webusb_manifest_serialized = json.dumps(webusb_manifest, indent=2)
    with open(bin_channel_path("webusb", "webusb.manifest.json"), "w") as webusb_manifest_file:
        webusb_manifest_file.write(webusb_manifest_serialized)


def post_build_copy_ota_fw(version):
    ota_bin_source = build_output_path("firmware.bin")
    ota_bin_target = bin_channel_path("ota", f"xptouch.{version}.bin")
    print(f"copy to ota : cp {ota_bin_source} {ota_bin_target}")
    shutil.copy(ota_bin_source, ota_bin_target)

    fw_bin_source = build_output_path("firmware.bin")
    fw_bin_target = bin_channel_path("fw", "firmware.bin")
    print(f"copy to fw : cp {fw_bin_source} {fw_bin_target}")
    shutil.copy(fw_bin_source, fw_bin_target)


def post_build_merge_bin(version):
    web_usb_fw = build_path("..", "..", "..", "..", "xptouch-bin", BIN_CHANNEL, "webusb", f"xptouch.web.{version}.bin")
    home = os.path.expanduser("~")
    esptool_py = os.path.join(home, ".platformio", "packages", "tool-esptoolpy", "esptool.py")
    esptool_cmd = [
        sys.executable,
        esptool_py,
        "--chip",
        CHIP_FAMILY,
        "merge_bin",
        "-o",
        web_usb_fw,
        "--flash_mode",
        "dio",
        "--flash_size",
        FLASH_SIZE,
        BOOTLOADER_OFFSET,
        "bootloader.bin",
        "0x8000",
        "partitions.bin",
        "0x10000",
        "firmware.bin",
    ]
    print(f"command {esptool_cmd}")
    subprocess.run(esptool_cmd, cwd=build_output_path())


def post_build_action(source, target, env):
    with open("version.json", "r") as version_file:
        version_data = json.load(version_file)
        version_value = version_data.get("version", "UNKNOWN")
    ensure_output_dirs()
    print(version_value)
    print(f"xptouch delete_bin_files {bin_channel_path('ota')}")
    delete_bin_files(bin_channel_path("ota"))
    print(f"xptouch delete_bin_files {bin_channel_path('webusb')}")
    delete_bin_files(bin_channel_path("webusb"))
    print("xptouch post_build_manifest")
    post_build_manifest(version_value)
    print("xptouch post_build_copy_ota_fw")
    post_build_copy_ota_fw(version_value)
    print("xptouch post_build_create_ota_json")
    post_build_create_ota_json(version_value)
    print("xptouch post_build_merge_bin")
    post_build_merge_bin(version_value)
    print("xptouch POSTBUILD")


env.AddPostAction("buildprog", post_build_action)
