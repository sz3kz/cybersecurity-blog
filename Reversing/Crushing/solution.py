#!/usr/bin/env python

targetfile="message.txt.cz"
pos2char = dict()

with open(f"{targetfile}","rb") as file:
    charcode = 0
    while charcode < 256:
        list_length = int.from_bytes(file.read(8), "little")
        #print(f"Length of Linked List of charcode: 0x{charcode} -> {list_length}")
        for _ in range(0, list_length):
            position = int.from_bytes(file.read(8), "little")
            pos2char[position] = charcode
        charcode += 1

message = ""
message_last_position= max(pos2char.keys())
for position in range(0, message_last_position + 1):
    message += chr(pos2char[position])

print(f"{message}")
