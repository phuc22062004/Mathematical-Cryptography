#Note encryption and decryption

from Crypto.Cipher import AES  # pip install pycryptodome
import base64
import os

# -*- coding: utf-8 -*-
import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

def encrypt_note(note, key):
    try:
        cipher = AES.new(key, AES.MODE_GCM)
        nonce = cipher.nonce
        ciphertext, tag = cipher.encrypt_and_digest(note.encode('utf-8'))
        # Combine nonce, ciphertext, and tag
        encrypted_data = nonce + ciphertext + tag
        return base64.b64encode(encrypted_data).decode('utf-8')
    except Exception as e:
        raise Exception(f"Encryption error: {str(e)}")

def decrypt_note(encrypted_note, key):
    try:
        encrypted_data = base64.b64decode(encrypted_note)
        nonce = encrypted_data[:16]
        tag = encrypted_data[-16:]  # Last 16 bytes are the tag
        ciphertext = encrypted_data[16:-16]  # Everything in between
        
        cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
        plaintext = cipher.decrypt_and_verify(ciphertext, tag)
        return plaintext.decode('utf-8')
    except Exception as e:
        raise Exception(f"Decryption error: {str(e)}")

if __name__ == "__main__":
    try:
        keyBytes = 32 # 256-bit key
        key = os.urandom(keyBytes)  # Generate random key
        # print(f"Key: {key}")
        note = "Đây là ghi chú cần bảo mật."
        
        # Test encryption and decryption
        encrypted = encrypt_note(note, key)
        decrypted = decrypt_note(encrypted, key)
        
        print(f"Original: {note}".encode('utf-8').decode('utf-8'))
        print(f"Encrypted: {encrypted}")
        print(f"Decrypted: {decrypted}".encode('utf-8').decode('utf-8'))
    except Exception as e:
        print(f"Error: {str(e)}")