import sys, os, struct
from pathlib import Path
Path(__file__).resolve()  # ensure module context is fine

# SHP(TD) writer: 1 frame, uncompressed (ShapeType=2), 64x48 8bpp.
# Usage: python tools\png_to_shptd.py <indexed_raw_or_png_via_magick> ...
# Better: python tools\png_to_shptd.py --raw <64x48_3072bytes> <out.shp>
# Or use ImageMagick externally to produce raw palette indices.

def write_shptd(out_path, width, height, pixels):
    assert len(pixels) == width * height
    num = 1
    # Offsets are relative to bytebuf+2 (start of offsets array). Extract_Shape: bytebuf+2+offset
    # offsets array = num*4 bytes, so frame starts at offset = num*4
    offsets = [num * 4]
    block = struct.pack('<H', num)
    for o in offsets:
        block += struct.pack('<i', o)
    # Shape_Type (pack 1): ShapeType(u16),Height(u8),Width(u16),OriginalHeight(u8),ShapeSize(u16),DataLength(u16),Colortable[16]
    datalen = width * height
    shapesize = 26 + datalen
    hdr = struct.pack('<H B H B H H', 2, height, width, height, shapesize, datalen)
    hdr += b'\x00' * 16
    p = Path(out_path)
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(block + hdr + bytes(pixels))

if __name__ == '__main__':
    if len(sys.argv) == 4 and sys.argv[1] == '--raw':
        pixels = Path(sys.argv[2]).read_bytes()
        if len(pixels) != 64*48:
            print(f'expected 3072 bytes, got {len(pixels)}'); sys.exit(2)
        write_shptd(sys.argv[3], 64, 48, pixels)
        print('wrote', sys.argv[3])
    else:
        print('usage: python tools/png_to_shptd.py --raw <64x48.gray.raw> <SBTRICON.SHP>')
        print('Raw = 3072 bytes of 8-bit palette indices, row-major, 64x48.')
        sys.exit(1)
