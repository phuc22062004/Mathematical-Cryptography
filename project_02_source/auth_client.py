import requests
import time

BASE_URL = "http://127.0.0.1:5000"


class AuthClient:
    def __init__(self):
        self.token = None
        self.token_expiry = None

    def login(self, username, password):
        url = f"{BASE_URL}/login"
        response = requests.post(url, json={"username": username, "password": password})

        if response.status_code == 200:
            data = response.json()
            self.token = data.get("access_token")
            # Giả định thời gian hết hạn token là 300 giây (5 phút)
            self.token_expiry = time.time() + 300
            return True, "Login successful!"
        else:
            return False, response.json().get("msg", "Login failed")

    def register(self, username, password):
        url = f"{BASE_URL}/register"
        response = requests.post(url, json={"username": username, "password": password})

        if response.status_code == 201:
            return True, response.json().get("msg", "Registration successful!")
        else:
            return False, response.json().get("msg", "Registration failed")

    def is_token_valid(self):
        if self.token and self.token_expiry > time.time():
            return True
        return False
