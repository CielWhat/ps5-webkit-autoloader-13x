#!/usr/bin/env python3
import sys
import os

def generate_c_array(input_file, output_file, array_name):
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found. Did you run 'npm run build'?")
        sys.exit(1)

    with open(input_file, 'rb') as f:
        data = f.read()

    with open(output_file, 'w') as f:
        f.write(f"/* Auto-generated from {input_file} */\n")
        f.write(f"#ifndef ASSETS_{array_name.upper()}_H\n")
        f.write(f"#define ASSETS_{array_name.upper()}_H\n\n")
        f.write(f"const unsigned char assets_{array_name}[] = {{\n")
        
        # Write data in hex format
        for i, byte in enumerate(data):
            if i % 12 == 0:
                f.write("\n    ")
            f.write(f"0x{byte:02x}, ")
        
        f.write("\n};\n\n")
        f.write(f"const unsigned int assets_{array_name}_len = {len(data)};\n\n")
        f.write(f"#endif\n")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: gen_assets.py <input> <output> <name>")
        sys.exit(1)
    
    generate_c_array(sys.argv[1], sys.argv[2], sys.argv[3])
