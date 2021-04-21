import sys
import re

results = ""
try:
    with open("results.txt", "r") as f:
        results = f.read()
except:
    with open("results.txt", "r", encoding="ISO-8859-1") as f:
        results = f.read()

results = results.lower()

top, bot = results.split("\n\n")

top += "\n\nDICO_SEP\n\n"

bot = re.sub(r" +|\t+", "", bot).strip()
bot_lines = bot.split("\n")
for line in bot_lines:
    try:
        
        name, value = line.split(":")
        name = name.lower()
        
        if "fault" in name:
            value = int(value) - int(sys.argv[1])
        elif "backup" in name:
            value = int(value) - 0#256
        elif "ptlookup" in name:
            value = int(value) - 256

        top += name + ":" + str(value) + "\n"
    except:
        pass

with open("report.txt", "w") as f:
    f.write(top)
