import json
data = json.load(open('version50.json'))
print(f"-D XTOUCH_FIRMWARE_VERSION=\\\"{data['version']}\\\"")
