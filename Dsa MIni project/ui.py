import tkinter as tk
from tkinter import scrolledtext
import subprocess
import threading
import os
C_EXECUTABLE = "songplayer2.exe" 

class MusicPlayerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Music Player")
        self.root.geometry("600x500")
        self.root.configure(bg="#2c3e50")
        try:
           
            current_dir = os.path.dirname(os.path.abspath(__file__))
            
        
            exe_path = os.path.join(current_dir, C_EXECUTABLE)

            self.process = subprocess.Popen(
                [exe_path],
                cwd=current_dir,           
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True, 
                bufsize=1  
            )
        except FileNotFoundError:
            print(f"Error: Could not find {C_EXECUTABLE}. Did you compile it?")
            self.root.destroy()
            return

        self.create_widgets()

        
        self.read_thread = threading.Thread(target=self.read_c_output, daemon=True)
        self.read_thread.start()

    def create_widgets(self):
        # 1. Output Display Box
        self.output_box = scrolledtext.ScrolledText(self.root, width=70, height=15, bg="#ecf0f1", font=("Consolas", 10))
        self.output_box.pack(pady=10)

       
        input_frame = tk.Frame(self.root, bg="#2c3e50")
        input_frame.pack(pady=5)
        
        tk.Label(input_frame, text="Song Name:", bg="#2c3e50", fg="white").pack(side=tk.LEFT)
        self.entry_box = tk.Entry(input_frame, width=30)
        self.entry_box.pack(side=tk.LEFT, padx=5)

    
        btn_frame = tk.Frame(self.root, bg="#2c3e50")
        btn_frame.pack(pady=10)

        def make_btn(text, command):
            return tk.Button(btn_frame, text=text, command=command, bg="#3498db", fg="white", 
                             font=("Arial", 10, "bold"), width=12, cursor="hand2")

        make_btn("Load Folder", lambda: self.send_to_c("1")).grid(row=0, column=0, padx=5, pady=5)
        make_btn("Show Playlist", lambda: self.send_to_c("6")).grid(row=0, column=1, padx=5, pady=5)
        make_btn("Sort Playlist", lambda: self.send_to_c("5")).grid(row=0, column=2, padx=5, pady=5)
        make_btn("Play Specific", self.play_specific_song).grid(row=0, column=3, padx=5, pady=5)

        make_btn("⏪ Play First", lambda: self.send_to_c("7")).grid(row=1, column=0, padx=5, pady=5)
        make_btn("⏮ Prev Track", lambda: self.send_to_c("4")).grid(row=1, column=1, padx=5, pady=5)
        make_btn("⏭ Next Track", lambda: self.send_to_c("3")).grid(row=1, column=2, padx=5, pady=5)

    def play_specific_song(self):
        song_name = self.entry_box.get()
        if song_name.strip():
            self.send_to_c("2")
            self.send_to_c(song_name)
            self.entry_box.delete(0, tk.END)

    def send_to_c(self, command):
        if self.process.poll() is None: 
            self.process.stdin.write(command + "\n")
            self.process.stdin.flush()

    def read_c_output(self):
        while True:
            line = self.process.stdout.readline()
            if not line:
                break 
            self.output_box.insert(tk.END, line)
            self.output_box.see(tk.END) 

    def on_closing(self):
        self.send_to_c("8")
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = MusicPlayerUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()