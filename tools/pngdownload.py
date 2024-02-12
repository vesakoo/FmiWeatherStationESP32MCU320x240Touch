import os, sys
from PIL import Image
import requests
nums = range(1,200)
size = 20,20
for i in nums:
  URL = "http://cdn.fmi.fi/symbol-images/smartsymbol/v3/p/"+str(i)+".png"
  response = requests.get(URL)
  if response.status_code == 200:
    open(str(i)+".png", "wb").write(response.content)
  #try:
  #  im = Image.open(str(i)+".png")
  #  im.thumbnail(size, Image.Resampling.LANCZOS)
  #  im.save("./thmb/"+str(i)+".png", "PNG")
  #  #open(str(i)+".png", "wb").write(response.content)
  #except IOError:
  #  print( "no file:" +str(i))
