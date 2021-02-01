import json
import time, hmac, hashlib
import requests
import glob
import os
import logging
import threading
import time

HMAC_KEY = "ddd3de11b2962ea92fa06bc85ca8f4a2"
API_KEY  = "ei_0e4a98eeb47411d002a1a55c871a65184651bde6278c0a35e23ef800c334eaec"

# Empty signature (all zeros). HS256 gives 32 byte signature, and we encode in hex, so we need 64 characters here
emptySignature = ''.join(['0'] * 64)


def get_x_filename(filename):
    m_codes = ['D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 'D11', 'D12', 'D13', 'D14', 'D15', 'D16', 'D17', 'D18', 'D19']
    f_codes = ['F01', 'F02', 'F03', 'F04', 'F05', 'F06', 'F07', 'F08', 'F09', 'F10', 'F11', 'F12', 'F13', 'F14', 'F15']
    code = filename.split('_')[0]
    
    label = ''

    if code in m_codes:
        label = 'Move'

    if code in f_codes:
        label = 'Fall'

    if label == '':
        raise Exception('label not found')

    x_filename = '{}.{}'.format(label, filename)
    return x_filename 

def upload(tid, files):
    for index, path in enumerate(files):
        filename = os.path.basename(path)
        values = []
        with open(path) as infile:
            skip = 2
            for line in infile:
                if skip == 0:
                    skip = 2
                    line = line.strip()
                    row  = line.replace(" ", "") 
                    cols = row.split(',')
                    ax = ((2 * 16) / (2 ** 13)) * float(cols[0]) * 9.81
                    ay = ((2 * 16) / (2 ** 13)) * float(cols[1]) * 9.81
                    az = ((2 * 16) / (2 ** 13)) * float(cols[2]) * 9.81
                    values.append([ax, ay, az])
                else:
                    skip = skip - 1
        if (len(values) == 0):
            continue
    
        data = {
            "protected": {
                "ver": "v1",
                "alg": "HS256",
                "iat": time.time() # epoch time, seconds since 1970
            },
            "signature": emptySignature,
            "payload": {
                "device_name": "ff:ee:dd:cc:bb:aa",
                "device_type": "generic",
                "interval_ms": 15.625,
                "sensors": [
                    { "name": "accX", "units": "m/s2" },
                    { "name": "accY", "units": "m/s2" },
                    { "name": "accZ", "units": "m/s2" }
                ],
                "values": values
            }
        }
    
        # encode in JSON
        encoded = json.dumps(data)
    
        # sign message
        signature = hmac.new(bytes(HMAC_KEY, 'utf-8'), msg = encoded.encode('utf-8'), digestmod = hashlib.sha256).hexdigest()
    
        # set the signature again in the message, and encode again
        data['signature'] = signature
    
        encoded = json.dumps(data)
    
        x_filename = get_x_filename(filename)
    
        # and upload the file
        headers = {
            'Content-Type': 'application/json',
            'x-file-name': x_filename,
            'x-api-key': API_KEY
        }
    
        url = 'https://ingestion.edgeimpulse.com/api/training/data'
        res = requests.post(url=url, data=encoded, headers=headers)
    
        if (res.status_code == 200):
            logging.info('Thread {}: Uploaded file {} to Edge Impulse {}'.format(tid, index, res.content))
        else:
            logging.info('Thread {}: Failed to upload file {} to Edge Impulse {}'.format(tid, index, res.content))
    

if __name__ == "__main__":
    format = "%(asctime)s: %(message)s"
    logging.basicConfig(format=format, level=logging.INFO,
                        datefmt="%H:%M:%S")

    paths = glob.glob("../SisFall_dataset/*/*.txt")
    div = 4
    n = int(len(paths) / div)
    threads = list()

    for i in range(div):
        if i ==  (div - 1):
            files = paths[n*i: ]
        else:
            files = paths[n*i: n*(i+1)]

        x = threading.Thread(target=upload, args=(i, files))
        threads.append(x)
        x.start()

    for thread in threads:
        thread.join()

    logging.info("Finished")
