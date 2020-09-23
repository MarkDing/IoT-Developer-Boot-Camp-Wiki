#!/usr/bin/python
import json
import subprocess

class OTAUpdate(object):
    def __init__(self):
        with open('configuration.json') as self.file:
            object = self.file.read()
        jsonData = json.loads(object)
        self.process_ota(jsonData[jsonData['protocol']], jsonData["commander"])
        self.file.close()
        return
       
    def process_ota(self, protocol, commander):
        self.update_device(protocol, commander)
        self.ota_update(protocol["hostApp"])

    def erase_device(self, commander):
        if(commander["erase"][0] != "-"):
            cmd_line = commander["path"] + " " + commander["erase"] + " "
            for dev in commander["wstk"]:
                cmd = cmd_line + dev
                print(cmd)
                try:
                    result = subprocess.run(cmd)
                    result.check_returncode()
                except subprocess.CalledProcessError:
                    print(result)
                    return -1
        else:
            print("No device need to be erased")
        return

    def program_image(self, cmd_line, filename):
        cmd = cmd_line + " " + filename
        print(cmd)
        try:
            result = subprocess.run(cmd)
            result.check_returncode()
        except subprocess.CalledProcessError:
            print(result)
            return -1

    def flash_device(self, protocol, commander):
        cmd_line = commander["path"] + " " + commander["flash"]
        if(protocol["server"][0] != "-"):
            cmd = cmd_line + " " + commander["wstk"][0]
            self.program_image(cmd, protocol["server"])
            self.program_image(cmd, protocol["bootloader"])

        if(protocol["client"][0] != "-"):
            cmd = cmd_line + " " + commander["wstk"][1]
            self.program_image(cmd, protocol["client"])
            self.program_image(cmd, protocol["bootloader"])
        return

    def update_device(self, protocol, commander):
        self.erase_device(commander)
        self.flash_device(protocol, commander)
        return

    def ota_update(self, host):
        cmd = host["path"]  +  " " + host["params"]
        print(cmd)
        try:
            result = subprocess.run(cmd)
            result.check_returncode()
        except subprocess.CalledProcessError:
            print(result)
            return -1
        return 0

def main():
    OTAUpdate();
    print('This message is from main function')

if __name__ == "__main__":
    main()

