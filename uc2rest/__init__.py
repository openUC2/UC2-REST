from .config import *
from .UC2Client import *
try:
    from .updater import *
except:
    print("Updater not available")