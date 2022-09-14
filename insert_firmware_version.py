Import("env")

name = env.GetProjectOption("custom_firmware_name")
version = env.GetProjectOption("custom_firmware_version")
if "@" in name or "@" in version:
    raise ValueError(
        "'custom_firmware_name' and 'custom_firmware_version' may not contain '@' characters"
    )
name_version = f"{name}@{version}"
env.Replace(PROGNAME=name_version)
env.Append(CPPDEFINES={"FIRMWARE_VERSION": f'\\"{name_version}\\"'})
