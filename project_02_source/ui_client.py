import tkinter as tk
from tkinter import messagebox
from auth_client import AuthClient

class App:
    def __init__(self, root):
        self.root = root
        self.auth = AuthClient()


        self.login_frame = tk.Frame(root)
        self.register_frame = tk.Frame(root)
        self.main_frame = tk.Frame(root)

        self.create_login_ui()
        self.create_register_ui()
        self.create_main_ui()


        self.show_frame(self.login_frame)

    def create_login_ui(self):
        tk.Label(self.login_frame, text="Login", font=("Arial", 16)).pack(pady=10)

        tk.Label(self.login_frame, text="Username:").pack()
        self.username_entry = tk.Entry(self.login_frame)
        self.username_entry.pack()

        tk.Label(self.login_frame, text="Password:").pack()
        self.password_entry = tk.Entry(self.login_frame, show="*")
        self.password_entry.pack()

        tk.Button(self.login_frame, text="Login", command=self.handle_login).pack(pady=10)
        tk.Button(self.login_frame, text="Register", command=lambda: self.show_frame(self.register_frame)).pack(pady=10)

    def create_register_ui(self):
        tk.Label(self.register_frame, text="Register", font=("Arial", 16)).pack(pady=10)

        tk.Label(self.register_frame, text="Username:").pack()
        self.reg_username_entry = tk.Entry(self.register_frame)
        self.reg_username_entry.pack()

        tk.Label(self.register_frame, text="Password:").pack()
        self.reg_password_entry = tk.Entry(self.register_frame, show="*")
        self.reg_password_entry.pack()

        tk.Button(self.register_frame, text="Register", command=self.handle_register).pack(pady=10)
        tk.Button(self.register_frame, text="Back to Login", command=lambda: self.show_frame(self.login_frame)).pack(pady=10)

    def create_main_ui(self):
        tk.Label(self.main_frame, text="Welcome to the Main Page!", font=("Arial", 16)).pack(pady=10)
        tk.Button(self.main_frame, text="Logout", command=self.handle_logout).pack(pady=10)

    def show_frame(self, frame):
        for widget in (self.login_frame, self.register_frame, self.main_frame):
            widget.pack_forget()
        frame.pack()

    def handle_login(self):
        username = self.username_entry.get()
        password = self.password_entry.get()

        success, message = self.auth.login(username, password)
        if success:
            messagebox.showinfo("Success", message)
            self.show_frame(self.main_frame)
            self.check_token_expiry()
        else:
            messagebox.showerror("Error", message)

    def handle_register(self):
        username = self.reg_username_entry.get()
        password = self.reg_password_entry.get()

        success, message = self.auth.register(username, password)
        if success:
            messagebox.showinfo("Success", message)
            self.show_frame(self.login_frame)
        else:
            messagebox.showerror("Error", message)

    def handle_logout(self):
        self.auth.token = None
        self.auth.token_expiry = None
        self.show_frame(self.login_frame)

    def check_token_expiry(self):
        if not self.auth.is_token_valid():
            messagebox.showwarning("Session Expired", "Your session has expired. Please login again.")
            self.handle_logout()
        else:
            self.root.after(1000, self.check_token_expiry)

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Client UI")
    root.geometry("400x300")
    app = App(root)
    root.mainloop()
