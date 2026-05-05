import json
data = json.load(open('version.json'))
print(f"-D XPTOUCH_FIRMWARE_VERSION=\\\"{data['version']}\\\"")
