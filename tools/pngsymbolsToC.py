import os
#import re
from PIL import Image
from enum import Enum

def png_to_hex_byte_array(png_file_path):
    """
    Converts a PNG file to a byte array in hexadecimal format and saves it as a .C file.

    Parameters:
    - png_file_path: str
        The path to the PNG file.

    Raises:
    - FileNotFoundError:
        If the specified PNG file does not exist.

    Returns:
    - str:
        The path to the generated .C file.
    """

    # Check if the PNG file exists
    if not os.path.exists(png_file_path):
        raise FileNotFoundError("The specified PNG file does not exist.")

    # Read the PNG file as binary data
    with open(png_file_path, "rb") as file:
        png_data = file.read()

    # Convert the binary data to a byte array in hexadecimal format
    hex_byte_array = " , ".join([f"0x{byte:02X}" for byte in png_data])

    # Create the .C file path
    c_file_path ="./symbols.h" #os.path.splitext(png_file_path)[0] + ".h"
    array_name = str(os.path.splitext(png_file_path)[0])

    # Save the byte array as a .C file
    with open(c_file_path, "a") as file:
        file.write(f"\nstatic const unsigned char symbol_{array_name}[] PROGMEM = {{ {hex_byte_array} }};")
        #file.write("static const unsigned char symbol_{ array_name } [] PROGMEM = {{ {hex_byte_array} }};")
        #file.write("\n");
        
    
    return c_file_path
# Example usage:
#print(f"The PNG file has been converted to a byte array in hexadecimal format and saved as {c_file_path}.")

def join_with_limit(arr, limit):
    result = ''
    count = 0

    for item in arr:
        result += str(item) + ', '
        count += 1

        if count == limit:
            result += '\n  '
            count = 0

    return result.rstrip(', ')



sym_arrays = "" #for symbol data hex arrays
sym_size_array ="" # for sizeof() array of each symbol
#here id:s from 1-199 correspond fmi symbol id
symbol_map_index= [-1] * 201 #for mapping symbol with pointer id
count = 0
tot_count = 0
file_list = sorted(os.listdir("./"),key=os.path.basename ) 
#orted_list = sorted(file_list,key=os.path.basename).sort()
#file_list =os.listdir("./") 
#file_list.sort(key=lambda f: int(''.join(filter(str.isdigit, f))))

for  file in file_list:
    if file.endswith(".png"):
        c_file_path = png_to_hex_byte_array(file)
        symnumber = int(os.path.splitext(file)[0])
        symbol_map_index[symnumber] = tot_count 
        sym_arrays = sym_arrays +"symbol_"+ str(os.path.splitext(file)[0]) + ", "
        sym_size_array = sym_size_array + f"sizeof(symbol_{str(os.path.splitext(file)[0])}), "
        count = count +1
        tot_count = tot_count +1
        if count == 10:
            sym_arrays = sym_arrays + "\n  "
            sym_size_array = sym_size_array + "\n  " 
            count =0

sym_arrays = sym_arrays[:len(sym_arrays)-2]
sym_size_array = sym_size_array[:len(sym_size_array)-2]

c_pointer_path = "./symbols.h"
#symbol_map_index_str = ",".join(map(str,symbol_map_index))
symbol_map_index_str = join_with_limit(symbol_map_index,10)
with open(c_pointer_path, "a") as file:
    file.write(f"\n\nPROGMEM const unsigned char* const fmisymbols[90] =\n{{ \n  {sym_arrays} \n}};")
    file.write(f"\n\nPROGMEM const long  fmisizes[90] =\n{{ \n  {sym_size_array} \n}};")
    file.write(f"\n\nPROGMEM const long  getSymbol[201] =\n{{ \n  {symbol_map_index_str} \n}};")



