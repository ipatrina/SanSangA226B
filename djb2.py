def djb2_hash(string):
    hash_value = 5381
    for char in string:
        hash_value = ((hash_value << 5) + hash_value) + ord(char)
    return hash_value & 0xFFFFFFFF

input_string = input("Input: ")
hash_hex = format(djb2_hash(input_string), '08x')

print(f"DJB2 Hash: {hash_hex}")

input("")