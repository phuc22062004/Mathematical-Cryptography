from note_en_de import *

def main():
    try:
        keyBytes = 32 # 256-bit key
        key = os.urandom(keyBytes)  # Generate random key
        # print(f"Key: {key}")
        note = "Đây là ghi chú cần bảo mật." # Note này có thể thay đổi bằng cách đọc string
        
        # Test encryption and decryption
        encrypted = encrypt_note(note, key)
        decrypted = decrypt_note(encrypted, key)
        
        print(f"Original: {note}".encode('utf-8').decode('utf-8'))
        print(f"Encrypted: {encrypted}")
        print(f"Decrypted: {decrypted}".encode('utf-8').decode('utf-8'))
    except Exception as e:
        print(f"Error: {str(e)}")
        
if __name__ == "__main__":
    main()