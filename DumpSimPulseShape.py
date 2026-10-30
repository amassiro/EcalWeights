#
# Dump EcalSimPulseShape from the conditions DB via Frontier (no CMSSW needed)
#
#   python3 DumpSimPulseShape.py -t EcalSimPulseShapes_TB2025_v1 -o shape_TB2025_v1.txt
#
# Payload is boost/serialization "portable binary archive" (eos::portable_oarchive):
#   - every integer/float is stored as: 1 signed byte = number of bytes (LE), then the bytes
#   - std::vector: count, item_version, elements
#
# EcalSimPulseShape members (CondFormats/EcalObjects/interface/EcalSimPulseShape.h), in order:
#   barrel_shape, endcap_shape, apd_shape (vector<double>)
#   barrel_thresh, endcap_thresh, apd_thresh (double)
#   time_interval (float)
#

import argparse
import base64
import re
import struct
import urllib.request
import zlib


# ---- Frontier ----

def frontierQuery(sql, server):
  enc = base64.b64encode(zlib.compress(sql.encode())).decode()
  enc = enc.replace('+', '.').replace('/', '-').replace('=', '_')
  url = server + "?type=frontier_request:1:DEFAULT&encoding=BLOBzip5&p1=" + enc
  req = urllib.request.Request(url, headers = {"X-Frontier-Id": "DumpSimPulseShape"})
  xml = urllib.request.urlopen(req, timeout = 60).read().decode()
  quality = re.search(r"<quality[^>]*/>", xml).group(0)
  if 'error="0"' not in quality:
    print ("Frontier error: ", quality)
    return None
  return zlib.decompress(base64.b64decode(re.search(r"<data>(.*?)</data>", xml, re.S).group(1)))


#
# Frontier binary stream: 0x06 = string/blob (4 bytes BE length + data), 0x07 = end of row
# the first 2*nColumns fields are column names and types
#
def parseFrontierRows(raw, nColumns):
  fields = []
  rows = []
  pos = 0
  while pos < len(raw):
    code = raw[pos]
    pos += 1
    if code == 0x06:
      length = struct.unpack(">I", raw[pos:pos+4])[0]
      pos += 4
      fields.append(raw[pos:pos+length])
      pos += length
    elif code == 0x07:
      rows.append(fields)
      fields = []
    else:
      break
  # first "row" has the header (names and types) merged with the first data row
  header_and_first = rows[0]
  rows[0] = header_and_first[2*nColumns:]
  return [row for row in rows if len(row) == nColumns]


# ---- portable binary archive ----

class PortableArchive:
  def __init__(self, blob):
    self.blob = blob
    self.pos = 0

  def readSize(self):
    s = struct.unpack("b", self.blob[self.pos:self.pos+1])[0]
    self.pos += 1
    return s

  def readInt(self):
    s = self.readSize()
    if s == 0:
      return 0
    n = abs(s)
    v = int.from_bytes(self.blob[self.pos:self.pos+n], "little")
    self.pos += n
    return -v if s < 0 else v

  def readFloatingPoint(self, nbytes, fmt):
    s = self.readSize()
    if s == 0:
      return 0.0
    v = self.blob[self.pos:self.pos+s] + b"\x00" * (nbytes - s)
    self.pos += s
    return struct.unpack(fmt, v)[0]

  def readDouble(self):
    return self.readFloatingPoint(8, "<d")

  def readFloat(self):
    return self.readFloatingPoint(4, "<f")

  def readVectorDouble(self):
    n = self.readInt()
    item_version = self.readInt()
    return [self.readDouble() for i in range(n)]


def decodeEcalSimPulseShape(blob):
  ar = PortableArchive(blob)
  signature = ar.blob[0]
  ar.pos = 1
  if signature != 0x7f:
    print ("Not a portable binary archive, signature = ", hex(signature))
  library_version = ar.readInt()
  # top level object: tracking + class version
  ar.readInt()
  ar.readInt()

  shape = {}
  shape["barrel_shape"]  = ar.readVectorDouble()
  shape["endcap_shape"]  = ar.readVectorDouble()
  shape["apd_shape"]     = ar.readVectorDouble()
  shape["barrel_thresh"] = ar.readDouble()
  shape["endcap_thresh"] = ar.readDouble()
  shape["apd_thresh"]    = ar.readDouble()
  shape["time_interval"] = ar.readFloat()

  if ar.pos != len(blob):
    print ("WARNING: decoded ", ar.pos, " bytes out of ", len(blob))
  return shape


if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument("-t", "--tag",    default = "EcalSimPulseShapes_TB2025_v1", help = "conditions tag")
  parser.add_argument("-o", "--output", default = "", help = "output text file (time, barrel, endcap, apd)")
  parser.add_argument("-s", "--server", default = "http://cmsfrontier.cern.ch:8000/FrontierProd/Frontier", help = "Frontier server")
  args = parser.parse_args()

  #
  # IOVs of the tag
  #
  raw = frontierQuery("SELECT SINCE, PAYLOAD_HASH FROM CMS_CONDITIONS.IOV WHERE TAG_NAME='" + args.tag + "' ORDER BY SINCE", args.server)
  iovs = parseFrontierRows(raw, 2)
  print ("tag = ", args.tag, " number of IOVs = ", len(iovs))
  for since, payload_hash in iovs:
    print ("   since = ", since.decode(), " hash = ", payload_hash.decode())

  # FIXME: only the last IOV is dumped
  payload_hash = iovs[-1][1].decode()

  raw = frontierQuery("SELECT OBJECT_TYPE, STREAMER_INFO, DATA FROM CMS_CONDITIONS.PAYLOAD WHERE HASH='" + payload_hash + "'", args.server)
  object_type, streamer_info, data = parseFrontierRows(raw, 3)[0]
  print ("object type = ", object_type.decode())
  print ("streamer info = ", streamer_info.decode().strip())

  if object_type.decode() != "EcalSimPulseShape":
    print ("Unsupported object type")
    exit(1)

  shape = decodeEcalSimPulseShape(data)

  print ("time_interval = ", shape["time_interval"])
  print ("thresholds (barrel, endcap, apd) = ", shape["barrel_thresh"], shape["endcap_thresh"], shape["apd_thresh"])
  for name in ["barrel_shape", "endcap_shape", "apd_shape"]:
    values = shape[name]
    if len(values) == 0:
      print (name, " : empty")
      continue
    imax = values.index(max(values))
    print (name, " : n = ", len(values), " max = ", values[imax], " at index ", imax, " (t = ", imax * shape["time_interval"], ")")

  if args.output != "":
    n = max(len(shape["barrel_shape"]), len(shape["endcap_shape"]), len(shape["apd_shape"]))
    with open(args.output, "w") as f:
      f.write("# tag " + args.tag + " hash " + payload_hash + "\n")
      f.write("# time   barrel   endcap   apd \n")
      for i in range(n):
        line = f"{i * shape['time_interval']:.4f}"
        for name in ["barrel_shape", "endcap_shape", "apd_shape"]:
          line += f"  {shape[name][i]:.7g}" if i < len(shape[name]) else "  0"
        f.write(line + "\n")
    print ("written: ", args.output)
