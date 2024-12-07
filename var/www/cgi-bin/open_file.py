#!/usr/bin/python3
# open_file.py

with open("./var/www/cgi-bin/file.txt", "r") as f:
    content = f.read()

print("<html><body>")
print(f"<h1>{content}</h1>")
print("</body></html>")
