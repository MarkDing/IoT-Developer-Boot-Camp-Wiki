# -*- coding: UTF-8 -*-
import sys
import binascii
import struct
import hashlib
from ecdsa import SigningKey, VerifyingKey, BadSignatureError

def zigbee_ota_signing(bin_file, priv_pem):
    # check if valid
    with open(bin_file, 'rb+') as fp:
        filedata = fp.read(2*1024*1024)
        bytesdata = bytearray(filedata)

        print(len(bytesdata))

        # check magic word
        magicwd = struct.unpack_from("I", bytesdata, 0)
        if magicwd[0] != 0x0BEEF11E:
            print("Magic word of OTA header is invalid")
            fp.close()
            return

        headerlen = struct.unpack_from("<H", bytesdata, 6)
        headerlen = headerlen[0]
        print(headerlen)

        totallen = struct.unpack_from("<I", bytesdata, 52)
        totallen = totallen[0]
        print(totallen)

        # check if it's already signed
        offset = headerlen
        while True:
            tag = struct.unpack_from("<H", bytesdata, offset)
            tag = tag[0]

            offset = offset + 2
            tag_len = struct.unpack_from("<I", bytesdata, offset)
            tag_len = tag_len[0]

            offset = offset + 4
            offset = offset + tag_len

            print("tag=%d len=%d offset=%d" % (tag, tag_len, offset))

            if 1 == tag or 5 == tag:
                print("OTA file is already signed!")
                fp.close()
                return

            if offset >= totallen:
                break

        # update header, add a new tag to fill the signature. tag(2) + length(4) + signature(64)
        totallen = totallen + 70
        struct.pack_into("<I", bytesdata, 52, totallen)

        with open(priv_pem) as fp_key:
            sk = SigningKey.from_pem(fp_key.read())
            fp_key.close()

        if sk is None:
            print("signing key file is invalid")
            fp.close()
            return

        sig = sk.sign(bytesdata, hashfunc=hashlib.sha256)
        print(binascii.b2a_hex(sig))
        print(len(bytearray(sig)))

        sigtag = bytearray(6)
        struct.pack_into("<H", sigtag, 0, 0x0001)
        struct.pack_into("<I", sigtag, 2, 64)
        sigtag = sigtag + bytearray(sig)
        bytesdata = bytesdata + sigtag

        fp.seek(0, 0)
        fp.write(bytesdata)
        fp.close()
        print("Sign OTA file successful!");


def zigbee_ota_sig_verify(bin_file, pub_pem):
    with open(bin_file, 'rb+') as fp:
        filedata = fp.read(2*1024*1024)
        bytesdata = bytearray(filedata)

        totallen = len(bytesdata)

        data = bytesdata[:totallen-70]
        signature = bytesdata[-64:]

        print("signature:")
        print(binascii.b2a_hex(signature))

        vk = VerifyingKey.from_pem(open(pub_pem).read())
        try:
            vk.verify(signature, data, hashfunc=hashlib.sha256)
            print("good signature")
        except BadSignatureError:
            print("BAD SIGNATURE")

if __name__ == '__main__':
    helperTexts = \
        "\nOVERVIEW: Zigbee OTA File Signing Tool\n" \
        "USAGE: \n" \
        "    zigbee-ota-sign sign <binary file> <private_key_pem>\n" \
        "    zigbee-ota-sign verify <binary file> <public_key_pem>\n"
    if len(sys.argv) < 4:
        print(helperTexts)
    else:
        bin_file=sys.argv[2]
        priv_pem=sys.argv[3]

        if sys.argv[1] == "sign":
            zigbee_ota_signing(bin_file, priv_pem)
        elif sys.argv[1] == "verify":
            zigbee_ota_sig_verify(bin_file, priv_pem)
        else:
            print(helperTexts)
