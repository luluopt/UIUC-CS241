import os

extCount = 0
taken = ""
defs = open("extdefs.h", "w")
hooks = open("extensions.h", "w")

def parseAndAppend(filename):
  global taken
  f = open(filename, "r"); 
  lines = f.readlines()
  author = lines[0].split("Author:")[1].strip()
  key_codes = lines[1].split("Key_codes:")[1].strip().replace("'", "")
  key_codes = key_codes.replace(" ", "")
  key_codes = key_codes.split(",")  

  f_name = lines[2].split("F_name:")[1].strip().replace("'", "")
  for i in range(len(lines)):
    lines[i] = lines[i].replace(f_name, "ext_{0}".format(extCount))

  for k in key_codes:
    if k in taken:
      print("Conflicting keycode in file {0}.".format(filename))
      return
  taken += "".join(key_codes)
  defs.write("".join(lines).replace("EXT_NO", str(extCount)))
  defs.write("\n")
  for k in key_codes:
    hooks.write("""case '{0}': {{\\\n\t ext_{1}(document, display, buffer, '{0}');\\\n}} break;\\\n""".format(k, extCount))
  print("loaded extension {0}".format(f));

if __name__ == "__main__":
  defs.write("#pragma once\n")
  defs.write('#include "display.h"\n')
  defs.write('#include "document.h"\n')
  defs.write('#include "../editor.h"\n')
  hooks.write("")
  hooks.write("#pragma once\n")
  hooks.write('#include "extdefs.h"\n')
  hooks.write('#define EXTENSIONS(c) ({\\\n')
  os.chdir("ext_src")
  l = os.listdir()
  os.chdir("..")
  for f in l:
    if f[-2:] == ".c":
      parseAndAppend("ext_src/"+f)
      extCount+=1
      if(extCount==100):
        print("Reached 100 extensions! Stopping now.")
        exit(1)
  defs.close()
  hooks.write('})')

