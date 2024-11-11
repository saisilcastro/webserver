#!/usr/bin/env python3

import cgi

print("Content-Type: text/html")
print()
print("""
<html>
  <head>
    <title>webserver</title>
    <link rel="stylesheet" href="css/style.css">
  </head>
  <body>
    <form method="post" enctype="multipart/form-data">
      <input type="file" name="myFile" />
      <input type="submit" value="Submit">
    </form>
  </body>
</html>
""")
