from PIL import Image
import sys

img = Image.open(sys.argv[1])
varname = (len(sys.argv) > 2 and sys.argv[2]) or 'bitmap'
w, h = img.size
pix = img.convert('1')
rows = []
for y in range(h):
    if w <= 8:
        r = 'B' + ''.join(str(int(pix.getpixel((x,y))==0)) for x in range(w)).zfill(8)
    else:
        data = [int(pix.getpixel((x,y))==0) for x in range(w)]
        chunked = [data[i:i+8] for i in range(0, len(data), 8)]
        if len(sys.argv) > 3 and sys.argv[3]:
            r = ', '.join(hex(int(''.join(map(str, c)), 2)) for c in chunked)
        else:
            r = ', '.join('B' + ''.join(map(str, c)) for c in chunked)
    rows.append(r)
res = ',\n\t'.join(rows)
print(f'const unsigned char PROGMEM {varname}[] = {{\n\t{w}, {h},\n\t{res}\n}};')