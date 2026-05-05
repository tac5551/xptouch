import json
data = json.load(open('version50.json'))
print(f"-D XPTOUCH_FIRMWARE_VERSION=\\\"{data['version']}\\\"")
