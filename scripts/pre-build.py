import json
import subprocess
import os
import hashlib
import time
import sys

Import("env")

env = DefaultEnvironment()

flag_file = ".prebuild_executed"

# MarkFlag機能: Prebuildが一度だけ実行されるようにする
def is_prebuild_already_executed():
    """ファイルベースでPrebuildが既に実行されたかチェック"""
    if os.path.exists(flag_file):
        # ファイルの作成時刻をチェック（5分以内なら有効）
        file_time = os.path.getmtime(flag_file)
        current_time = time.time()
        if current_time - file_time < 300:  # 5分 = 300秒
            return True
        else:
            # 古いフラグファイルを削除
            os.remove(flag_file)
    return False

def mark_prebuild_executed():
    """Prebuild実行済みをマーク"""
    with open(flag_file, 'w') as f:
        f.write(str(time.time()))

def clear_prebuild_flag():
    """Prebuildフラグをクリア"""
    if os.path.exists(flag_file):
        os.remove(flag_file)
        print(f"Cleared prebuild flag: {flag_file}")

# ターゲット名を安全に取得する関数
def get_current_target():
    # デフォルトは build
    target = "build"
    
    try:
        # 明示的に検出できるターゲットのみチェック
        
        # 方法1: 環境変数から取得
        env_target = os.environ.get('PIO_TARGET')
        if env_target and env_target not in ['NOT_SET', '0', '1']:
            target = env_target
        
        # 方法2: コマンドライン引数から明示的なターゲットを検出
        for i, arg in enumerate(sys.argv):
            # -t オプションの次の引数を取得
            if arg == '-t' and i + 1 < len(sys.argv):
                target = sys.argv[i + 1]
                break
            # --target オプションの次の引数を取得
            elif arg.startswith('--target='):
                target = arg.split('=', 1)[1]
                break
            # --clean オプションを検出
            elif arg == '--clean':
                target = 'clean'
                break
        
        # 方法3: コマンドライン引数から直接ターゲットを検出
        for arg in sys.argv:
            if arg in ['build', 'upload', 'clean', 'cleanall', 'monitor']:
                target = arg
                break
        
        return target
        
    except Exception as e:
        print(f"Warning: Could not determine target: {e}")
        return "build"

# 現在のターゲットを取得
current_target = get_current_target()

# デバッグ情報を出力
print(f"============================CURRENT TARGET: {current_target}===========================")
# print(f"Environment variables: PIO_TARGET={os.environ.get('PIO_TARGET', 'NOT_SET')}")

# コマンドライン引数からターゲット関連の情報のみ表示
target_args = [arg for arg in sys.argv if arg in ['build', 'upload', 'clean', 'cleanall', 'monitor', '-t', '--clean'] or arg.startswith('--target=')]
if target_args:
    print(f"Target-related args: {target_args}")

print(f"============================PREBUILD START=============================")
flag_file = ".prebuild_executed"

# Make sure 'vscode init' is not the current command
def is_pio_build():
    from SCons.Script import DefaultEnvironment
    env = DefaultEnvironment()
    return not env.IsIntegrationDump()

# 特定のターゲットかどうかを判定する関数
def is_target(target_name):
    return current_target == target_name

def is_upload_target():
    return is_target("upload")

def is_build_target():
    return is_target("build")

def is_clean_target():
    return is_target("clean") or is_target("cleanall")




def post_build_increment_semver(json_file, bump_type="patch"):
    # Load the JSON from the file
    with open(json_file, 'r') as f:
        data = json.load(f)

    # Get the current version from the JSON
    version = data.get("version", "0.0.1").split(".")

    # Increment the version based on the bump type (major, minor, or patch)
    if bump_type == "major":
        version[0] = str(int(version[0]) + 1)
        version[1] = "0"
        version[2] = "0"
    elif bump_type == "minor":
        version[1] = str(int(version[1]) + 1)
        version[2] = "0"
    elif bump_type == "patch":
        version[2] = str(int(version[2]) + 1)
    else:
        raise ValueError("Bump type must be 'major', 'minor', or 'patch'.")

    # Update the version in the JSON
    data["version"] = ".".join(version)
    print(f"xptouch version: {data['version']}")
    # Save the updated JSON to the file
    with open(json_file, 'w') as f:
        json.dump(data, f, indent=2)

def prebuild():
    
    
    # ターゲット別の処理例
    if is_upload_target():
        print("Upload target detected -Noting to do")
    elif is_build_target():
        print(f"============================★★ XTOUCH PREBUILD ★★==========================")
        print("Build target detected ")
        # MarkFlagチェック: 既に実行済みかチェック
        if is_prebuild_already_executed():
            print("Prebuild already executed - skipping")
            return

        result = subprocess.run(['node', 'scripts/download-errors.js'],
                                text=True, check=True, capture_output=True)
        print(result.stdout)

        print(f"xptouch post_build_increment_semver")
        post_build_increment_semver("version.json", bump_type="patch")

        # 実行完了をマーク
        print("Marking prebuild as executed")
        mark_prebuild_executed()

    elif is_clean_target():
        print(f"============================★★ XTOUCH CLEAN ★★==========================")
        print("Clean target detected - clear flag file")
        clear_prebuild_flag()
        return
    
    else:
        print(f"Unknown target '{current_target}' - nothing to do")
        return

if is_pio_build() == True:
    prebuild()

print(f"============================PREBUILD END  =============================")